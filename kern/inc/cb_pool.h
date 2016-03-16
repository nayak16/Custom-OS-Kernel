/** @file cb_pool.h
 *  @brief Defines interface for a control block pool
 *  @author Christopher Wei (cjwei)
 *  @author Aatish Nayak (aatishn)

 *  @bug No known bugs
 */


#ifndef _THR_POOL_H_
#define _THR_POOL_H_

#include <ll.h>
#include <mutex.h>

/**
 * @brief Struct representing a cbead pool
 */
typedef struct cb_pool {
    ll_t pool;
    mutex_t m;
} cb_pool_t;

int cb_pool_init(cb_pool_t *cp);
int cb_pool_add_cb(cb_pool_t *cp, void *cb);
int cb_pool_get_cb(cb_pool_t *cp, int id, void **cb);
int cb_pool_remove_cb(cb_pool_t *cp, int id);


#endif /* _THR_POOL_H_ */
