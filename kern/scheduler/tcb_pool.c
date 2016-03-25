/** @file tcb_pool.h
 *  @brief Implementation for a thread control block pool
 *
 *  The threads hashtable holds linked list nodes that either belong
 *  in the runnable pool or waiting pool
 *
 *  Keys are tids (int) and values are tcb_t*
 *  @author Christopher Wei (cjwei)
 *  @author Aatish Nayak (aatishn)

 *  @bug No known bugs
 */



#include <ll.h>
#include <ht.h>
#include <tcb_pool.h>
#include <tcb.h>

int tid_hash(key_t tid) {

    return (int) tid;
}

int tcb_pool_init(tcb_pool_t *tp) {
    if (tp == NULL) return -1;

    if (ht_init(&(tp->threads), TABLE_SIZE, tid_hash) < 0) return -2;

    if (ll_init(&(tp->runnable_pool)) < 0
            || ll_init(&(tp->waiting_pool))) return -3;

    return 0;
}

/**
 * @brief Adds the specified tcb to the runnable pool for the first time
 *
 * @param tp Thread pool to add to
 * @param tcb Pointer to tcb to add
 *
 * @return 0 on success, negative error code otherwise
 *
 */
int tcb_pool_add_runnable_tcb(tcb_pool_t *tp, tcb_t *tcb) {
    if (tp == NULL || tcb == NULL) return -1;

    /* Create new node to insert into hashtable */
    ll_node_t *node = malloc(sizeof(ll_node_t));
    if (ll_node_init(node, (void *) tcb) < 0) return -2;

    /* Insert node into hashtable */
    if (ht_put(&(tp->threads), (key_t) tcb->tid, (void*) node) < 0) return -3;

    /* Put same node into runnable pool */
    if (ll_link_node_last(&(tp->runnable_pool), node) < 0) return -4;

    return 0;
}

int tcb_pool_get_next_tcb(tcb_pool_t *tp, tcb_t **next_tcb) {
    if (tp == NULL || next_tcb == NULL) return -1;

    /* Cycle runnable pool once */
    if (ll_cycle(&(tp->runnable_pool)) < 0) return -2;

    /* Head of runnable_pool should be next tcb */
    if (ll_peek(&(tp->runnable_pool), (void**) next_tcb) < 0) return -3;

    return 0;
}

int tcb_pool_find_tcb(tcb_pool_t *tp, int tid, tcb_t **tcbp) {
    if (tp == NULL || tcbp == NULL) return -1;
    if (ht_get(&(tp->threads), (key_t) tid, (void**) tcbp) < 0) {
        /* Not found */
        return -2;
    }
    return 0;
}

int tcb_pool_remove_tcb(tcb_pool_t *tp, int id);


