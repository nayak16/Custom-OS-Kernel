/** @file sem_type.h
 *  @brief This file defines the type for semaphores.
 *  @author Christopher Wei (cjwei), Aatish Nayak (aatishn)
 *  @bug No known bugs
 */

#ifndef _SEM_TYPE_H
#define _SEM_TYPE_H

#include <mutex_type.h>
#include <ll.h>

/** @brief Defines a semaphore struct and type */
typedef struct sem {
	/** @brief The number of resources currently avaliable from sem */
    int count;
    /** @brief The internal mutex that protects q */
    mutex_t m;
    /** @brief Queue of waiting threads on sem */
    ll_t q;
} sem_t;

#endif /* _SEM_TYPE_H */
