/** @file
 *
 */

#include <stdlib.h>

#include <x86/asm.h>
#include <pcb.h>
#include <tcb.h>
#include <scheduler.h>
#include <dispatcher.h>
#include <queue.h>

int scheduler_init(scheduler_t *sched){
    if (sched == NULL) return -1;
    sched->next_tid = 0;
    sched->next_pid = 0;
    sched->cur_tid = -1;
    if(queue_init(&(sched->runnable_pool)) < 0
            || queue_init(&(sched->waiting_pool)) < 0) {
        return -1;
    }
    if (thr_pool_init(&(sched->thr_pool)) < 0) return -1;

    return 0;
}

int scheduler_add_process(scheduler_t *sched, pcb_t *pcb){
    if (sched == NULL) return -1;
    /* Assign next pid */
    pcb->pid = sched->next_pid++;

    /* Create a new tcb to run the pcb */
    tcb_t *tcb = malloc(sizeof(tcb_t));
    if (tcb == NULL) return -2;
    int tid = sched->next_tid;

    tcb_init(tcb, tid, pcb->pid,
             pcb->stack_top, pcb->entry_point);

    sched->next_tid++;

    if (thr_pool_add_tcb(&(sched->thr_pool), tcb) < 0) return -3;
    if (queue_enq(&(sched->runnable_pool), (void *) tid) < 0) return -4;


    return 0;
}

int scheduler_start(scheduler_t *sched){
    // set page directory of current pcb
    //
    // enable_interrupts
    enable_interrupts();

    int tid;
    queue_deq(&(sched->runnable_pool), (void**)&tid);
    tcb_t *tcb;
    thr_pool_get_tcb(&(sched->thr_pool), tid, &tcb);

    initial_mode_switch(tcb);

    return 0;
}
