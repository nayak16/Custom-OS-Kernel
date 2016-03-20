/** @file life_cycle_handlers.c
 *  @brief implements life cycle syscall handlers
 *
 *  @author Christopher Wei (cjwei)
 *  @author Aatish Nayak (aatishn)
 *  @bug No known bugs.
 */

#include <kern_internals.h>
#include <malloc.h>

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

    mutex_lock(&scheduler_lock);
    int tid;
    /* Add duplicate to scheduler runnable queue */
    if((tid = scheduler_add_process(&sched, duplicate_pcb, saved_regs)) < 0) {
        return -6;
    }
    mutex_unlock(&scheduler_lock);
    return tid;
}

void syscall_set_status_c_handler(int status){
    lprintf("<set_status NYI> status = %d", status);
    return;
}

void syscall_vanish_c_handler(){
    lprintf("<vanish NYI> spin looping");
    while(1);
}
