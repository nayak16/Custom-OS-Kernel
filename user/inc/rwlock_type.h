/** @file rwlock_type.h
 *  @brief This file defines the type for reader/writer locks.
 *  @author Christopher Wei (cjwei), Aatish Nayak (aatishn)
 *  @bug No known bugs
 */

#ifndef _RWLOCK_TYPE_H
#define _RWLOCK_TYPE_H

#include <mutex_type.h>
#include <cond_type.h>
#include <stdbool.h>

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

#endif /* _RWLOCK_TYPE_H */
