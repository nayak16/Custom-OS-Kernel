/** @file tcb_pool.h
 *  @brief Implementation for a thread control block pool
 *
 *  The threads hashtable holds linked list nodes that either belong
 *  in the runnable pool, waiting pool, sleeping, or zombie pool.
 *  Keys are tids (int) and values are tcb_t*
 *  See README for more info
 *
 *  There is also a seperate hash table that holds pcbs
 *
 *  @author Christopher Wei (cjwei)
 *  @author Aatish Nayak (aatishn)

 *  @bug No known bugs
 */


/* P3 specific includes */
#include <x86/asm.h>
#include <ll.h>
#include <ht.h>
#include <tcb_pool.h>
#include <tcb.h>
#include <kern_internals.h>


/**
 * @brief Approximate number of addresses tcb_pool data structures have
 * allocated per tcb. Obtained by looking through tcb_pool code and
 * seeing max number of frees required to remove a tcb and pcb from the
 * thr_pool data structures
 *
 * Used to determine size of circ_buf to store allocated addresses
 * in tcb_pool_reap
 */
#define NUM_ADDRS 32

/**
 * @brief Hashing function for tids used in the threads hashtable
 *
 * @param tid tid to hash
 *
 * @return hash of the tid
 *
 */
int tid_hash(key_t tid) {
    return (int) tid;
}

/**
 * @brief Hashing function for pids used in the processes hashtable
 *
 * @param pid pid to hash
 *
 * @return hash of the pid
 *
 */
int pid_hash(key_t pid) {
    return (int) pid;
}

/**
 * @brief Initializes a thread pool
 *
 * @param tp thread pool to initialize
 *
 * @return 0 on success, negative error code othwerise
 *
 */
int tcb_pool_init(tcb_pool_t *tp) {
    if (tp == NULL) return -1;

    /* Initialize the threads and processes hash tables */
    if (ht_init(&(tp->threads), TABLE_SIZE, tid_hash) < 0
        || ht_init(&(tp->processes), TABLE_SIZE, pid_hash) < 0) return -2;

    /* Initialize the runnable, waiting, sleeping, and zombie pools */
    if (ll_init(&(tp->runnable_pool)) < 0
        || ll_init(&(tp->waiting_pool)) < 0
        || ll_init(&(tp->sleeping_pool)) < 0
        || ll_init(&(tp->zombie_pool))< 0) return -3;

    /* Initialize the zombie semaphore */
    if (sem_init(&(tp->zombies_sem), 0) < 0) return -4;

    return 0;
}


/**
 * @brief Adds the specified tcb to the runnable pool for the first time.
 * Utilizes the scheduler lock to ensure no other thread is touching the
 * data structures while they are being modified. To ensure, that the
 * scheduler isn't locked during malloc, since it is a blackbox and may
 * take a long time, all mallocing is done before hand.
 *
 * @param tp Thread pool to add to
 * @param tcb Pointer to tcb to add
 *
 * @return 0 on success, negative error code otherwise
 *
 */
int tcb_pool_add_runnable_tcb_safe(tcb_pool_t *tp, tcb_t *tcb) {
    if (tp == NULL || tcb == NULL) return -1;

    /* Malloc all necessary structures before locking the scheduler */

    /* Create new node to insert into hashtable */
    ll_node_t *node = malloc(sizeof(ll_node_t));
    if (ll_node_init(node, (void *) tcb) < 0) return -2;

    /* Create new hash table entry for new tcb node */
    ht_entry_t *new_e = malloc(sizeof(ht_entry_t));
    if (new_e == NULL) return -3;
    new_e->val = (void*)node;
    new_e->key = (key_t) tcb->tid;

    /* Create node to insert entry into hashtable bucket */
    ll_node_t *entry_node = malloc(sizeof(ll_node_t));
    if (ll_node_init(entry_node, (void*) new_e) < 0) return -4;

    /* Lock the scheduler while inserting */
    sched_mutex_lock(&sched_lock);

    /* Insert entry and node into hashtable */
    if (ht_put_entry(&(tp->threads), new_e, entry_node) < 0) return -3;

    /* Put same node into runnable pool */
    if (ll_link_node_last(&(tp->runnable_pool), node) < 0) return -4;

    /* Unlock the scheduler and proceed */
    sched_mutex_unlock(&sched_lock);

    return 0;
}

/**
 * @brief Adds the specified pcb to the processes hash table.
 * Utilizes the scheduler lock to ensure no other thread is touching the
 * data structures while they are being modified. To ensure, that the
 * scheduler isn't locked during malloc, since it is a blackbox and may
 * take a long time, all mallocing is done before hand.
 *
 * @param tp Thread pool to add to
 * @param tcb Pointer to tcb to add
 *
 * @return 0 on success, negative error code otherwise
 *
 */
int tcb_pool_add_pcb_safe(tcb_pool_t *tp, pcb_t *pcb) {
    if (tp == NULL || pcb == NULL) return -1;

    /* Malloc all necessary structures before locking the scheduler */

    /* Create new node to insert into hashtable */
    ll_node_t *node = malloc(sizeof(ll_node_t));
    if (ll_node_init(node, (void *) pcb) < 0) return -2;

    /* Create new hash table entry for new pcb node */
    ht_entry_t *new_e = malloc(sizeof(ht_entry_t));
    if (new_e == NULL) return -3;
    new_e->val = (void*)node;
    new_e->key = (key_t) pcb->pid;

    /* Create node to insert entry into hashtable bucket */
    ll_node_t *entry_node = malloc(sizeof(ll_node_t));
    if (ll_node_init(entry_node, (void*) new_e) < 0) return -4;

    /* Lock the scheduler while inserting */
    sched_mutex_lock(&sched_lock);

    /* Insert entry and node into hashtable safely */
    if (ht_put_entry(&(tp->processes), new_e, entry_node) < 0) return -3;

    /* Unlock the scheduler and proceed */
    sched_mutex_unlock(&sched_lock);

    return 0;
}


/**
 * @brief Removes the pcb with the specified pid from the processes
 * hash table. Has an optional circ_buf_t parameter that saves all addresses
 * need to be freed, and does not free them.
 *
 * @param tp thr_pool to access
 * @param pid pid of pcb to remove
 * @param addrs_to_free optional parameter that saves all addresses that need
 * to be freed. NULL if addresses should be freed right away
 *
 * @return 0 on success, negative error code otherwise
 *
 *
 */
int tcb_pool_remove_pcb(tcb_pool_t *tp, int pid, circ_buf_t *addrs_to_free) {
    if (tp == NULL) return -1;

    ll_node_t *node;
    /* Get specified node and pcb from hash table */
    if (ht_remove(&(tp->processes), (key_t) pid, (void**) &node, addrs_to_free) < 0) {
        /* Not found */
        return -2;
    }

    /* Check if node addr needs to be saved */
    if (addrs_to_free != NULL) {
        if (circ_buf_write(addrs_to_free, (void*) node) < 0) {
            free(node);
        }
    } /* Otherwise free right now */
    else {
        free(node);
    }
    return 0;
}

/**
 * @brief Get the next tcb in the runnable pool. First, rotate
 * the runnable pool once and peek at the head.
 *
 * @param tp tcb pool to get next tcb from
 * @param next_tcb address to put the pointer to the next tcb
 *
 * @return 0 on success, negative error code otherwise
 *
 */
int tcb_pool_get_next_tcb(tcb_pool_t *tp, tcb_t **next_tcb) {
    if (tp == NULL || next_tcb == NULL) return -1;

    int ret;
    /* Rotate runnable pool once */
    if ((ret = ll_rotate(&(tp->runnable_pool))) == -2) {
        /* Runnable pool is empty */
        return -2;
    } else if (ret < 0) {
        /* Some other error */
        return -3;
    }

    /* Head of runnable_pool should be next tcb */
    if (ll_peek(&(tp->runnable_pool), (void**) next_tcb) < 0) return -4;

    return 0;
}

/**
 * @brief Finds the tcb with the specified tid in the tcb pool
 *
 * @param tp thr_pool to search
 * @param tid tid of tcb to find
 * @param tcbp address to put the pointer to the found tcb
 *
 * @return 0 on success, negative error code otherwise
 *
 */
int tcb_pool_find_tcb(tcb_pool_t *tp, int tid, tcb_t **tcbp) {
    if (tp == NULL || tcbp == NULL) return -1;
    ll_node_t *node;
    if (ht_get(&(tp->threads), (key_t) tid, (void**) &node) < 0) {
        /* Not found */
        return -2;
    }
    if (ll_node_get_data(node, (void**)tcbp) < 0) {
        return -3;
    }
    return 0;
}

/**
 * @brief Finds the pcb with the specified pid in the pcb pool
 *
 * @param tp thr_pool to search
 * @param pid pid of pcb to find
 * @param pcbp address to put the pointer to the found pcb
 *
 * @return 0 on success, negative error code otherwise
 *
 */
int tcb_pool_find_pcb(tcb_pool_t *tp, int pid, pcb_t **pcbp) {
    if (tp == NULL || pcbp == NULL) return -1;
    ll_node_t *node;
    if (ht_get(&(tp->processes), (key_t) pid, (void**) &node) < 0) {
        /* Not found */
        return -2;
    }
    if (ll_node_get_data(node, (void**)pcbp) < 0) {
        return -3;
    }
    return 0;
}

/**
 * @brief Moves the node holding the tcb with the specified tid from the
 * runnable pool to the waiting pool. Returns an error if tcb was already
 * in the runnable pool
 *
 * @param tp tcb pool to manipulate
 * @param tid tid of tcb to make runnable
 *
 * @return 0 on success, negative error code otherwise
 */
int tcb_pool_make_sleeping(tcb_pool_t *tp, int tid) {
    if (tp == NULL) return -1;

    ll_node_t *node;

    /* Get specified node and tcb from hash table */
    if (ht_get(&(tp->threads), (key_t) tid, (void**) &node) < 0) {
        /* Not found */
        return -2;
    }
    tcb_t *tcb;
    if (ll_node_get_data(node, (void**)&tcb) < 0) {
        return -3;
    }

    /* Remove from runnable pool */
    if (ll_unlink_node(&(tp->runnable_pool), node) < 0) return -5;

    /* Add to sleeping pool */
    if (ll_link_node_sorted(&(tp->sleeping_pool), node, &tcb_t_wakeup_cmp) < 0)
        return -6;

    return 0;

}

/**
 * @brief Loops through the sleeping pool and wakes up a thread if
 * the its wakeup time is equal to the current time.
 *
 * @param tp thr_pool to check
 * @param curr_time current scheduler tick count
 *
 * @return 0 on success, negative error code otherwise
 *
 */
int tcb_pool_wakeup(tcb_pool_t *tp, uint32_t curr_time){
    if (tp == NULL) return -1;
    tcb_t *tcb;
    while (ll_size(&(tp->sleeping_pool)) > 0){
        if (ll_peek(&(tp->sleeping_pool), (void **)&tcb) < 0)
            return -1;
        if (tcb->t_wakeup == curr_time){
            /* wakey wakey shrek */
            tcb_pool_make_runnable(tp, tcb->tid);
        } else {
            break;
        }
    }
    return 0;
}

/**
 * @brief Reaps zombies in the zombie pool everytime the zombies_sem is
 * signaled. Frees all of a zombies resources and returns it back to the kernel.
 *
 * The scheduler reaper thread loops infinitely in this function and reaps any
 * zombie threads. While modifying removing a tcb/pcb from their respective
 * pools, the scheduler is locked so nothing can modify the data structures.
 * Additionally, we want to not lock the scheduler while freeing, since
 * it may take a long time or not even have the heap lock. Because of this
 * we save all the addresses in a seperate buffer, and free them after
 * unlocking the scheduler lock. The pcb of a tcb is also removed/destroyed if
 * it contains no more threads. This function should never return.
 *
 * @return should never return
 *
 *
 */
int tcb_pool_reap(tcb_pool_t *tp){
    if (tp == NULL) return -1;

    circ_buf_t addrs_to_free;

    if (circ_buf_init(&addrs_to_free, NUM_ADDRS) < 0) {
        panic("Cannot allocate space to hold to-be-freed addresses. \
                Error in ShrekOS.");
    }

    tcb_t *tcb;
    void *addr;
    while(1) {
        /* Wait on zombies to be available */
        sem_wait(&(tp->zombies_sem));

        /* Get first zombie */
        if (ll_peek(&(tp->zombie_pool), (void **)&tcb) < 0) {
            /* No zombies to reap, wait again */
            continue;
        }

        /* Disable Interrupts while modifying tcb pool */
        sched_mutex_lock(&sched_lock);

        /* Remove from tcb hash table and zombie pool */
        if (tcb_pool_remove_tcb(tp, tcb->tid, &addrs_to_free) < 0) {
            continue;
        }

        /* Remove if last thread in pcb */
        if (tcb->pcb->num_threads == 0) {
            tcb_pool_remove_pcb(tp, tcb->pcb->pid, &addrs_to_free);
        }

        /* Enable interrupts before freeing and attempting to acquire heap lock */
        sched_mutex_unlock(&sched_lock);

        /* Check if pcb has no more threads running */
        if (tcb->pcb->num_threads == 0) {
            pcb_destroy_s(tcb->pcb);
            free(tcb->pcb);
        }
        /* Destroy tcb itself */
        tcb_destroy(tcb);
        free(tcb);

        /* Free rest of the addresses */
        while(circ_buf_read(&addrs_to_free, (void**) &addr) >= 0) {
            free(addr);
        }

    }

    /* For completeness */
    /* Destroy circ buf containing addresses to free */
    circ_buf_destroy(&addrs_to_free);

    /* To placate the compiler */
    return 0;
}
/**
 * @brief Moves the node holding the tcb with the specified tid from the
 * runnable pool to the waiting pool. Returns an error if tcb was already
 * in the runnable pool
 *
 * @param tp tcb pool to manipulate
 * @param tid tid of tcb to make runnable
 *
 * @return 0 on success, negative error code otherwise
 */
int tcb_pool_make_waiting(tcb_pool_t *tp, int tid) {
    if (tp == NULL) return -1;

    ll_node_t *node;

    /* Get specified node and tcb from hash table */
    if (ht_get(&(tp->threads), (key_t) tid, (void**) &node) < 0) {
        /* Not found */
        return -2;
    }
    tcb_t *tcb;
    if (ll_node_get_data(node, (void**)&tcb) < 0) {
        return -3;
    }

    /* Check if tcb is not already WAITING */
    if (tcb->status == WAITING) return -4;

    /* Remove from runnable pool */
    if (ll_unlink_node(&(tp->runnable_pool), node) < 0) return -5;

    /* Add to waiting pool */
    if (ll_link_node_last(&(tp->waiting_pool), node) < 0) return -6;

    return 0;

}

/**
 * @brief Moves the node holding the tcb with the specified tid from the
 * waiting pool or the sleeping pool to the runnable pool.
 * Returns an error if tcb was already in the runnable pool
 *
 * @param tp tcb pool to manipulate
 * @param tid tid of tcb to make runnable
 *
 * @return 0 on success, negative error code otherwise
 */
int tcb_pool_make_runnable(tcb_pool_t *tp, int tid) {
    if (tp == NULL) return -1;

    ll_node_t *node;

    /* Get specified node and tcb from hash table */
    if (ht_get(&(tp->threads), (key_t) tid, (void**) &node) < 0) {
        /* Not found */
        return -2;
    }
    tcb_t *tcb;
    if (ll_node_get_data(node, (void**)&tcb) < 0) {
        return -3;
    }

    /* Check if tcb is not already RUNNABLE */
    if (tcb->status == RUNNABLE) return -4;

    /* Remove from waiting pool */
    if (tcb->status == WAITING){
        if (ll_unlink_node(&(tp->waiting_pool), node) < 0) return -5;
    } else {
        if (ll_unlink_node(&(tp->sleeping_pool), node) < 0) return -5;
    }

    /* Add to runnable pool */
    if (ll_link_node_last(&(tp->runnable_pool), node) < 0) return -6;

    return 0;

}

/**
 * @brief Moves the node holding the tcb with the specified tid from the
 * runnable pool to the zombie pool. Signals the zombie semaphore so the
 * reaper thread can reap its resources.
 * Returns an error if tcb was already a zombie
 *
 * @param tp tcb pool to manipulate
 * @param tid tid of tcb to make runnable
 *
 * @return 0 on success, negative error code otherwise
 */
int tcb_pool_make_zombie(tcb_pool_t *tp, int tid){
    if (tp == NULL) return -1;

    ll_node_t *node;

    /* Get specified node and tcb from hash table */
    if (ht_get(&(tp->threads), (key_t) tid, (void**) &node) < 0) {
        /* Not found */
        return -2;
    }
    tcb_t *tcb;
    if (ll_node_get_data(node, (void**)&tcb) < 0) {
        return -3;
    }

    /* Signal to reaper to reap zombie */
    sem_signal(&(tp->zombies_sem));

    /* Remove from whatever pool it was in */
    switch(tcb->status){
        case RUNNABLE:
        case RUNNING:
            if (ll_unlink_node(&(tp->runnable_pool), node) < 0) return -5;
            break;
        case WAITING:
            /* hard to concieve a way for this to happen */
            if (ll_unlink_node(&(tp->waiting_pool), node) < 0) return -5;
            break;
        default:
            return -6;
    }
    tcb->status = ZOMBIE;
    /* Put into zombie pool */
    if (ll_link_node_last(&(tp->zombie_pool), node) < 0) return -7;

    return 0;
}

/**
 * @brief Removes the tcb with the specified tid from the threads
 * hash table. Has an optional circ_buf_t parameter that saves all addresses
 * need to be freed, and does not free them.
 *
 * @param tp thr_pool to access
 * @param pid pid of pcb to remove
 * @param addrs_to_free optional parameter that saves all addresses that need
 * to be freed. NULL if addresses should be freed right away
 *
 * @return 0 on success, negative error code otherwise
 *
 *
 */
int tcb_pool_remove_tcb(tcb_pool_t *tp, int tid, circ_buf_t *addrs_to_free) {
    if (tp == NULL) return -1;

    ll_node_t *node;
    /* Get specified node and tcb from hash table */
    if (ht_remove(&(tp->threads), (key_t) tid,
                    (void**) &node, addrs_to_free) < 0) {
        /* Not found */
        return -2;
    }

    tcb_t *tcb;
    if (ll_node_get_data(node, (void**)&tcb) < 0) {
        return -3;
    }

    /* Remove from appropriate pool */
    switch(tcb->status) {
        case RUNNABLE:
        case RUNNING:
            if (ll_unlink_node(&(tp->runnable_pool), node) < 0) return -5;
            break;
        case WAITING:
            if (ll_unlink_node(&(tp->waiting_pool), node) < 0) return -5;
            break;
        case ZOMBIE:
            if (ll_unlink_node(&(tp->zombie_pool), node) < 0) return -5;
            break;
        default:
            return -4;
    }
    /* Check if node addr needs to be saved */
    if (addrs_to_free != NULL) {
        circ_buf_write(addrs_to_free, (void*) node);
    } /* Otherwise free right now */
    else {
        free(node);
    }
    return 0;
}


