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
} thr_pool_t;

int add_tcb(thr_pool_t *tp, tcb_t *tcb);
int get_tcb(thr_pool_t *tp, int tid, tcb_t **tcb);
int remove_tcb(thr_pool_t, int tid);


#endif /* _THR_POOL_H_ */
