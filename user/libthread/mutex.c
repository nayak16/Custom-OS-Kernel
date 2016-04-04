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
#include <syscall.h>
#include <simics.h>

/* P1 Specific includes */
#include <mutex_type.h>
#include <thread.h>

/** @brief Initializes a mutex
 *  @param mp Pointer to mutex to be intialized
 *  @return 0 on success, negative code on error
 */
int mutex_init( mutex_t *mp ){
    if (mp == NULL) return -1;
    mp->lock = 1;
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
    /* Keep trying to until lock is free and key is accepted */
    while (!xchng(&mp->lock, 0)) {
        /* Yield to next thread to avoid busy waiting */
        yield(-1);
    }
    mp->lock = 0;
    return;
}

/** @brief Unlocks a mutex
 *  @param mp Pointer to mutex to be unlocked
 *  @return Void
 */
void mutex_unlock( mutex_t *mp ){
    xchng(&mp->lock, 1);
    return;
}

