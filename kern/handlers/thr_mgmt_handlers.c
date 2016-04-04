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

#include <string.h>
#include <ureg.h>

#include <simics.h>

int syscall_gettid_c_handler(){
    return thr_gettid();
}


int syscall_yield_c_handler(uint32_t old_esp, int tid){
    return thr_yield(old_esp, tid);
}
int syscall_deschedule_c_handler(uint32_t old_esp, int *reject){
   return thr_deschedule(old_esp, reject);
}

int syscall_make_runnable_c_handler(int tid){
    return thr_make_runnable(tid);
}

unsigned int syscall_get_ticks_c_handler(){
    return scheduler_num_ticks;
}

int syscall_sleep_c_handler(uint32_t old_esp, int ticks){
    if (ticks < 0) return -1;
    if (ticks == 0) return 0;
    return thr_sleep(old_esp, ticks);
}

int syscall_swexn_c_handler(void *esp3,
        void (*eip)(void *arg, ureg_t *ureg),
        void *arg, ureg_t *newureg, uint32_t *stack){
    /* get current tcb */
    tcb_t *cur_tcb;
    if (scheduler_get_current_tcb(&sched, &cur_tcb) < 0) return -1;

    /* check if we are either installing or uninstalling a handler */
    if (esp3 == NULL || eip == NULL){
        /* deregister any custom handlers if any */
        if (tcb_deregister_swexn_handler(cur_tcb, NULL, NULL, NULL) < 0){
            //undo newureg
            return -2;
        }
    } else {
        // check for invalid esp or eip values
        if (tcb_register_swexn_handler(cur_tcb, esp3, eip, arg) < 0){
            //undo newureg
            return -3;
        }
    }
    if (newureg != NULL){
        // check if newureg has any values that would crash the kernel
        // if newureg not valid return negative integer code
        restore_context((uint32_t)&(newureg->ds));
    }
    return 0;
}
