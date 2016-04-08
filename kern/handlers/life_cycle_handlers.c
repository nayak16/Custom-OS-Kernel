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

    /* Grab scheduler lock */
    /* Get current running pcb */
    pcb_t *cur_pcb;
    if(scheduler_get_current_pcb(&sched, &cur_pcb) < 0) {
        return -2;
    }
    /* If the current pcb has > 1 threads reject fork */
    if (cur_pcb->num_threads > 1) return -3;

    /* Allocate space for a duplicate pcb */
    pcb_t *duplicate_pcb = malloc(sizeof(pcb_t));
    if(duplicate_pcb == NULL) return -4;
    if (pcb_init(duplicate_pcb) < 0) return -5;

    /* Create copy of current pcb with duplicate address space */
    if (pcb_copy(duplicate_pcb, cur_pcb) < 0) return -6;

    int tid;
    /* Add duplicate to scheduler runnable queue */
    if((tid = scheduler_add_process_safe(&sched,
                duplicate_pcb, saved_regs)) < 0) {
        return -7;
    }

    /* Inc children count in current process */
    pcb_inc_children(cur_pcb);

    return tid;
}

int syscall_thread_fork_c_handler(uint32_t *saved_regs) {

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
        // TODO: Fatal Error
        panic("Can't obtain current pcb");
        return -4;
    }
    /* Get current pcb */
    pcb_t *cur_pcb;
    if (tcb_get_pcb(cur_tcb, &cur_pcb) < 0) return -5;

    /* If the current pcb has > 1 threads reject exec */
    if (cur_pcb->num_threads > 1) return -3;

    // TODO: Check validity and mapping of each string and arg
    /* Parse args and get argc*/
    char **argp = argvec;
    int argc = 0;
    while(/*pd_get_mapping(argvec) &&*/ *argp != NULL) {
        argc++;
        lprintf("argvec: %p, arg: %s", argp, *argp);
        argp += 1;
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

    /* Clear old pcb user space mappings */
    if (vmm_clear_user_space(&(cur_pcb->pd)) < 0) return -6;

    /* Load in new program */
    lprintf("Loading new program...");
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
    tcb_reload_safe(cur_tcb, cur_pcb);

    /* Get stack to start at */
    void *init_stack;
    tcb_get_init_stack(cur_tcb, &init_stack);

    /* Restore context with new program */
    restore_context((uint32_t)init_stack);

    /* SHOULD NEVER RETURN */
    return 0;
}

void syscall_set_status_c_handler(int status){
    thr_set_status(status);
}

void syscall_vanish_c_handler(){
    thr_vanish();
}

int syscall_wait_c_handler(int *status_ptr){
    pcb_t *cur_pcb;
    int original_pid;

    /* get the current running pcb */
    if (scheduler_get_current_pcb(&sched, &cur_pcb) < 0) return -1;

    /* pcb has children, wait until they report their status */
    if (pcb_wait_on_status(cur_pcb, status_ptr, &original_pid) < 0)
        return -2;

    /* Decrement pcb child count */

    pcb_dec_children(cur_pcb);

    return original_pid;
}

