/** @file
 *
 */

#include <pcb.h>
#include <tcb.h>
#include <queue.h>
#include <tcb_pool.h>

#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

typedef struct scheduler{
    int next_tid;
    int next_pid;
    tcb_t *idle_tcb;

    tcb_pool_t thr_pool;
    tcb_t *cur_tcb;
} scheduler_t;

extern uint32_t scheduler_num_ticks;


int scheduler_init(scheduler_t *sched);

int scheduler_add_idle_process(scheduler_t *sched, pcb_t *idle_pcb);
int scheduler_add_process(scheduler_t *sched, pcb_t *pcb, uint32_t *regs);
int scheduler_add_process_safe(scheduler_t *sched,
                               pcb_t *pcb, uint32_t *regs);

int scheduler_start(scheduler_t *sched);

int scheduler_get_next_tcb(scheduler_t *sched, tcb_t **tcb);
int scheduler_get_tcb_by_tid(scheduler_t *sched,
                             int target_tid, tcb_t **tcbp);

int scheduler_set_running_tcb(scheduler_t *sched,
                                tcb_t *tcb, uint32_t *new_esp);

int scheduler_get_current_tcb(scheduler_t *sched, tcb_t **tcb);

int scheduler_get_current_pcb(scheduler_t *sched, pcb_t **pcb);

int scheduler_deschedule_current_safe(scheduler_t *sched);

int scheduler_defer_current_tcb(scheduler_t *sched, uint32_t old_esp);

int scheduler_get_current_tid(scheduler_t *sched, int *tidp);

#endif /* _SCHEDULER_H_ */

