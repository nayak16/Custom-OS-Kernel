/** @file thr_mgmt_handlers.c
 *  @brief implements thread management syscall handlers
 *
 *  @author Christopher Wei (cjwei)
 *  @author Aatish Nayak (aatishn)
 *  @bug No known bugs.
 */

#include <kern_internals.h>
#include <scheduler.h>

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
    return 0;
}
int syscall_make_runnable_c_handler(int tid){
    return 0;
}


