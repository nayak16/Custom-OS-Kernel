/** @file thr_helpers.c
 *  @brief Implementation of functions for manipulating threads inside the
 *  kernel
 *
 *  @author Aatish Nayak (aatishn)
 *  @author Christopher Wei (cjwei)
 *  @bug No known bugs.
 */

#include <scheduler.h>
#include <kern_internals.h>
#include <tcb.h>

int thr_deschedule(int *reject) {

    // TODO: Check if reject is valid pointer
    if (reject == NULL) return -1;

    /* Atomically check integer pointed to be reject */
    if (xchng(reject, 0) != 0) return 0;

    tcb_t *my_tcb;
    /* Get current tcb */
    if(scheduler_get_current_tcb(&sched, &my_tcb) < 0) return -2;

    if (scheduler_deschedule_current_safe(&sched) < 0) return -3;
    /* Loop until current tcb is RUNNABLE again. Allows time for scheduler to context
     * switch into another tcb, and only resume when cur_tcb is put back into the runnable pool */
    int status;
    do {
        if(tcb_get_status(my_tcb, &status) < 0) return -4;
    } while (status != RUNNING);

    return 0;
}

int thr_make_runnable(int tid) {
    if (scheduler_make_runnable_safe(&sched, tid) < 0) return -3;
    return 0;

}

int thr_sleep(int ticks) {
    tcb_t *my_tcb;
    if(scheduler_get_current_tcb(&sched, &my_tcb) < 0) return -2;

    if (scheduler_make_current_sleeping_safe(&sched, ticks) < 0) return -3;

    int status;
    do {
        if(tcb_get_status(my_tcb, &status) < 0) return -4;
    } while (status != RUNNING);

    return 0;

}

int thr_gettid() {
    int cur_tid;
    if(scheduler_get_current_tid(&sched, &cur_tid) < 0) {
        return -1;
    }
    return cur_tid;
}


int thr_yield(int tid);


