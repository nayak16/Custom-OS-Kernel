/** @file
 *
 */

#include <x86/cr.h>
#include <x86/asm.h>

#include <pcb.h>
#include <tcb.h>
#include <dispatcher.h>
#include <kern_internals.h>

#include <simics.h>


/**
 * @brief Disables interrupts and does a context switch (defined below)
 *
 * @param old_esp esp of the thread you are context switching out of
 * @param target_tid tid of thread to switch into (-1 if scheduler should
 * determine thread)
 *
 * @return esp of thread that is to be switched into
 *
 */
uint32_t context_switch_safe(uint32_t old_esp, int target_tid) {
    sched_mutex_lock(&sched_lock);
    uint32_t new_esp = context_switch(old_esp, target_tid);
    sched_mutex_unlock(&sched_lock);
    return new_esp;
}

/**
 * @brief Saves the old_esp into the current running tcb, gets the next
 * tcb to run, and sets it to running
 *
 * @param old_esp esp of the thread you are context switching out of
 * @param target_tid tid of thread to switch into (-1 if scheduler should
 * determine thread)
 *
 * @return esp of thread that is to be switched into
 *
 */
uint32_t context_switch(uint32_t old_esp, int target_tid) {

    /* Defer current TCB */
    if (scheduler_defer_current_tcb(&sched, old_esp) < 0) {
        panic("Cannot defer current running thread. \
                Scheduler is corrupted and cannot context switch");
    }

    tcb_t *next_tcb;
    uint32_t new_esp;
    /* Switch to scheduler selected tid */
    if (target_tid < 0) {
        /* Get next available TCB */
        if (scheduler_get_next_tcb(&sched, &next_tcb) < 0) {
            panic("Scheduler is corrupted and cannot context switch!");
        }

    } else {
        /* Get desired tcb */
        if (scheduler_get_tcb_by_tid(&sched,
                                        target_tid, &next_tcb)) {
            lprintf("Thread %d does not exist! Running the idle thread...,", target_tid);
            if (scheduler_get_idle_tcb(&sched, &next_tcb) < 0) {
                panic("Scheduler is corruped, cannot get idle thread!");
            }
        }
    }

    /* Set current running tcb and get new esp */
    if (scheduler_set_running_tcb(&sched, next_tcb, &new_esp) < 0) {
        lprintf("Couldnt set running tcb!!");
        panic("Error trying to run Thread %d. \
                Cannot context switch...", next_tcb->tid);
    }
    return new_esp;

}

