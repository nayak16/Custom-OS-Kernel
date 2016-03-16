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
#include <cb_pool.h>
/* pdbr */
#include <special_reg_cntrl.h>
/* set_esp0 */
#include <x86/cr.h>

int scheduler_init(scheduler_t *sched){
    if (sched == NULL) return -1;
    sched->next_tid = 0;
    sched->next_pid = 0;
    sched->cur_tid = -1;
    if(queue_init(&(sched->runnable_pool)) < 0
            || queue_init(&(sched->waiting_pool)) < 0) {
        return -1;
    }
    if (cb_pool_init(&(sched->thr_pool)) < 0
            || cb_pool_init(&(sched->process_pool)) < 0) return -1;

    return 0;
}


int scheduler_add_process(scheduler_t *sched, pcb_t *pcb){
    if (sched == NULL) return -1;
    /* Assign next pid */
    pcb->id = sched->next_pid++;

    /* Create a new tcb to run the pcb */
    tcb_t *tcb = malloc(sizeof(tcb_t));
    if (tcb == NULL) return -2;
    int tid = sched->next_tid;
    sched->next_tid++;

    tcb_init(tcb, tid, pcb->id,
             (uint32_t *)pcb->stack_top, pcb->entry_point);

    /* Add pcb and tcb to respective pools */
    if (cb_pool_add_cb(&(sched->thr_pool), (void*)tcb) < 0
            || cb_pool_add_cb(&(sched->process_pool), (void*)pcb) < 0) {

        return -3;
    }

    if (queue_enq(&(sched->runnable_pool), (void *) tid) < 0) return -4;


    return 0;
}

int scheduler_start(scheduler_t *sched){
    // set page directory of current pcb
    //
    // enable_interrupts
    enable_interrupts();

    return 0;
}

int scheduler_save_old_tcb(scheduler_t *sched, uint32_t old_esp) {
    if (sched->cur_tid >= 0) {
        /* Get current tcb */
        tcb_t *tcb;
        if(cb_pool_get_cb(&(sched->thr_pool),
                         sched->cur_tid, (void**)&tcb) < 0) {
            return -1;
        }
        /* Save esp */
        tcb->esp = old_esp;
    }
    return 0;
}

int scheduler_set_running_tcb(scheduler_t *sched, tcb_t *tcb, uint32_t *new_esp) {

    /* Enqueue cur tid back into runnable pool */
    if(queue_enq(&(sched->runnable_pool), (void*) sched->cur_tid) < 0) {
        return -1;
    }

    /* Set new current running tid */
    sched->cur_tid = tcb->id;

    /* Save new esp  */
    *new_esp = tcb->esp;

    /* Set new esp0 */
    pcb_t *pcb;
    cb_pool_get_cb(&(sched->process_pool), tcb->pid, (void **)(&pcb));

    set_esp0((uint32_t)tcb->k_stack);
    set_pdbr((uint32_t) pd_get_base_addr(&(pcb->pd)));

    return 0;
}



int scheduler_get_next_tcb(scheduler_t *sched, tcb_t **tcbp) {

    int tid;
    if(queue_deq(&(sched->runnable_pool), (void**) &tid) < 0) {
        /* TODO: FATAL ERROR */
        return -1;
    }
    if(cb_pool_get_cb(&(sched->thr_pool), tid, (void**)tcbp) < 0) {
        /* TODO: FATAL ERROR */
        return -2;
    }
    return 0;
}
