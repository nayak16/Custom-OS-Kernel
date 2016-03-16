/** @file cb_pool.c
 *  @brief Implementation for cbead pool
 *  @author Christopher Wei (cjwei)
 *  @author Aatish Nayak (aatishn)

 *  @bug No known bugs
 */

#include <cb_pool.h>
#include <stdlib.h>
#include <tcb.h>

int cb_pool_init(cb_pool_t *tp) {
    if (tp == NULL) return -1;

    if (mutex_init(&(tp->m)) < 0 || ll_init(&(tp->pool)) < 0) return -1;
    return 0;
}

/** @brief */
int cb_pool_add_cb(cb_pool_t *tp, void *cb){
    if (tp == NULL || cb == NULL) return -1;
    mutex_lock(&(tp->m));
    int status = ll_add(&(tp->pool), (void *)(cb));
    mutex_unlock(&(tp->m));
    return status;
}

void *get_id_from_cb(void *cb){
    if (cb == NULL) return (void *)(-1);
    return (void *)(((tcb_t *)cb)->id);
}

/** @brief */
int cb_pool_get_cb(cb_pool_t *tp, int id, void **cb){
    if (tp == NULL || cb == NULL || id < 0) return -1;

    mutex_lock(&(tp->m));
    int status = ll_find(&(tp->pool), &get_id_from_cb, (void *)id, (void*)cb);
    mutex_unlock(&(tp->m));
    return status;
}

/** @brief */
int cb_pool_remove_cb(cb_pool_t *tp, int id){
    if (tp == NULL || id == -1) return -1;
    mutex_lock(&(tp->m));
    int status = ll_remove(&(tp->pool), &get_id_from_cb, (void *) id);
    mutex_unlock(&(tp->m));
    return status;
}


