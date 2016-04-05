/** @file cond_type.h
 *  @brief This file defines the type for condition variables.
 *  @author Christopher Wei (cjwei), Aatish Nayak (aatishn)
 *  @bug No known bugs
 */

#ifndef _COND_TYPE_H
#define _COND_TYPE_H

#include <ll.h>
#include <mutex_type.h>

/** @brief defines a conditional variable struct and type */
typedef struct cond {
	/** @brief The internal conditional variable mutex for it's queue */
    mutex_t m;
    /** @brief The queue of waiting threads */
    ll_t q;
} cond_t;

#endif /* _COND_TYPE_H */
