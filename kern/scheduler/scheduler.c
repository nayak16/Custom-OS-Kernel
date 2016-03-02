/** @file
 *
 */

#include <pcb_t.h>
#include <tcb_t.h>

typedef struct scheduler{
    int next_tid;
    int next_pid;
    pcb *current_pcb;
    tcb *current_tcb;
} scheduler_t;


int scheduler_init(scheduler_t *sched){
    if (sched == NULL) return -1;
    sched->next_tid = 0;
    sched->next_pid = 0;
}

int scheduler_add_process(scheduler_t *sched, pcb_t *pcb){
    if (sched == NULL) return -1;
    pcb->pid = sched->next_pid++;
    sched->current_pcb = pcb;
    tcb_t *tcb = malloc(sizeof(tcb_t));
    tcb_init(tcb);
    tcb->tid = next_tid++;
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
