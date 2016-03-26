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
#include <tcb_pool.h>
/* pdbr */
#include <special_reg_cntrl.h>
/* set_esp0 */
#include <x86/cr.h>

#include <simics.h>

uint32_t scheduler_num_ticks = 0;

int is_started(scheduler_t *sched) {
    return sched->cur_tcb != NULL;
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
    sched->cur_tcb = NULL;

    /* Init tcb pool */
    if (tcb_pool_init(&(sched->thr_pool)) < 0) return -2;

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
    *tidp = sched->cur_tcb->tid;
    return 0;
}
/**
 * @brief Gets the tcb with the specified tid
 *
 * @param sched Scheduler to get next tcb from
 * @param target_tid tid to look for
 * @param tcbp Address to store pointer to tcb
 *
 * @return 0 on success, negative error code otherwise
 */
int scheduler_get_tcb_by_tid(scheduler_t *sched,
                             int target_tid, tcb_t **tcbp) {
    if (sched == NULL || tcbp == NULL) return -1;

    /* Cycle runnable pool and get next tcb to run */
    if (tcb_pool_find_tcb(&(sched->thr_pool), target_tid, tcbp) < 0) return -2;

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
    *pcb = sched->cur_tcb->pcb;
    return 0;
}

/**
 * @brief Gets current running tcb
 *
 * @param sched scheduler to get tcb from
 * @param tcb address to store current tcb
 *
 * @return 0 on success, negative error code otherwise
 *
 */
int scheduler_get_current_tcb(scheduler_t *sched, tcb_t **tcb) {

    if (sched == NULL) return -1;
    *tcb = sched->cur_tcb;
    return 0;
}


/**
 * @brief Creates a tcb to run the specified pcb and adds
 * it to the scheduler's runnable pool
 *
 * Used in fork/exec/kernel startup
 *
 * @param sched scheduler to add pcb to
 * @param pcb address to pcb to add the scheduler
 * @param regs pointer to register values to set to the new thread's
 * registers
 *
 * @return tid of thread running process on success,
 *         negative error code otherwise
 */
int scheduler_add_process(scheduler_t *sched, pcb_t *pcb, uint32_t *regs){
    if (sched == NULL) return -1;

    /* Assign next pid */
    pcb->pid = sched->next_pid++;

    /* Create a new tcb to run the pcb */
    tcb_t *tcb = malloc(sizeof(tcb_t));
    if (tcb == NULL) return -2;
    int tid = sched->next_tid++;

    tcb_init(tcb, tid, pcb, regs);

    /* Add a runnable tcb to pool */
    if (tcb_pool_add_runnable_tcb(&(sched->thr_pool), tcb) < 0) return -3;

    return tid;
}

int scheduler_add_process_safe(scheduler_t *sched,
                                pcb_t *pcb, uint32_t *regs){

    disable_interrupts();
    int status = scheduler_add_process(sched, pcb, regs);
    enable_interrupts();
    return status;
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
 * SHOULD ONLY BE USED BY CONTEXT_SWITCH
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

        /* Save k_stack esp */
        sched->cur_tcb->tmp_k_stack = (uint32_t *)old_esp;

        /* Set current tcb status back to RUNNABLE */
        sched->cur_tcb->status = RUNNABLE;
    }
    return 0;
}

/**
 * @brief Sets the specified tcb to run, and stores it's saved k_stack esp
 * at the specified address
 *
 * Houskeeping tasks:
 * update current tcb, and pcbs
 * reports previously saved k_stack esp
 *
 * Processor Level tasks:
 * Set new esp0
 * Set new pdbr
 *
 * SHOULD ONLY BE USED BY CONTEXT_SWITCH
 *
 * @param sched Scheduler to use to set running tcb
 * @param tcb The tcb to set to running
 * @param new_esp Address to put saved k_stack esp
 *
 * @return 0 on success, negative error code otherwise
 */
int scheduler_set_running_tcb(scheduler_t *sched, tcb_t *tcb, uint32_t *new_esp) {
    if (sched == NULL || tcb == NULL || new_esp == NULL) return -1;
    /* Set new current running tid */
    sched->cur_tcb = tcb;
    sched->cur_tcb->status = RUNNING;

    /* Save new esp  */
    *new_esp = (uint32_t)tcb->tmp_k_stack;

    /* Set new esp0 */
    set_esp0((uint32_t)(tcb->orig_k_stack));

    /* Set new page directory */
    set_pdbr((uint32_t) pd_get_base_addr(&(tcb->pcb->pd)));

    return 0;
}

/**
 * @brief Reports the next tcb to run/schedule
 *
 * @param sched Scheduler to get next tcb from
 * @param tcbp Address to store pointer to next tcb
 *
 * @return 0 on success, negative error code otherwise
 */
int scheduler_get_next_tcb(scheduler_t *sched, tcb_t **tcbp) {
    if (sched == NULL || tcbp == NULL) return -1;

    /* Cycle runnable pool and get next tcb to run */
    if (tcb_pool_get_next_tcb(&(sched->thr_pool), tcbp) < 0) return -2;

    return 0;
}

