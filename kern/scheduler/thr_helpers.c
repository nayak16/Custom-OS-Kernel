/** @file thr_helpers.c
 *  @brief Implementation of functions for manipulating threads inside the
 *  kernel
 *
 *  These are seperate functions because the kernel as well as syscalls
 *  triggered by user level INTs need these functionalities.
 *
 *  @author Aatish Nayak (aatishn)
 *  @author Christopher Wei (cjwei)
 *  @bug No known bugs.
 */

#include <stdio.h>

#include <scheduler.h>
#include <kern_internals.h>
#include <thr_helpers.h>
#include <dispatcher.h>
#include <tcb.h>


/**
 * @brief Deschedules the current thread
 *
 * @param old_esp stack of the current thread (used to context switch back
 * into thread once it's made runnable again
 * @param reject pointer to integer flag (function returns immediately if it
 * points to a non zero integer)
 *
 *
 * @return Should never return unless an error occurs, in which case a negative
 * error code will be returned
 *
 */
int thr_deschedule(uint32_t old_esp, int *reject) {

    tcb_t *my_tcb;
    /* Get current tcb */
    if(scheduler_get_current_tcb(&sched, &my_tcb) < 0) return -2;

    /* Check if reject is a valid pointer */
    if (pd_get_mapping(&(my_tcb->pcb->pd), (uint32_t) reject, NULL) < 0) {
        return -3;
    }

    /* Atomically check integer pointed to be reject */
    if (xchng(reject, 0) != 0) return 0;

    /* Deschedule current thread */
    if (scheduler_deschedule_current_safe(&sched) < 0) return -3;

    /* Yield to another thread */
    thr_yield(old_esp, -1);

    /* Placate compilter */
    return 0;
}

/**
 * @brief Makes the thread with the specified tid runnable.
 *
 * @param tid tid of thread to make runnable
 *
 * @return 0 on success, negative error code othwerwise
 *
 */
int thr_make_runnable(int tid) {
    if (scheduler_make_runnable_safe(&sched, tid) < 0) return -2;
    return 0;
}

/**
 * @brief Sets the exit status of the current thread
 *
 * @param status status to set the exit status to
 *
 * @return void
 *
 */
void thr_set_status(int status) {
    tcb_t *cur_tcb;
    if (scheduler_get_current_tcb(&sched, &cur_tcb) < 0) {
        return;
    }
    cur_tcb->exit_status = status;
    return;
}

/**
 * @brief Terminates execution of the current thread.
 *
 * If the current thread is the last thread in its process, signal the parent
 * process to collect the exit status and original tid. If the current thread's
 * parent process has died, signal the init process. The thread is then made a
 * zombie so the reaper can reap its resources at a later time. This ensures
 * that vanish isn't unnecessarily inefficient in cleaning up.
 *
 * @return void
 *
 */
void thr_vanish(void) {

    pcb_t *cur_pcb, *parent_pcb;
    tcb_t *cur_tcb;

    /* get current pcb and tcb */
    scheduler_get_current_tcb(&sched, &cur_tcb);
    tcb_get_pcb(cur_tcb, &cur_pcb);

    int exit_status;
    /* Get exit status of exited tcb */
    if (tcb_get_exit_status(cur_tcb, &exit_status) < 0) exit_status = -2;

    int original_tid;
    /* Get the original tid of the original thread of current pcb */
    pcb_get_original_tid(cur_pcb, &original_tid);

    /* from cur_pcb, get parent_pcb */
    if (scheduler_get_pcb_by_pid(&sched, pcb_get_ppid(cur_pcb), &parent_pcb) < 0){

        pcb_t *init_pcb;
        /* Orphan child, so get init pcb from scheduler */
        scheduler_get_init_pcb(&sched, &init_pcb);

        /* Let init know that it has an orphan grandchild, so
         * it can adopt it momentarily */
        pcb_inc_children_s(init_pcb);

        /* Decrement and check num threads value */
        if (pcb_dec_threads_s(cur_pcb) == 0) {
            /* Now signal init to collect grandchild's status */
            pcb_signal_status(init_pcb, exit_status, original_tid);
        }

    } else {
        /* Ensure parent_pcb isn't destroyed while signaling */
        mutex_lock(&(parent_pcb->m));

        /* Decrement and check num threads value */
        if (pcb_dec_threads_s(cur_pcb) == 0) {
            /* signal the status to parent_pcb */
            pcb_signal_status(parent_pcb, exit_status, original_tid);
        }

        /* Release lock */
        mutex_unlock(&(parent_pcb->m));
    }

    /* Indicate exit on console */
    printf("Thread %d exited with status %d\n", cur_tcb->tid, exit_status);

    /* Make current tcb a zombie */
    scheduler_make_current_zombie_safe(&sched);

    /* Yield to another thread */
    if (thr_yield(0,-1) < 0) {
        panic("Error occured while vanishing. Cannot terminate execution \
                of calling thread");
    }
}

/**
 * @brief Makes the current tcb sleep for specified number of ticks
 *
 * @param old_esp Stack of current thread used to switch back into when it
 * wakes up
 * @param ticks Number of ticks the thread should sleep for
 *
 * @return should not return on success, negative error code otherwise
 *
 *
 */
int thr_sleep(uint32_t old_esp, int ticks) {

    /* Make the current tcb sleep */
    if (scheduler_make_current_sleeping_safe(&sched, ticks) < 0) return -3;

    /* Yield to another thread */
    thr_yield(old_esp, -1);

    /* Should never return */
    /* Placate compiler */
    return 0;

}

/**
 * @brief Returns the tid of the current tcb
 *
 * @return tid of current tcb on success, negative error code otherwise
 *
 */
int thr_gettid() {
    int cur_tid;
    if(scheduler_get_current_tid(&sched, &cur_tid) < 0) {
        return -1;
    }
    return cur_tid;
}

/**
 * @brief Yields execution to the thread with the specified tid
 *
 * @param old_esp stack of the current thread used to context switch back
 * into when resuming execution
 *
 * @param tid tid to thread to yield to, -1 if yield to a scheduler
 * selected tcb
 *
 * @return returns 0 on success to calling thread when execution returns,
 * returns a negative error code "immediately" to calling thread if an
 * error occurs
 *
 */
int thr_yield(uint32_t old_esp, int tid) {
    if (tid >= 0) {
        int ret;
        /* Check if specified tcb is runnable */
        if ((ret = scheduler_check_is_runnable(&sched, tid)) < 0) {
            /* Not found */
            return -2;
        } else if (ret == 0) {
            /* tcb is not runnable, yield to -1 */
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


