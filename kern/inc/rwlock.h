/** @file rwlock.h
 *  @brief This file defines the type for reader/writer locks.
 *  @author Christopher Wei (cjwei), Aatish Nayak (aatishn)
 *  @bug No known bugs
 */

#ifndef _RWLOCK_H_
#define _RWLOCK_H_

#include <mutex.h>
#include <cond.h>
#include <stdbool.h>

#define RWLOCK_READ  0
#define RWLOCK_WRITE 1


/** @brief Defines a struct and type for reader/writer lock */
typedef struct rwlock {
    /** @brief Internal mutex protecting the actual lock */
    mutex_t m;
    /** @brief Condition variable used to signal writer */
    cond_t cv;
    /** @brief Number of readers currently in critical section */
    int count;
    /** @brief Whether or not a writer holds the lock */
    bool writer_locked;
} rwlock_t;

/* readers/writers lock functions */
int rwlock_init( rwlock_t *rwlock );
void rwlock_lock( rwlock_t *rwlock, int type );
void rwlock_unlock( rwlock_t *rwlock );
void rwlock_destroy( rwlock_t *rwlock );
void rwlock_downgrade( rwlock_t *rwlock);

#endif /* _RWLOCK_H_ */

