/** @file rwlock.c
 *  @brief This file implements readers/writers locks
 *
 *  We have decide on implementing a reader writer lock that gives priority
 *  to the writer. The thought process behind this is that a writer request
 *  means that something in the shared resource has inherently changed and
 *  future reads should wait until the update has occured before reading.
 *  We implement this using a mutex to protect internal values, a condition
 *  variable used to signal sleeping readers and writers, a count to keep
 *  track of the number of readers currently reading (which is used to signal
 *  when it reachs 0) and a writer_locked boolean to prevent further requests
 *  from happening until that writer has been serviced.
 *
 *  @author Christopher Wei (cjwei), Aatish Nayak (aatishn)
 *  @bug No known bugs
 */

#include <cond.h>
#include <mutex.h>
#include <stdbool.h>
#include <stdlib.h>

#include <rwlock.h>
#include <rwlock_type.h>

/* readers/writers lock functions */

/** @brief Initializes a reader writer lock
 *
 *  @param rwlock The pointer to the r/w lock to be initialized
 *  @return 0 on success, -1 on failure
 */
int rwlock_init( rwlock_t *rwlock ){
    if (rwlock == NULL) return -1;
    if (mutex_init(&(rwlock->m)) < 0) return -1;
    if (cond_init(&(rwlock->cv)) < 0) return -1;
    rwlock->count = 0;
    rwlock->writer_locked = false;
    return 0;
}

/** @brief Locks the rwlock
 *
 *  @param rwlock The pointer to the r/w lock to be locked
 *  @param type The integer code defining the type of lock requested
 *  @return Void
 */
void rwlock_lock( rwlock_t *rwlock, int type ){
    if (rwlock == NULL) return;
    if (type != RWLOCK_WRITE && type != RWLOCK_READ)
        panic("Invalid type supplied to rwlock_lock");

    mutex_lock(&(rwlock->m));
    if (rwlock->count < 0)
        panic("Attempted to lock a invalid rwlock");

    if (type == RWLOCK_READ){
        /* wait until a writer has finished using critical section */
        while(rwlock->writer_locked){
            cond_wait(&(rwlock->cv), &(rwlock->m));
        }
        /* increment count to let future writers know we are reading
         * the critical section */
        rwlock->count++;
    } else {
        /* wait until any current writers are finished */
        while (rwlock->writer_locked || rwlock->count > 0){
            cond_wait(&(rwlock->cv), &(rwlock->m));
        }
        rwlock->writer_locked = true;
    }
    mutex_unlock(&(rwlock->m));
}

/** @brief unlocks the rwlock
 *
 *  @param rwlock The pointer to the r/w lock to be locked
 *  @return Void
 */
void rwlock_unlock( rwlock_t *rwlock ){
    if (rwlock == NULL) return;
    mutex_lock(&(rwlock->m));
    if (rwlock->count <= 0 && !rwlock->writer_locked)
        panic("Attempted to unlock rwlock that is already unlocked");
    if (rwlock->writer_locked){
        rwlock->writer_locked = false;
        cond_broadcast(&(rwlock->cv));
    } else {
        rwlock->count--;
        /* since count > 0, everything waiting on cv is a writer, therefore
         * wake up next writer */
        if (rwlock->count == 0) cond_signal(&(rwlock->cv));
    }
    mutex_unlock(&(rwlock->m));
}


/** @brief Destroys the rwlock
 *
 *  @param rwlock The lock to be destroyed
 *  @return Void
 */
void rwlock_destroy( rwlock_t *rwlock ){
    if (rwlock == NULL) return;
    mutex_lock(&(rwlock->m));
    rwlock->count = -1;
    cond_destroy(&(rwlock->cv));
    mutex_unlock(&(rwlock->m));
    mutex_destroy(&(rwlock->m));
}
/** @brief Converts a writer lock to a reader lock
 *
 * @param rwlock The lock to be downgraded
 * @return Void
 */
void rwlock_downgrade( rwlock_t *rwlock){
    if (rwlock == NULL) return;
    mutex_lock(&(rwlock->m));
    if (!rwlock->writer_locked)
        panic("Attempted to downgrade a non-writer thread");
    rwlock->writer_locked = false;
    rwlock->count++;
    cond_broadcast(&(rwlock->cv));
    mutex_unlock(&(rwlock->m));
}

