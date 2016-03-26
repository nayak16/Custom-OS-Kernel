/** @file thr_mgmt_handlers.c
 *  @brief implements thread management syscall handlers
 *
 *  @author Christopher Wei (cjwei)
 *  @author Aatish Nayak (aatishn)
 *  @bug No known bugs.
 */

#include <kern_internals.h>
#include <scheduler.h>
#include <thr_helpers.h>

#include <simics.h>

int syscall_gettid_c_handler(){
    return thr_gettid();
}


int syscall_yield_c_handler(int tid){
    return 0;
}
int syscall_deschedule_c_handler(int *reject){
   return thr_deschedule(reject);
}

int syscall_make_runnable_c_handler(int tid){
    return thr_make_runnable(tid);
}

unsigned int syscall_get_ticks_c_handler(){
    return scheduler_num_ticks;
}

int syscall_sleep_c_handler(int ticks){
    if (ticks < 0) return -1;
    if (ticks == 0) return 0;

    return 0;
}
