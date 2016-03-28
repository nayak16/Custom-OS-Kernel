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

    /* Find the correct tcb from the pool */
    if (tcb_pool_find_tcb(&(sched->thr_pool), target_tid, tcbp) < 0) return -2;

    return 0;
}

/**
 * @brief Checks if the tcb with the specified tid is currently
 * runnable or not
 *
 * @param sched Scheduler to check
 * @param target_tid tid of tcb to check
 *
 * @return 1 if tcb is RUNNABLE, 0 if not, negative error code otherwise
 *
 */
int scheduler_check_is_runnable(scheduler_t *sched, int target_tid) {
    if (sched == NULL) return -1;

    tcb_t *tcb;
    /* Find the correct tcb from the pool */
    if (tcb_pool_find_tcb(&(sched->thr_pool), target_tid, &tcb) < 0) return -2;

    /* Check the status */
    return tcb->status == RUNNABLE;

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
int scheduler_get_pcb_by_pid(scheduler_t *sched,
                             int target_pid, pcb_t **pcbp) {
    if (sched == NULL || pcbp == NULL) return -1;

    if (tcb_pool_find_pcb(&(sched->thr_pool), target_pid, pcbp) < 0) return -2;

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
 * @brief Removes the current tcb from the runnable pool and puts it
 * in the waiting pool
 *
 * @param sched Scheduler to manipulate
 *
 * @return 0 on success, negative error code otherwise
 */
int scheduler_deschedule_current(scheduler_t *sched) {
    if (sched == NULL) return -1;

    /* Manipulate tcb_pool*/
    if (tcb_pool_make_waiting(&(sched->thr_pool), sched->cur_tcb->tid) < 0) {
        return -3;
    }
    /* Set current tcb to WAITING */
    sched->cur_tcb->status = WAITING;

    return 0;
}

/**
 * @brief Disables interrupts and deschedules the current tcb
 *
 * Context switch must not happen while modifying scheduler
 * data structures i.e. runnable_pool, waiting_pool
 *
 * @param sched Scheduler to manipulate
 *
 * @return 0 on success, negative error code otherwise
 */
int scheduler_deschedule_current_safe(scheduler_t *sched) {
    disable_interrupts();
    int status = scheduler_deschedule_current(sched);
    enable_interrupts();
    return status;
}

/**
 * @brief Moves the tcb with the specified tid from waiting or sleeping
 *        to runnable
 *
 * @param sched Scheduler to manipulate
 * @param tid tid of tcb to make runnable
 *
 * @return 0 on success, negative error code otherwise
 */
int scheduler_make_runnable(scheduler_t *sched, int tid) {
    if (sched == NULL) return -1;

    /* Manipulate tcb_pool*/
    if (tcb_pool_make_runnable(&(sched->thr_pool), tid) < 0) {
        return -2;
    }

    tcb_t *tcb;
    /* Get specified tcb */
    if (tcb_pool_find_tcb(&(sched->thr_pool), tid, &tcb) < 0) return -3;

    /* Set current tcb to RUNNABLE */
    tcb->status = RUNNABLE;

    return -0;
}

/**
 * @brief Disables interrupts and makes the tcb with the specified
 * tid runnable
 *
 * Context switch must not happen while modifying scheduler
 * data structures i.e. runnable_pool, waiting_pool
 *
 * @param sched Scheduler to manipulate
 * @param tid tid of tcb to make runnable
 *
 * @return 0 on success, negative error code otherwise
 */
int scheduler_make_runnable_safe(scheduler_t *sched, int tid) {
    disable_interrupts();
    int status = scheduler_make_runnable(sched, tid);
    enable_interrupts();
    return status;
}

/**
 * @brief Moves the tcb with the specified tid from running to sleeping
 *
 * @param sched Scheduler to manipulate
 * @param tid tid of tcb to make sleeping
 *
 * @return 0 on success, negative error code otherwise
 */
int scheduler_make_current_sleeping(scheduler_t *sched, int ticks) {
    if (sched == NULL || ticks < 0) return -1;

    /* Return immediately if ticks == 0 */
    if (ticks == 0) return 0;

    /* Somehow already sleeping...?*/
    if (sched->cur_tcb->status == SLEEPING) return -4;
    /* Set current tcb to RUNNABLE */
    sched->cur_tcb->status = SLEEPING;
    if (scheduler_num_ticks + ticks < scheduler_num_ticks){
        //TODO: handle this...
        MAGIC_BREAK;
    }
    sched->cur_tcb->t_wakeup = scheduler_num_ticks+ticks;

    /* Manipulate tcb_pool*/
    if (tcb_pool_make_sleeping(&(sched->thr_pool), sched->cur_tcb->tid) < 0) {
        return -2;
    }

    return -0;
}

/**
 * @brief Disables interrupts and makes the tcb with the specified
 * tid sleeping
 *
 * Context switch must not happen while modifying scheduler
 * data structures i.e. runnable_pool, waiting_pool
 *
 * @param sched Scheduler to manipulate
 * @param tid tid of tcb to make sleeping
 *
 * @return 0 on success, negative error code otherwise
 */
int scheduler_make_current_sleeping_safe(scheduler_t *sched, int ticks) {
    disable_interrupts();
    int status = scheduler_make_current_sleeping(sched, ticks);
    enable_interrupts();
    return status;
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

/**
 * @brief Moves the tcb with the specified tid into the zombie pool
 *
 * @param sched Scheduler to manipulate
 * @param tid tid of tcb to make sleeping
 *
 * @return 0 on success, negative error code otherwise
 */
int scheduler_make_current_zombie(scheduler_t *sched) {
    if (sched == NULL) return -1;
    if (tcb_pool_make_zombie(&(sched->thr_pool), sched->cur_tcb->tid) < 0) {
        return -2;
    }
    return 0;
}

/**
 * @brief Disables interrupts and makes the tcb with the specified
 * tid a zombie
 *
 * Context switch must not happen while modifying scheduler
 * data structures i.e. runnable_pool, waiting_pool
 *
 * @param sched Scheduler to manipulate
 * @param tid tid of tcb to make zombie
 *
 * @return 0 on success, negative error code otherwise
 */
int scheduler_make_current_zombie_safe(scheduler_t *sched) {
    disable_interrupts();
    int status = scheduler_make_current_zombie(sched);
    enable_interrupts();
    return status;
}

int scheduler_get_current_tcb(scheduler_t *sched, tcb_t **tcb) {
    if (sched == NULL) return -1;
    *tcb = sched->cur_tcb;
    return 0;
}

/**
 * @brief Creates a tcb for the idle pcb specified and saves it in the
 * scheduler.
 *
 * The idle tcb will run when no other tcb exists in the
 * runnable pool.
 *
 * @param sched Scheduler to add to
 * @param idle_pcb Pointer to idle pcb with add
 *
 * @return tid of idle tcb on success, negative error code otherwise
 *
 */
int scheduler_add_idle_process(scheduler_t *sched, pcb_t *idle_pcb) {
    if (sched == NULL) return -1;

    /* Set pid of idle_pcb */
    idle_pcb->pid = sched->next_pid++;

    /* Create idle tcb */
    tcb_t *idle_tcb = malloc(sizeof(tcb_t));
    if (idle_tcb == NULL) return -2;
    int tid = sched->next_tid++;
    tcb_init(idle_tcb, tid, idle_pcb, NULL);

    /* Save into scheduler */
    sched->idle_tcb = idle_tcb;

    return tid;
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

    /* Add the pcb to the pool */
    if (tcb_pool_add_pcb(&(sched->thr_pool), pcb) < 0) return -4;
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

        /* Set current tcb status back to RUNNABLE if it's not WAITING */
        if (sched->cur_tcb->status == RUNNING)
            sched->cur_tcb->status = RUNNABLE;
    }
    return 0;
}

int scheduler_wakeup(scheduler_t *sched){
    return tcb_pool_wakeup(&(sched->thr_pool), scheduler_num_ticks);
}

int scheduler_reap(scheduler_t *sched){
    return tcb_pool_reap(&(sched->thr_pool));
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

    int ret;
    /* Cycle runnable pool and get next tcb to run */
    if ((ret = tcb_pool_get_next_tcb(&(sched->thr_pool), tcbp)) == -2) {
        /* Runnable Pool is empty, run the idle tcb */
        *tcbp = sched->idle_tcb;

        return 0;
    } else if (ret < 0) {
        /* Some other error */
        return -2;
    }

    return 0;
}

