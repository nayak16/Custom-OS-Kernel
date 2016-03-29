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
#include <thr_helpers.h>
#include <dispatcher.h>
#include <tcb.h>

int thr_deschedule(uint32_t old_esp, int *reject) {

    // TODO: Check if reject is valid pointer
    if (reject == NULL) return -1;

    /* Atomically check integer pointed to be reject */
    if (xchng(reject, 0) != 0) return 0;

    tcb_t *my_tcb;
    /* Get current tcb */
    if(scheduler_get_current_tcb(&sched, &my_tcb) < 0) return -2;

    if (scheduler_deschedule_current_safe(&sched) < 0) return -3;

    /* Yield to another thread */
    thr_yield(old_esp, -1);

    return 0;
}

int thr_make_runnable(int tid) {
    if (scheduler_make_runnable_safe(&sched, tid) < 0) return -3;
    return 0;
}

int thr_sleep(uint32_t old_esp, int ticks) {

    /* Make the current tcb sleep */
    if (scheduler_make_current_sleeping_safe(&sched, ticks) < 0) return -3;

    /* Yield to another thread */
    thr_yield(old_esp, -1);

    return 0;

}

int thr_gettid() {
    int cur_tid;
    if(scheduler_get_current_tid(&sched, &cur_tid) < 0) {
        return -1;
    }
    return cur_tid;
}


int thr_yield(uint32_t old_esp, int tid) {
    if (tid >= 0) {
        int ret;
        /* Check if specified tcb is runnable */
        if ((ret = scheduler_check_is_runnable(&sched, tid)) < 0) {
            /* Not found */
            return -2;
        } else if (ret == 0) {
            /* tcb is not runnable */
            return -3;
        }
        /* Success, tcb is runnable */
    }
    /* Context switch */
    uint32_t new_esp = context_switch_safe(old_esp, tid);
    /* Restore context with new selected esp */
    restore_context(new_esp);

    return 0;
}


