/** @file life_cycle_handlers.c
 *  @brief implements life cycle syscall handlers
 *
 *  @author Christopher Wei (cjwei)
 *  @author Aatish Nayak (aatishn)
 *  @bug Validation of syscall arguments
 */

#include <kern_internals.h>
#include <malloc.h>
#include <x86/asm.h>

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
    mutex_lock(&scheduler_lock);
    /* Get current running pcb */
    pcb_t *cur_pcb;
    if(scheduler_get_current_pcb(&sched, &cur_pcb) < 0) {
        return -2;
    }
    mutex_unlock(&scheduler_lock);

    /* Allocate space for a duplicate pcb */
    pcb_t *duplicate_pcb = malloc(sizeof(pcb_t));
    if(duplicate_pcb == NULL) return -3;
    if (pcb_init(duplicate_pcb) < 0) return -4;

    /* Create copy of current pcb with duplicate address space */
    if (pcb_copy(duplicate_pcb, cur_pcb) < 0) {
        return -5;
    }

    int tid;
    /* Add duplicate to scheduler runnable queue */
    if((tid = scheduler_add_process_safe(&sched,
                                         duplicate_pcb, saved_regs)) < 0) {
        return -6;
    }
    return tid;
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

    // TODO: Check validity and mapping of each string and arg
    /* Parse args and get argc*/
    char **argp = argvec;
    int argc = 0;
    while(/* pd_check_mapped(argvec) && */*argp != NULL) {
        argc++;
        lprintf("argvec: %p, arg: %s", argp, *argp);
        argp += 1;
    }

    /* Create new pcb and load new program */
    pcb_t new_pcb;
    pcb_init(&new_pcb);
    MAGIC_BREAK;
    if (pcb_load_prog(&new_pcb, execname, argc, argvec) < 0) {
        lprintf("Failed to load program: %s", execname);
        return -2;
    }

    /* Add process to scheduler */


    lprintf("Starting program '%s', with %d args", execname, argc);
    return 0;
}

void syscall_set_status_c_handler(int status){
    lprintf("<set_status NYI> status = %d", status);
    return;
}

void syscall_vanish_c_handler(){
    lprintf("<vanish NYI> spin looping");
    while(1);
}

