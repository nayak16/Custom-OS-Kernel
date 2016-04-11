/** @file mutex.h
 *  @brief This file defines mutexes.
 *  @author Christopher Wei (cjwei), Aatish Nayak (aatishn)
 *  @bug No known bugs
 */

#ifndef _MUTEX_H
#define _MUTEX_H

/** @brief Defines a mutex struct and type */
typedef struct mutex {
	/** @brief Represents whether or not mutex is free or taken */
    int lock;
    /** @brief Tid of whoever currently holds the mutex */
    int owner;
} mutex_t;

int mutex_init( mutex_t *mp );
void mutex_destroy( mutex_t *mp );
void mutex_lock( mutex_t *mp );
void mutex_unlock( mutex_t *mp );

#endif /* _MUTEX_H */
