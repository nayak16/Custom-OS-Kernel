/** @file thr_mgmt_handlers.c
 *  @brief implements thread management syscall handlers
 *
 *  @author Christopher Wei (cjwei)
 *  @author Aatish Nayak (aatishn)
 *  @bug No known bugs.
 */

#include <kern_internals.h>
#include <scheduler.h>

#include <simics.h>

int syscall_gettid_c_handler(){
    int cur_tid;
    if(scheduler_get_current_tid(&sched, &cur_tid) < 0) {
        return -1;
    }
    return cur_tid;
}


int syscall_yield_c_handler(int tid){
    return 0;
}
int syscall_deschedule_c_handler(int *reject){
    // TODO: Check if reject is valid pointer
    if (reject == NULL) return -1;

    /* Atomically check integer pointed to be reject */
    if (xchng(reject, 0) != 0) return 0;

    tcb_t *my_tcb;
    /* Get current tcb */
    if(scheduler_get_current_tcb(&sched, &my_tcb) < 0) return -2;

    /* Deschedule current TCB */
    if (scheduler_deschedule_current_safe(&sched) < 0) return -3;
    /* Loop until current tcb is RUNNABLE again. Allows time for scheduler to context
     * switch into another tcb, and only resume when cur_tcb is put back into the runnable pool */

    int status;
    while(status != RUNNABLE) {
        if(tcb_get_status(my_tcb, &status) < 0) {
            // TODO: Make runnable again
            return -4;
        }
    }

    return 0;
}
int syscall_make_runnable_c_handler(int tid){
    return 0;
}

unsigned int syscall_get_ticks_c_handler(){
    return scheduler_num_ticks;
}

int syscall_sleep_c_handler(int ticks){
    if (ticks < 0) return -1;
    if (ticks == 0) return 0;

    return 0;
}
