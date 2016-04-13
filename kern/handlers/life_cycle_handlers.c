/** @file life_cycle_handlers.c
 *  @brief implements life cycle syscall handlers
 *
 *  @author Christopher Wei (cjwei)
 *  @author Aatish Nayak (aatishn)
 *  @bug Validation of syscall arguments
 */

#include <kern_internals.h>
#include <malloc.h>
#include <string.h>
#include <dispatcher.h>
#include <virtual_mem_mgmt.h>
#include <special_reg_cntrl.h>
#include <common_kern.h>
#include <x86/asm.h>
#include <x86/cr.h>
#include <loader.h>
#include <thr_helpers.h>

#include <simics.h>
/**
 * @brief Implements the fork system call. Creates a new process
 * from the invoking one copying its registers and duplicating address
 * space
 *
 * @param saved_regs Array of all of invoking thread's registers
 *
 * @return tid of the newly created thread running the forked process
 * if success, negative error code on error
 *
 */
int syscall_fork_c_handler(uint32_t *saved_regs){

    /* Get current running pcb */
    pcb_t *cur_pcb;
    if(scheduler_get_current_pcb(&sched, &cur_pcb) < 0) {
        return -2;
    }

    /* Lock while reading number of threads */
    mutex_lock(&(cur_pcb->m));
    /* If the current pcb has > 1 threads reject fork */
    if (cur_pcb->num_threads > 1) {
        mutex_unlock(&(cur_pcb->m));
        return -3;
    }
    mutex_unlock(&(cur_pcb->m));

    /* Allocate space for a duplicate pcb */
    pcb_t *duplicate_pcb = malloc(sizeof(pcb_t));
    if(duplicate_pcb == NULL) return -4;

    if (pcb_init(duplicate_pcb) < 0) {
        /* Cleanup on failure */
        free(duplicate_pcb);
        return -5;
    }

    /* Create copy of current pcb with duplicate address space */
    if (pcb_copy(duplicate_pcb, cur_pcb) < 0) {
        /* Cleanup on failure */
        pcb_destroy_s(duplicate_pcb);
        free(duplicate_pcb);
        return -6;
    }

    int tid;
    /* Add duplicate to scheduler runnable queue */
    if((tid = scheduler_add_process(&sched,
                duplicate_pcb, saved_regs)) < 0) {
        /* Cleanup on failure */
        pcb_destroy_s(duplicate_pcb);
        free(duplicate_pcb);
        return -7;
    }

    /* Inc children count safely in current process */
    pcb_inc_children_s(cur_pcb);

    return tid;
}

int syscall_thread_fork_c_handler(uint32_t *saved_regs) {

    /* Add a new thread to the scheduler using same registers */
    return scheduler_add_new_thread(&sched, saved_regs);
}


/**
 * @brief Handles the exec syscall. Loads a new pcb and immediately starts
 * running it
 *
 * @bug What if one of the arguments is 0 or null string
 *
 * @param execname Name of program to run
 * @param argvec Null-terminated array of string args to pass to program
 *
 * @return 0 on success, negative error code otherwise
 */

int syscall_exec_c_handler(char *execname, char **argvec) {
    if (execname == NULL || argvec == NULL) return -1;

    /* Check if elf filename exists */
    if (!load_elf_exists(execname)){
        return -2;
    }

    /* Get current tcb */
    tcb_t *cur_tcb;
    if (scheduler_get_current_tcb(&sched, &cur_tcb) < 0) {
        panic("Can't obtain current tcb. Scheduler is corrupted...");
        return -4;
    }

    /* Get current pcb */
    pcb_t *cur_pcb;
    if (tcb_get_pcb(cur_tcb, &cur_pcb) < 0) return -5;

    /* Lock pcb while reading value */
    mutex_lock(&(cur_pcb->m));

    /* If the current pcb has > 1 threads reject exec */
    if (cur_pcb->num_threads > 1) return -3;

    /* Release lock */
    mutex_unlock(&(cur_pcb->m));

    /* Parse args and get argc*/
    char **argp = argvec;
    int argc = 0;
    /* Check validity in argp */
    while((pd_get_mapping(&(cur_pcb->pd), (uint32_t) argp, NULL) == 0)
            && *argp != NULL) {
        /* Check if each string is a valid pointer */
        if (pd_get_mapping(&(cur_pcb->pd), (uint32_t) *argp, NULL) < 0) {
            return -7;
        }
        argc++;
        argp += 1;
    }
    /* Check if failed due to bad mapping */
    if (pd_get_mapping(&(cur_pcb->pd), (uint32_t) argp, NULL) < 0) {
        return -8;
    }

    /* Make local copy of execname */
    int len = strlen(execname);
    char name_copy[len+1];
    memcpy(name_copy, execname, len);
    /* Null terminated */
    name_copy[len] = '\0';
    /* Make local copy of argv */
    char *local_argv[argc];
    int i;
    for (i = 0; i < argc ; i++) {
        int len = strlen(argvec[i])+1;
        /* Allocate space for each local_argv arg */
        local_argv[i] = malloc(sizeof(char) * len);
        memcpy(local_argv[i], argvec[i], len);
    }

    lprintf("Starting program %s ...", execname);
    /* Clear old pcb user space mappings */
    if (vmm_clear_user_space(&(cur_pcb->pd)) < 0) return -6;

    /* Load in new program */
    if (pcb_load_prog(cur_pcb, name_copy, argc, local_argv) < 0) {
        // TODO: Figure out failure
        lprintf("Failed to load program: %s", name_copy);
        MAGIC_BREAK;
        return -3;
    }
    /* Free all allocated local_argv args */
    for (i = 0; i < argc ; i++) {
        free(local_argv[i]);
    }

    /* Reload a tcb with new contents */
    tcb_reload(cur_tcb, cur_pcb);

    /* Get stack to start at */
    void *init_stack;
    tcb_get_init_stack(cur_tcb, &init_stack);

    /* Set esp0 to newly malloced kstack */
    set_esp0((uint32_t)init_stack);

    /* Restore context with new program */
    restore_context((uint32_t)init_stack);

    /* SHOULD NEVER RETURN */
    return 0;
}


/** @brief Implements the set_status system call
 *  @param status The status to set the current tcb's exit status to
 *  @return Void
 */
void syscall_set_status_c_handler(int status){
    thr_set_status(status);
}

/** @brief Implements the vanish system call
 *  @return Does not return
 */
void syscall_vanish_c_handler(){
    thr_vanish();
}


/** @brief Implements the wait system call
 *  @param status_ptr Where to store the status of the collected child
 *  @return 0 on success, negative integer code on failure
 */
int syscall_wait_c_handler(int *status_ptr){
    pcb_t *cur_pcb;
    int original_pid;

    /* get the current running pcb */
    if (scheduler_get_current_pcb(&sched, &cur_pcb) < 0) return -1;

    /* pcb has children, wait until they report their status */
    if (pcb_wait_on_status(cur_pcb, status_ptr, &original_pid) < 0)
        return -2;

    /* Decrement pcb child count */

    pcb_dec_children_s(cur_pcb);

    return original_pid;
}

