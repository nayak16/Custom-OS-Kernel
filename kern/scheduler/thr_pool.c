/** @file thr_pool.c
 *  @brief Implementation for thread pool
 *  @author Christopher Wei (cjwei)
 *  @author Aatish Nayak (aatishn)

 *  @bug No known bugs
 */

#include <thr_pool.h>
#include <stdlib.h>

/** @brief */
int thr_pool_add_tcb(thr_pool_t *tp, tcb_t *tcb){
    if (tp == NULL || tcb == NULL) return -1;
    mutex_lock(&(tp->m));
    int status = ll_add(&(tp->pool), (void *)(tcb));
    mutex_unlock(&(tp->m));
    return status;
}

void *get_tid_from_tcb(void *tcb){
    if (tcb == NULL) return (void *)(-1);
    return (void *)(((tcb_t *)tcb)->tid);
}

/** @brief */
int thr_pool_get_tcb(thr_pool_t *tp, int tid, tcb_t **tcb){
    if (tp == NULL || tcb == NULL || tid < 0) return -1;

    mutex_lock(&(tp->m));
    int status = ll_find(&(tp->pool), &get_tid_from_tcb, (void *)tid, (void*)tcb);
    mutex_unlock(&(tp->m));
    return status;
}

/** @brief */
int thr_pool_remove_tcb(thr_pool_t *tp, int tid){
    if (tp == NULL || tid == -1) return -1;
    mutex_lock(&(tp->m));
    int status = ll_remove(&(tp->pool), &get_tid_from_tcb, (void *) tid);
    mutex_unlock(&(tp->m));
    return status;
}


