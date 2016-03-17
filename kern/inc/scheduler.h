/** @file
 *
 */

#include <pcb.h>
#include <tcb.h>
#include <queue.h>
#include <cb_pool.h>

#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

typedef struct scheduler{
    int next_tid;
    int next_pid;
    cb_pool_t thr_pool;
    cb_pool_t process_pool;

    int cur_tid;
    int cur_pid;
    queue_t runnable_pool;
    queue_t waiting_pool;

} scheduler_t;


int scheduler_init(scheduler_t *sched);

int scheduler_add_process(scheduler_t *sched, pcb_t *pcb, uint32_t *regs);

int scheduler_start(scheduler_t *sched);

int scheduler_get_next_tcb(scheduler_t *sched, tcb_t **tcb);
int scheduler_set_running_tcb(scheduler_t *sched, tcb_t *tcb, uint32_t *new_esp);

int scheduler_copy_current_pcb(scheduler_t *sched, uint32_t *regs);

int scheduler_save_running_tcb(scheduler_t *sched, uint32_t old_esp);

int scheduler_get_current_tid(scheduler_t *sched, int *tidp);

#endif /* _SCHEDULER_H_ */

