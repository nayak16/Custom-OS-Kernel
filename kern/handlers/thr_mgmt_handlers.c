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
#include <dispatcher.h>

#include <simics.h>

int syscall_gettid_c_handler(){
    return thr_gettid();
}


int syscall_yield_c_handler(uint32_t old_esp, int tid){
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
    return thr_sleep(ticks);
}
