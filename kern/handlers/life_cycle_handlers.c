/** @file life_cycle_handlers.c
 *  @brief implements life cycle syscall handlers
 *
 *  @author Christopher Wei (cjwei)
 *  @author Aatish Nayak (aatishn)
 *  @bug No known bugs.
 */

#include <kern_internals.h>
#include <malloc.h>

int syscall_fork_c_handler(uint32_t *saved_regs){


    /* Get current running pcb */
    pcb_t *cur_pcb;
    if(scheduler_get_current_pcb(&sched, &cur_pcb) < 0) {
        return -2;
    }

    /* Create copy of current pcb with duplicate address space */
    pcb_t *duplicate_pcb = malloc(sizeof(pcb_t));
    if(duplicate_pcb == NULL) return -3;
    if (pcb_init(duplicate_pcb) < 0) return -4;

    if (pcb_copy(duplicate_pcb, cur_pcb) < 0) {
        return -5;
    }

    int tid;
    /* Add duplicate to scheduler runnable queue */
    if((tid = scheduler_add_process(&sched, duplicate_pcb, saved_regs)) < 0) {
        return -6;
    }
    return tid;
}

