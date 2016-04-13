/** @file mutex.c
 *  @brief This file implements mutexes.
 *
 *  We use a very simple implementation of mutexes which simply tries to
 *  atomically exchange a 0 into the mutex until it passes. This does not
 *  ensure unbounded waiting unlike methods such as TestAndSet, but also
 *  does not need to know exactly how many threads are running in order to
 *  determine some arbitration method. Indeed a thread could be starved out
 *  due to purely bad luck or a naughty kernel; this is a consideration we
 *  made when deciding to not implement a unbounded waiting algorithm
 *
 *  @author Christopher Wei (cjwei) Aatish Nayak (aatishn)
 *  @bug No known bugs
 */

/* C Standard Lib specific Includes */
#include <stdlib.h>
#include <simics.h>


/* P3 Specific includes */
#include <mutex.h>
#include <kern_internals.h>
#include <thr_helpers.h>

/** @brief Initializes a mutex
 *  @param mp Pointer to mutex to be intialized
 *  @return 0 on success, negative code on error
 */
int mutex_init( mutex_t *mp ){
    if (mp == NULL) return -1;
    mp->lock = 1;
    mp->owner = -1;
    return 0;
}

/** @brief Destroys a mutex
 *  @param mp Pointer to mutex to be destroyed
 *  @return Void
 */
void mutex_destroy( mutex_t *mp ){
    mp->lock = -1;
    return;
}

/** @brief Locks a mutex and hangs until successful
 *  @param mp Pointer to mutex to be locked
 *  @return Void
 */
void mutex_lock( mutex_t *mp ){
    if (!sched.started) return;

    /* Get the current tid */
    int cur_tid;
    if (scheduler_get_current_tid(&sched, &cur_tid) < 0){
        /* somethings wrong with the scheduler? */
        cur_tid = -1;
    }
    /* Keep trying to until lock is free and key is accepted */
    while (!xchng(&mp->lock, 0)) {
        if (thr_kern_yield(mp->owner) < 0) {
            /* Owner is not runnable, yield to scheduler picked thread */
            thr_kern_yield(-1);
        }
    }
    /* after securing the lock, set the owner to yourself so
     * threads that are waiting on this mutex will yield to this
     * thread - in a sense, the thread holding this mutex has
     * gained a higher priority */
    mp->owner = cur_tid;
    return;
}

/** @brief Unlocks a mutex
 *  @param mp Pointer to mutex to be unlocked
 *  @return Void
 */
void mutex_unlock( mutex_t *mp ){
    if (!sched.started) return;
    /* update the owner to -1 so we don't prioritize this thread anymore. There
     * is a slight race condition (which doesn't affect correctness just
     * efficiency when we mutex unlock and then another thread mutex locks but
     * is context switched out before it can set the mutex's owner. In this case
     * it is better to let the scheduler pick a thread to run instead of
     * forcing other threads to yield back to here */
    mp->owner = -1;
    xchng(&mp->lock, 1);
    return;
}

