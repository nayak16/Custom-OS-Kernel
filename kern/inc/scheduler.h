/** @file
 *
 */

#include <pcb.h>
#include <tcb.h>
#include <queue.h>
#include <tcb_pool.h>
#include <stdbool.h>

#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

typedef struct scheduler{
    bool started;

    int next_tid;
    int next_pid;
    void *reaper_stack_bot;
    void *reaper_stack_top;

    pcb_t *init_pcb;
    tcb_t *reaper_tcb;
    tcb_t *idle_tcb;

    tcb_pool_t thr_pool;
    tcb_t *cur_tcb;
} scheduler_t;

extern uint32_t scheduler_num_ticks;


int scheduler_init(scheduler_t *sched, void (*reap_func)(void));

int scheduler_add_init_process(scheduler_t *sched, pcb_t *init_pcb);
int scheduler_add_idle_process(scheduler_t *sched, pcb_t *idle_pcb);

int scheduler_add_reaper_proc(scheduler_t *sched,
                              pcb_t *reaper_pcb, void (*reap_func)(void));

int scheduler_add_process(scheduler_t *sched, pcb_t *pcb, uint32_t *regs);
int scheduler_add_new_thread(scheduler_t *sched, uint32_t *regs);

int scheduler_add_process_safe(scheduler_t *sched,
                               pcb_t *pcb, uint32_t *regs);

int scheduler_start(scheduler_t *sched);

int scheduler_get_next_tcb(scheduler_t *sched, tcb_t **tcb);
int scheduler_get_tcb_by_tid(scheduler_t *sched,
                             int target_tid, tcb_t **tcbp);

int scheduler_get_pcb_by_pid(scheduler_t *sched,
                             int target_pid, pcb_t **pcbp);

int scheduler_get_init_pcb(scheduler_t *sched, pcb_t **init_pcbp);
int scheduler_get_idle_tcb(scheduler_t *sched, tcb_t **idle_tcbp);

int scheduler_check_is_runnable(scheduler_t *sched, int target_tid);

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

