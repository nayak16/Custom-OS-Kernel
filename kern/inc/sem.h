/** @file sem.h
 *  @brief This file defines the interface to semaphores
 */

#ifndef SEM_H
#define SEM_H

#include <mutex.h>
#include <queue.h>

/** @brief Defines a semaphore struct and type */
typedef struct sem {
	/** @brief The number of resources currently avaliable from sem */
    int count;
    /** @brief The internal mutex that protects q */
    mutex_t m;
    /** @brief Queue of waiting threads on sem */
    queue_t q;
} sem_t;

/* semaphore functions */
int sem_init( sem_t *sem, int count );
void sem_wait( sem_t *sem );
void sem_signal( sem_t *sem );
void sem_destroy( sem_t *sem );

#endif /* SEM_H */
