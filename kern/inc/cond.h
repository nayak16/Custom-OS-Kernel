/** @file cond.h
 *  @brief This file defines the interface for condition variables.
 */

#ifndef _COND_H_
#define _COND_H_

#include <mutex.h>
#include <queue.h>


/** @brief defines a conditional variable struct and type */
typedef struct cond {
	/** @brief The internal conditional variable mutex for it's queue */
    mutex_t m;
    /** @brief The queue of waiting threads */
    queue_t q;
} cond_t;

/* condition variable functions */
int cond_init( cond_t *cv );
void cond_destroy( cond_t *cv );
void cond_wait( cond_t *cv, mutex_t *mp );
void cond_signal( cond_t *cv );
void cond_broadcast( cond_t *cv );

#endif /* _COND_H_ */
