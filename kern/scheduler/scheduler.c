/** @file
 *
 */

#include <stdlib.h>
#include <stdbool.h>

#include <x86/asm.h>
#include <x86/seg.h>
#include <pcb.h>
#include <tcb.h>
#include <scheduler.h>
#include <dispatcher.h>
#include <queue.h>
#include <circ_buffer.h>
#include <tcb_pool.h>
#include <kern_internals.h>
/* pdbr */
#include <special_reg_cntrl.h>
/* set_esp0 */
#include <x86/cr.h>

#include <simics.h>


/**
 * @brief Adds the OS/shell init process to the scheduler
 *
 * Saves the pcb so future vanished threads can report to this process
 *
 * @param sched Scheduler to add to
 * @param init_pcb Pcb containing the init program
 *
 * @return 0 on success, -1 on error
 */
int scheduler_add_init_process(scheduler_t *sched, pcb_t *init_pcb) {
    if (sched == NULL || init_pcb == NULL) return -1;

    sched->init_pcb = init_pcb;

    if (scheduler_add_process(sched, init_pcb, NULL) < 0) return -2;

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
 * @brief Initializes a reaper process and adds it to the runnable pool.
 *
 * Since the reaper will exclusively be running in kernel mode, it needs
 * different initial register values than a normal thread that runs in user
 * and kernel mode. A scheduler malloced reaper stack is used by the reaper
 * thread while running.
 *
 * @param sched scheduler_t to add reaper process to
 * @param reaper_pcb pcb that has the reaper program
 * @param reap_func function that the reaper will infinitely loop in
 *
 * @return 0 on success, negative error code otherwise
 *
 */
int scheduler_add_reaper_proc(scheduler_t *sched,
                              pcb_t *reaper_pcb, void (*reap_func)(void)) {
    if (sched == NULL || reaper_pcb == NULL) return -1;

    uint32_t regs[REGS_SIZE];

    /* Construct reg values */
    regs[SS_IDX] = SEGSEL_KERNEL_DS;
    regs[ESP_IDX] = (uint32_t) sched->reaper_stack_top;
    regs[EFLAGS_IDX] = get_user_eflags();
    regs[CS_IDX] = SEGSEL_KERNEL_CS;
    regs[EIP_IDX] = (uint32_t) reap_func;
    regs[ECX_IDX] = 0;
    regs[EDX_IDX] = 0;
    regs[EBX_IDX] = 0;
    regs[EBP_IDX] = (uint32_t) sched->reaper_stack_top;
    regs[ESI_IDX] = 0;
    regs[EDI_IDX] = 0;
    regs[DS_IDX] = SEGSEL_KERNEL_DS;
    regs[ES_IDX] = SEGSEL_KERNEL_DS;
    regs[FS_IDX] = SEGSEL_KERNEL_DS;
    regs[GS_IDX] = SEGSEL_KERNEL_DS;

    /* Create reaper tcb */
    tcb_t *reaper_tcb = malloc(sizeof(tcb_t));
    if (reaper_tcb == NULL) return -2;
    int tid = sched->next_tid++;

    /* Init new tcb */
    if (tcb_init(reaper_tcb, tid, reaper_pcb, regs) < 0) {
        free(reaper_tcb);
        return -3;
    }

    /* Save reaper tcb */
    sched->reaper_tcb = reaper_tcb;

    /* Add the pcb to the pool */
    if (tcb_pool_add_pcb_safe(&(sched->thr_pool), reaper_pcb) < 0) return -4;
    /* Add a runnable tcb to pool */
    if (tcb_pool_add_runnable_tcb_safe(&(sched->thr_pool),
                                        reaper_tcb) < 0) return -3;

    return 0;
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
int scheduler_init(scheduler_t *sched, void (*reap_func)(void)){
    if (sched == NULL) return -1;
    sched->started = false;

    sched->num_ticks = 0;
    sched->next_tid = 0;
    sched->next_pid = 0;
    sched->cur_tcb = NULL;

    /* Malloc a cleanup_stack */
    sched->reaper_stack_bot = malloc(4*PAGE_SIZE);
    sched->reaper_stack_top = (void*)((uint32_t)sched->reaper_stack_bot
                                        + (uint32_t) 4*PAGE_SIZE);


    /* Init tcb pool */
    if (tcb_pool_init(&(sched->thr_pool)) < 0) return -2;

    pcb_t *idle_pcb = malloc(sizeof(pcb_t));
    pcb_init(idle_pcb);

    /* Create, init, and add reaper process */
    pcb_t *reaper_pcb = malloc(sizeof(pcb_t));
    pcb_init(reaper_pcb);

    /* Create init program */
    pcb_t *init_pcb = malloc(sizeof(pcb_t));
    pcb_init(init_pcb);

    /* Set pdbr to idle pd so pcb load prog loads to correct pd */
    set_pdbr((uint32_t) pd_get_base_addr(&idle_pcb->pd));

    enable_pge();
    enable_paging();

    /* Load idle program */
    pcb_load_prog(idle_pcb, "idle", 0, NULL);
    /* Add idle program to scheduler */
    scheduler_add_idle_process(sched, idle_pcb);

    /* Add reaper process to scheduler */
    scheduler_add_reaper_proc(sched, reaper_pcb, reap_func);

    /* Set pdbr to init pd so pcb load prog loads into correct pd */
    set_pdbr((uint32_t) pd_get_base_addr(&init_pcb->pd));
    /* Load the init program into an init pcb */
    pcb_load_prog(init_pcb, "init", 0, NULL);
    /* Add init process to scheduler */
    scheduler_add_init_process(sched, init_pcb);
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
    if (sched == NULL || sched->cur_tcb == NULL) return -1;
    *tidp = sched->cur_tcb->tid;
    return 0;
}


int scheduler_get_idle_tcb(scheduler_t *sched, tcb_t **idle_tcbp) {
    if (sched == NULL || idle_tcbp == NULL) return -1;

    *idle_tcbp = sched->idle_tcb;

    if (*idle_tcbp == NULL) return -2;
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
 * @brief Gets the pcb with the specified pid
 *
 * @param sched Scheduler to pcb from
 * @param target_pid pid to look for
 * @param tcbp Address to store pointer to pcb
 *
 * @return 0 on success, negative error code otherwise
 */
int scheduler_get_pcb_by_pid(scheduler_t *sched,
                             int target_pid, pcb_t **pcbp) {
    if (sched == NULL || pcbp == NULL) return -1;

    if (tcb_pool_find_pcb(&(sched->thr_pool), target_pid, pcbp) < 0) return -2;

    return 0;
}

int scheduler_get_init_pcb(scheduler_t *sched, pcb_t **pcbp) {
    if (sched == NULL || pcbp == NULL) return -1;
    if (sched->init_pcb == NULL) return -2;

    *pcbp = sched->init_pcb;
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
int scheduler_get_current_pcb(scheduler_t *sched, pcb_t **pcbp) {

    if (sched == NULL) return -1;
    *pcbp = sched->cur_tcb->pcb;
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
    sched_mutex_lock(&sched_lock);
    int status = scheduler_deschedule_current(sched);
    sched_mutex_unlock(&sched_lock);
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
    sched_mutex_lock(&sched_lock);
    int status = scheduler_make_runnable(sched, tid);
    sched_mutex_unlock(&sched_lock);
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
    /* Check for overflow (should not happen for several years) */
    if (sched->num_ticks + ticks < sched->num_ticks){
        panic("You've been running ShrekOS for several continuous years \
                Please restart your machine before continuing.");
    }
    sched->cur_tcb->t_wakeup = sched->num_ticks+ticks;

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
    sched_mutex_lock(&sched_lock);
    int status = scheduler_make_current_sleeping(sched, ticks);
    sched_mutex_unlock(&sched_lock);
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
    /* Make current tcb NULL */
    sched->cur_tcb = NULL;

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
    sched_mutex_lock(&sched_lock);
    int status = scheduler_make_current_zombie(sched);
    sched_mutex_unlock(&sched_lock);
    return status;
}

/**
 * @brief Gets the current running tcb from the scheduler
 *
 * @param sched Scheduler to get current tcb from
 * @param tcbp Address to save current tcb
 *
 * 0 on success, negative error code otherwise
 */
int scheduler_get_current_tcb(scheduler_t *sched, tcb_t **tcbp) {
    if (sched == NULL) return -1;
    *tcbp = sched->cur_tcb;
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

    /* Get next tid */
    int tid = sched->next_tid++;

    /* Add a new tcb to run the pcb*/
    tcb_t *new_tcb = malloc(sizeof(tcb_t));
    if (new_tcb == NULL) return -2;

    /* Init new tcb */
    if (tcb_init(new_tcb, tid, pcb, regs) < 0) return -3;

    /* Set the original tid of the pcb */
    pcb_set_original_tid(pcb, tid);
    /* Inc num threads in pcb */
    pcb_inc_threads_s(pcb);

    /* Add the pcb to the pool */
    if (tcb_pool_add_pcb_safe(&(sched->thr_pool), pcb) < 0) return -4;
    /* Add a runnable tcb to pool */
    if (tcb_pool_add_runnable_tcb_safe(&(sched->thr_pool), new_tcb) < 0) return -3;

    /* Return tid of tcb added */
    return tid;
}

int scheduler_add_new_thread(scheduler_t *sched, uint32_t *regs) {
    if (sched == NULL) return -1;

    /* Get next tid */
    int tid = sched->next_tid++;

    /* Add a new tcb to run the pcb*/
    tcb_t *new_tcb = malloc(sizeof(tcb_t));
    if (new_tcb == NULL) return -2;

    /* Init new tcb */
    if (tcb_init(new_tcb, tid, sched->cur_tcb->pcb, regs) < 0) return -3;

    /* Inc num threads in pcb */
    pcb_inc_threads_s(sched->cur_tcb->pcb);
    /* Safely add a runnable tcb to pool */
    if (tcb_pool_add_runnable_tcb_safe(&(sched->thr_pool), new_tcb) < 0) return -3;

    /* Return tid of tcb added */
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
    sched->started = true;
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
    if (sched->cur_tcb != NULL) {

        /* Save k_stack esp */
        sched->cur_tcb->tmp_k_stack = (uint32_t *)old_esp;

        /* Set current tcb status back to RUNNABLE if it's not WAITING */
        if (sched->cur_tcb->status == RUNNING)
            sched->cur_tcb->status = RUNNABLE;
    }
    return 0;
}

int scheduler_wakeup(scheduler_t *sched){
    return tcb_pool_wakeup(&(sched->thr_pool), sched->num_ticks);
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
 * @return 0 on success, negative code otherwise
 */
int scheduler_get_next_tcb(scheduler_t *sched, tcb_t **tcbp) {
    if (sched == NULL || tcbp == NULL) return -1;

    /* Cycle runnable pool and get next tcb to run */
    if (tcb_pool_get_next_tcb(&(sched->thr_pool), tcbp) < 0) {
        /* Runnable Pool is empty, or some other error occured
         * run the idle tcb */
        *tcbp = sched->idle_tcb;
    }

    return 0;
}

