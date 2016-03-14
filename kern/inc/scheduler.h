/** @file
 *
 */

#include <pcb.h>
#include <tcb.h>

#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

typedef struct scheduler{
    int next_tid;
    int next_pid;
    pcb_t *current_pcb;
    tcb_t *current_tcb;
} scheduler_t;


int scheduler_init(scheduler_t *sched);

int scheduler_add_process(scheduler_t *sched, pcb_t *pcb);

int scheduler_start(scheduler_t *sched);

#endif /* _SCHEDULER_H_ */
