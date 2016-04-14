/** @file scheduler.h
 *  @brief defines the interface for a scheduler
 *
 *  @author Christopher Wei (cjwei)
 *  @author Aatish Nayak (aatishn)
 */

#include <pcb.h>
#include <tcb.h>
#include <queue.h>
#include <tcb_pool.h>
#include <stdbool.h>

#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

typedef struct scheduler{
    /** @brief whether or not a scheduler has started yet */
    bool started;

    /** @brief the number of ticks since the scheduler has started */
    int num_ticks;
    /** @brief the next tid to create */
    int next_tid;
    /** @brief the next pid to create */
    int next_pid;
    /** @brief the stack bot of a reaper thread */
    void *reaper_stack_bot;
    /** @brief the stack top of a reaper thread */
    void *reaper_stack_top;

    /** @brief the init pcb */
    pcb_t *init_pcb;
    /** @brief the reaper pcb */
    tcb_t *reaper_tcb;
    /** @brief the idle pcb */
    tcb_t *idle_tcb;

    /** @brief the thread pool */
    tcb_pool_t thr_pool;
    /** @brief the current running tcb */
    tcb_t *cur_tcb;
} scheduler_t;

extern uint32_t scheduler_num_ticks;


int scheduler_init(scheduler_t *sched, void (*reap_func)(void));

int scheduler_add_process(scheduler_t *sched, pcb_t *pcb, uint32_t *regs);
int scheduler_add_new_thread(scheduler_t *sched, uint32_t *regs);


int scheduler_start(scheduler_t *sched);

int scheduler_get_next_tcb(scheduler_t *sched, tcb_t **tcb);
int scheduler_get_tcb_by_tid(scheduler_t *sched,
                             int target_tid, tcb_t **tcbp);

int scheduler_get_pcb_by_pid(scheduler_t *sched,
                             int target_pid, pcb_t **pcbp);

int scheduler_get_init_pcb(scheduler_t *sched, pcb_t **init_pcbp);
int scheduler_get_idle_tcb(scheduler_t *sched, tcb_t **idle_tcbp);


int scheduler_check_is_runnable(scheduler_t *sched, int target_tid);

int scheduler_set_running_tcb(scheduler_t *sched,
                                tcb_t *tcb, uint32_t *new_esp);

int scheduler_get_current_tcb(scheduler_t *sched, tcb_t **tcb);

int scheduler_get_current_pcb(scheduler_t *sched, pcb_t **pcb);

int scheduler_deschedule_current_safe(scheduler_t *sched);
int scheduler_make_runnable_safe(scheduler_t *sched, int tid);
int scheduler_make_current_sleeping_safe(scheduler_t *sched, int ticks);
int scheduler_make_current_zombie_safe(scheduler_t *sched);
int scheduler_cleanup_current_safe(scheduler_t *sched);

int scheduler_wakeup(scheduler_t *sched);
int scheduler_reap(scheduler_t *sched);

int scheduler_defer_current_tcb(scheduler_t *sched, uint32_t old_esp);

int scheduler_get_current_tid(scheduler_t *sched, int *tidp);

#endif /* _SCHEDULER_H_ */

