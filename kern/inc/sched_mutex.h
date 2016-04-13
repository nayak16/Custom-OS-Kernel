/** @file sched_mutex_t.h
 *  @brief Interface for a scheduler lock
 *  @author Christopher Wei (cjwei), Aatish Nayak (aatishn)
 *  @bug No known bugs
 */

#ifndef _SCHED_MUTEX_
#define _SCHED_MUTEX_

#include <scheduler.h>
#include <mutex.h>
#include <stdbool.h>

/** @brief Defines a mutex struct and type */
typedef struct sched_mutex {
	/** @brief The scheduler to protect */
    scheduler_t *sched;
} sched_mutex_t;

int sched_mutex_init( sched_mutex_t *mp, scheduler_t *sched);
void sched_mutex_destroy( sched_mutex_t *mp);
void sched_mutex_lock( sched_mutex_t *mp );
void sched_mutex_unlock( sched_mutex_t *mp );

#endif /* _SCHED_MUTEX_ */
