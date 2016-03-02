/** @file
 *
 */

#include <stdlib.h>

#include <pcb.h>
#include <tcb.h>
#include <scheduler.h>

int scheduler_init(scheduler_t *sched){
    if (sched == NULL) return -1;
    sched->next_tid = 0;
    sched->next_pid = 0;
    return 0;
}

int scheduler_add_process(scheduler_t *sched, pcb_t *pcb){
    if (sched == NULL) return -1;
    pcb->pid = sched->next_pid++;
    sched->current_pcb = pcb;
    tcb_t *tcb = malloc(sizeof(tcb_t));
    tcb_init(tcb);
    tcb->tid = sched->next_tid++;
    tcb->pid = pcb->pid;
    sched->current_tcb = tcb;
    return 0;
}

int scheduler_start(scheduler_t *sched){
    // set page directory of current pcb
    //
    // enable_interrupts

    return 0;
}
