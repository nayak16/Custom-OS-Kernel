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

#include <simics.h>

int is_started(scheduler_t *sched) {
    return sched->cur_tid >= 0;
}

int scheduler_init(scheduler_t *sched){
    if (sched == NULL) return -1;
    sched->next_tid = 0;
    sched->next_pid = 0;
    sched->cur_tid = -1;
    sched->cur_pid = -1;

    if(queue_init(&(sched->runnable_pool)) < 0
            || queue_init(&(sched->waiting_pool)) < 0) {
        return -1;
    }
    if (cb_pool_init(&(sched->thr_pool)) < 0
            || cb_pool_init(&(sched->process_pool)) < 0) return -1;

    return 0;
}


int scheduler_get_current_tid(scheduler_t *sched, int *tidp) {
    *tidp = sched->cur_tid;
    return 0;
}

int scheduler_copy_current_pcb(scheduler_t *sched, uint32_t *regs) {

    if (sched == NULL) return -1;

    /* Find current pcb in pool */
    pcb_t *cur_pcb;
    if (cb_pool_get_cb(&(sched->process_pool),
                        sched->cur_tid, (void **)(&cur_pcb)) < 0) {
        return -2;
    }
    /* Create copy of current pcb with duplicate address space */
    pcb_t duplicate_pcb;
    if (pcb_copy(cur_pcb, &duplicate_pcb) < 0) {
        return -3;
    }

    int tid;
    /* Add duplicate to scheduler runnable queue */
    if((tid = scheduler_add_process(sched, &duplicate_pcb, regs)) < 0) {
        return -4;
    }

    return tid;

}

int scheduler_add_process(scheduler_t *sched, pcb_t *pcb, uint32_t *regs){
    if (sched == NULL) return -1;
    /* Assign next pid */
    pcb->id = sched->next_pid++;

    /* Create a new tcb to run the pcb */
    tcb_t *tcb = malloc(sizeof(tcb_t));
    if (tcb == NULL) return -2;
    int tid = sched->next_tid;
    sched->next_tid++;

    tcb_init(tcb, tid, pcb->id,
             (uint32_t *)pcb->stack_top, pcb->entry_point, regs);

    /* Add pcb and tcb to respective pools */
    if (cb_pool_add_cb(&(sched->thr_pool), (void*)tcb) < 0
            || cb_pool_add_cb(&(sched->process_pool), (void*)pcb) < 0) {

        return -3;
    }

    if (queue_enq(&(sched->runnable_pool), (void *) tid) < 0) return -4;


    return tid;
}

int scheduler_start(scheduler_t *sched){
    // set page directory of current pcb
    //
    // enable_interrupts
    enable_interrupts();

    return 0;
}

int scheduler_save_running_tcb(scheduler_t *sched, uint32_t old_esp) {
    if (is_started(sched)) {
        /* Get current tcb */
        tcb_t *tcb;
        if(cb_pool_get_cb(&(sched->thr_pool),
                         sched->cur_tid, (void**)&tcb) < 0) {
            return -1;
        }
        /* Save k_stack esp */
        tcb->k_stack = (void *) old_esp;

        /* Put current tid back into runnable pool */
        if(queue_enq(&(sched->runnable_pool), (void*) sched->cur_tid) < 0) {
            return -1;
        }

    }
    return 0;
}

int scheduler_set_running_tcb(scheduler_t *sched, tcb_t *tcb, uint32_t *new_esp) {

    /* Set new current running tid */
    sched->cur_tid = tcb->id;

    /* Set new current running pid */
    sched->cur_pid = tcb->pid;


    /* Save new esp  */
    *new_esp = (uint32_t)tcb->k_stack;

    /* Set new esp0 */
    set_esp0((uint32_t)tcb->k_stack);

    pcb_t *pcb;
    if (cb_pool_get_cb(&(sched->process_pool), tcb->pid, (void **)(&pcb)) < 0) {
        return -2;
    }

    /* Set new page directory */
    set_pdbr((uint32_t) pd_get_base_addr(&(pcb->pd)));

    return 0;
}


int scheduler_get_next_tcb(scheduler_t *sched, tcb_t **tcbp) {

    int tid;
    /* Dequeue from runnable pool */
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

