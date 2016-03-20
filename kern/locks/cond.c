/**
 *  @file cond.c
 *  @brief The implementation for condition variables.
 *
 *  To implement condition variables, we have an internal mutex that protects
 *  a queue. Threads waiting on a condition variable will enqueue all the
 *  information needed to awaken the thread in the cv's queue. One interesting
 *  problem we had while designing cond_wait is that there is a gap between
 *  the time we unlock the condition variable's mutex, thus allowing for other
 *  threads to signal the current thread, and the time we deschedule. We have
 *  to make this "atomic" otherwise our thread could sleep forever. In order
 *  to prevent a cond_signal from signally a about-to-sleep thread, we have
 *  a reject pointer that will essentially reject the deschedule if it is set
 *  due to another thread calling signal. Additionally, we found that some of
 *  the misbehave modes caused sleeping threads to run without a cond_signal
 *  so we wrapped this area in a while(!reject) so it can only awaken from a
 *  cond_signal.
 *
 *  @author Christopher Wei (cjwei), Aatish Nayak (aatishn)
 *  @bug No known bugs
 */
#include <stdlib.h>
#include <syscall.h>

#include <cond.h>
#include <mutex.h>
#include <thread.h>
#include <simics.h>

/** @brief Initializes a condition variable
 *
 *  @param cv Pointer to the condition variable that is to be initialized
 *  @return 0 on success, negative code on failure
 */
int cond_init( cond_t *cv ) {
    if (cv == NULL) return -1;
    if (queue_init(&cv->q) < 0) return -1;
    if (mutex_init(&cv->m) < 0) return -1;
    return 0;
}

/** @brief Destroys a condition variable
 *
 *  @param cv Pointer to the condition variable that is to be destroyed
 *  @return Void
 */
void cond_destroy( cond_t *cv ) {
    if (cv == NULL) return;
    mutex_lock(&(cv->m));
    queue_destroy(&(cv->q));
    mutex_unlock(&(cv->m));
    mutex_destroy(&(cv->m));
}

/** @brief Deschedules current thread until a condition is satisfied
 *
 *  @param cv Pointer to the condition variable that specifies the condition
 *            that is to be satisfied
 *  @param world_mp Pointer to the mutex that protects the global resource
 *  @return Void
 */
void cond_wait( cond_t *cv, mutex_t *world_mp ) {
    if (cv == NULL || world_mp == NULL) return;
    /* grab the internal cond mutex - we can now safely enqueue to cv queue */
    mutex_lock(&(cv->m));

    /* create a new thread struct */
    thread_t thread;
    thread.k_tid = gettid();
    /* reject is another condition that will ensure that only a call to
     * cond_signal can awaken the current thread and any external reasons for
     * the thread awakening are essentially ignored and put back to sleep */
    thread.reject = 0;

    queue_enq(&(cv->q), &thread);
    mutex_unlock(world_mp);
    /* let go of cv mutex; current thread can now be preempted by cond_signal */
    mutex_unlock(&(cv->m));
    /* in the case that cond_signal has occured between mutex_unlock(&(cv->mp))
     * and this check, reject will be 1 so the thread will not be descheduled.
     * if the thread is made runnable for some other reason besides cond_signal,
     * it will simply go back to sleep. */
    while(!(thread.reject))
        deschedule(&(thread.reject));

    /* upon returning, free allocated struct */
    //free(thread);
    mutex_lock(world_mp);
}

/** @brief Signals that a condition has been updated and a single waiting thread
 *         should one exist should be awoken
 *
 *  @param cv Pointer to the condition variable that is to be signaled
 *  @return Void
 */
void cond_signal( cond_t *cv ) {
    if (cv == NULL) return;
    mutex_lock(&(cv->m));
    thread_t *temp;
    /* dequeue cv's linked list into temp */
    if(queue_deq(&(cv->q), (void **)(&temp)) < 0) {
        /* no threads waiting to be signaled */
        mutex_unlock(&(cv->m));
        return;
    }
    /* signal to waiting thread that it should not deschedule itself if it
     * has not already and awaken thread */
    temp->reject = 1;
    make_runnable(temp->k_tid);
    mutex_unlock(&(cv->m));
}


/** @brief Signals that a condition has been updated and all waiting threads
 *         should be signaled if any exist
 *
 *  @param cv Pointer to the condition variable that is to be signaled
 *  @return Void
 */
void cond_broadcast( cond_t *cv ) {
    if (cv == NULL) return;
    mutex_lock(&(cv->m));
    thread_t *thr;
    while(queue_deq(&(cv->q), (void **)(&thr)) == 0){
        thr->reject = 1;
        make_runnable(thr->k_tid);
    }
    mutex_unlock(&(cv->m));
}


