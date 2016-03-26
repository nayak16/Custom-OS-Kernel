/** @file tcb_pool.h
 *  @brief Defines interface for a thread control block pool
 *
 *  @author Christopher Wei (cjwei)
 *  @author Aatish Nayak (aatishn)

 *  @bug No known bugs
 */


#ifndef _TCB_POOL_H_
#define _TCB_POOL_H_

#include <ll.h>
#include <ht.h>
#include <tcb.h>

#define TABLE_SIZE 64
/**
 * @brief Struct representing a thread pool
 */
typedef struct tcb_pool {
    ll_t runnable_pool;
    ht_t threads;
    ll_t waiting_pool;
} tcb_pool_t;

int tcb_pool_init(tcb_pool_t *tp);
int tcb_pool_add_runnable_tcb(tcb_pool_t *tp, tcb_t *tcb);
int tcb_pool_make_runnable(tcb_pool_t *tp, int tid);
int tcb_pool_make_waiting(tcb_pool_t *tp, int tid);

int tcb_pool_get_next_tcb(tcb_pool_t *tp, tcb_t **next_tcbp);
int tcb_pool_find_tcb(tcb_pool_t *tp, int tid, tcb_t **tcbp);
int tcb_pool_remove_tcb(tcb_pool_t *tp, int tid);


#endif /* _TCB_POOL_H_ */
