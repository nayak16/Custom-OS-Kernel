/** @file sched_mutex_t.c
 *  @brief Implementation for a scheduler lock
 *  @author Christopher Wei (cjwei), Aatish Nayak (aatishn)
 *  @bug No known bugs
 */
#include <sched_mutex.h>
#include <mutex.h>
#include <x86/asm.h>
#include <simics.h>

/**
 * @brief Initializes a scheduler lock
 *
 * @param mp mutex to init
 * @param sched scheduler_t this lock protects
 *
 * @return 0 on success, negative error code otherwise
 *
 */
int sched_mutex_init( sched_mutex_t *mp, scheduler_t *sched) {
    if (mp == NULL || sched == NULL) return -1;
    mp->sched = sched;

    return 0;
}

/**
 * @brief Locks the scheduler by disabling interrupts only if the
 * scheduler has been started. This is to ensure interrupts are not
 * enabled/disabled unnecessarily while kernel is initializing
 *
 * Disabling interrupts ensures only the current thread has access
 * to the scheduler data structures. This also ensures that the kernel
 * cannot context switch to another thread via a timer interrupt.
 *
 * This function has no effect if the scheduler the lock protects
 * has not been started.
 *
 * @param mp mutex to lock
 *
 * @return void
 *
 */
void sched_mutex_lock( sched_mutex_t *mp ) {
    if (mp == NULL) return;
    if (mp->sched == NULL) return;

    /* Check if scheduler is started */
    if (mp->sched->started) {
        disable_interrupts();
    }
}

/**
 * @brief Unlocks the scheduler by renabling interrupts. This usually
 * follows a sched_mutex_lock(). Renables interrupts so that timer
 * interrupt can fire again and schedule threads to run.
 *
 * This function has no effect if the scheduler the lock protects
 * has not been started.
 *
 * @param mp mutex to lock
 *
 * @return void
 *
 */
void sched_mutex_unlock( sched_mutex_t *mp ) {
    if (mp == NULL) return;
    if (mp->sched == NULL) return;

    /* Check if scheduler is started */
    if (mp->sched->started) {
        enable_interrupts();
    }
}

/**
 * @brief Destroys a sched_mutex_t
 *
 * (Never called but here for completeness)
 *
 * @param mp mutex to destroy
 *
 * @return void
 *
 */
void sched_mutex_destroy( sched_mutex_t *mp) {
    mp->sched = NULL;
}

