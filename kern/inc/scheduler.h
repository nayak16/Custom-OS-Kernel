/** @file
 *
 */

#include <pcb.h>
#include <tcb.h>
#include <queue.h>
#include <thr_pool.h>

#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

typedef struct scheduler{
    int next_tid;
    int next_pid;
    thr_pool_t thr_pool;

    int cur_tid;
    queue_t runnable_pool;
    queue_t waiting_pool;

} scheduler_t;


int scheduler_init(scheduler_t *sched);

int scheduler_add_process(scheduler_t *sched, pcb_t *pcb);

int scheduler_start(scheduler_t *sched);

#endif /* _SCHEDULER_H_ */

