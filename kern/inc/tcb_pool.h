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
#include <circ_buffer.h>

#define TABLE_SIZE 64
/**
 * @brief Struct representing a thread pool
 */
typedef struct tcb_pool {
    /** @brief hash table for thread */
    ht_t threads;
    /** @brief hash table for processes */
    ht_t processes;

    /** @brief linked list of runnable threads */
    ll_t runnable_pool;
    /** @brief linked list of waiting threads */
    ll_t waiting_pool;
    /** @brief linke dlist of sleeping threads */
    ll_t sleeping_pool;
    /** @brief linked list of zombie threads */
    ll_t zombie_pool;

    /** @brief semaphore for signaling reaper thread */
    sem_t zombies_sem;

    } tcb_pool_t;

int tcb_pool_init(tcb_pool_t *tp);
int tcb_pool_add_runnable_tcb_safe(tcb_pool_t *tp, tcb_t *tcb);
int tcb_pool_add_pcb_safe(tcb_pool_t *tp, pcb_t *pcb);
int tcb_pool_remove_pcb(tcb_pool_t *tp, int pid, circ_buf_t *addr_to_free);

int tcb_pool_make_runnable(tcb_pool_t *tp, int tid);
int tcb_pool_make_waiting(tcb_pool_t *tp, int tid);
int tcb_pool_make_sleeping(tcb_pool_t *tp, int tid);
int tcb_pool_make_zombie(tcb_pool_t *tp, int tid);
int tcb_pool_wakeup(tcb_pool_t *tp, uint32_t curr_time);
int tcb_pool_reap(tcb_pool_t *tp);

int tcb_pool_get_next_tcb(tcb_pool_t *tp, tcb_t **next_tcbp);
int tcb_pool_remove_tcb(tcb_pool_t *tp, int tid, circ_buf_t *addr_to_free);
int tcb_pool_find_tcb(tcb_pool_t *tp, int tid, tcb_t **tcbp);
int tcb_pool_find_pcb(tcb_pool_t *tp, int pid, pcb_t **pcbp);


#endif /* _TCB_POOL_H_ */
