/** @file
 *
 */

#include <stdlib.h>

#include <pcb.h>
#include <tcb.h>
#include <scheduler.h>
#include <context_switch.h>

int scheduler_init(scheduler_t *sched){
    if (sched == NULL) return -1;
    sched->next_tid = 0;
    sched->next_pid = 0;
    return 0;
}

int scheduler_add_process(scheduler_t *sched, pcb_t *pcb){
    if (sched == NULL) return -1;
    /* Assign next pid */
    pcb->pid = sched->next_pid++;

    /* Set to current pcb TODO: Change this with add to pool */
    sched->current_pcb = pcb;

    /* Create a new tcb to run the pcb */
    tcb_t *tcb = malloc(sizeof(tcb_t));
    tcb_init(tcb, sched->next_tid++, pcb->pid,
             pcb->stack_top, pcb->entry_point);

    /* Set current tcb in scheduler TODO: Change to pool format */
    sched->current_tcb = tcb;
    return 0;
}

int scheduler_start(scheduler_t *sched){
    // set page directory of current pcb
    //
    // enable_interrupts
    restore_context(sched->current_tcb);

    return 0;
}
