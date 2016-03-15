/** @file thr_pool.h
 *  @brief Defines interface for thread pool
 *  @author Christopher Wei (cjwei)
 *  @author Aatish Nayak (aatishn)

 *  @bug No known bugs
 */


#ifndef _THR_POOL_H_
#define _THR_POOL_H_

#include <tcb.h>
#include <ll.h>

/**
 * @brief Struct representing a thread pool
 */
typedef struct thr_pool {
    ll_t pool;
    mutex_t m;
} thr_pool_t;

int thr_pool_init(thr_pool_t *tp);
int thr_pool_add_tcb(thr_pool_t *tp, tcb_t *tcb);
int thr_pool_get_tcb(thr_pool_t *tp, int tid, tcb_t **tcb);
int thr_pool_remove_tcb(thr_pool_t *tp, int tid);


#endif /* _THR_POOL_H_ */
