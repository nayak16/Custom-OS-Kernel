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

/**
 * @brief Initialize a scheduler's internal data structures and values
 * including internal thread and process pools
 *
 * @param sched scheduler to initialize
 *
 * @return 0 on success, negative error code otherwise
 *
 */
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

/**
 * @brief Get current running tcb's tid
 *
 * @param sched scheduler to get current tid from
 * @param tidp address to store current tid
 *
 * @return 0 on success, negative error code otherwise
 *
 */
int scheduler_get_current_tid(scheduler_t *sched, int *tidp) {
    if (sched == NULL) return -1;

    *tidp = sched->cur_tid;
    return 0;
}

/**
 * @brief Gets current running pcb
 *
 * @param sched scheduler to get pcb from
 * @param pcb address to store current pcb
 *
 * @return 0 on success, negative error code otherwise
 *
 */
int scheduler_get_current_pcb(scheduler_t *sched, pcb_t **pcb) {

    if (sched == NULL) return -1;

    /* Find current pcb */
    if (cb_pool_get_cb(&(sched->process_pool),
                        sched->cur_pid, (void **)(pcb)) < 0) {
        return -2;
    }
    return 0;

}

/**
 * @brief Creates a tcb to run the specified pcb and adds
 * it to the scheduler's runnable pool
 *
 * @param sched scheduler to add pcb to
 * @param pcb address to pcb to add the scheduler
 * @param regs pointer to register values to set to the new thread's
 * registers
 *
 * @return 0 on success, negative error code otherwise
 */
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

    /* Enqueue tid to runnable pool*/
    if (queue_enq(&(sched->runnable_pool), (void *) tid) < 0) return -4;


    return tid;
}

/**
 * @brief Starts the scheduler
 *
 * Enables interrupts to allow timer handler to start context switching
 *
 * @param sched Scheduler to start
 *
 * @return 0 on success, negative error code otherwise
 *
 */
int scheduler_start(scheduler_t *sched){

    enable_interrupts();

    return 0;
}


/**
 * @brief Removes current running tcb and puts it back into runnable pool
 *
 * @param sched Scheduler to get current tcb from
 * @param old_esp stack pointer in k_stack all registers are saved
 *
 * @return 0 on success, negative error code otherwise
 *
 *
 */
int scheduler_defer_current_tcb(scheduler_t *sched, uint32_t old_esp) {
    /* Check there is any running tcb */
    if (is_started(sched)) {
        /* Get current tcb */
        tcb_t *tcb;
        if(cb_pool_get_cb(&(sched->thr_pool),
                         sched->cur_tid, (void**)&tcb) < 0) {
            return -1;
        }
        /* Save k_stack esp */
        tcb->tmp_k_stack = (uint32_t *)old_esp;

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
    *new_esp = (uint32_t)tcb->tmp_k_stack;

    /* Set new esp0 */
    set_esp0((uint32_t)(tcb->orig_k_stack));

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

