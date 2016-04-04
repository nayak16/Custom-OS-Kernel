/** @file mutex_type.h
 *  @brief This file defines the type for mutexes.
 *  @author Christopher Wei (cjwei), Aatish Nayak (aatishn)
 *  @bug No known bugs
 */

#ifndef _MUTEX_TYPE_H
#define _MUTEX_TYPE_H

/* C Standard Lib includes */
#include <stdbool.h>

/** @brief Defines a mutex struct and type */
typedef struct mutex {
	/** @brief Represents whether or not mutex is free or taken */
    int lock;
} mutex_t;

#endif /* _MUTEX_TYPE_H */
