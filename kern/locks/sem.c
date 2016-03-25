/** @file sem.c
 *  @brief This file defines the implementation to semaphores
 *
 *  Our implementation of semaphores uses a linked list queue to store threads
 *  that are currently waiting on the semaphore, a count to remember how many
 *  resources are avaliable (or in need) and a mutex to protect the internals.
 *  Semaphores use the same reject pointer to achieve "atomic unlock and
 *  deschedule" as mutexes
 *
 *  @author Christopher Wei (cjwei), Aatish Nayak (aatishn)
 *  @bug No known bugs
 */

#include <stdlib.h>
#include <sem.h>
#include <queue.h>
#include <mutex.h>

#include <simics.h>

#include <kern_internals.h>

typedef struct thr_metadata{
    int tid;
    int reject;
} thr_metadata_t;

/* semaphore functions */

/** @brief Initializes a semaphore
 *
 *  @param sem Pointer to the semaphore that is to be initialized
 *  @param count Number of resource locks semaphore is intialized with
 *  @return 0 on success, negative code on failure
 */
int sem_init( sem_t *sem, int count ) {
    if (sem == NULL || count <= 0) return -1;
    sem->count = count;
    if (queue_init(&(sem->q)) < 0) return -1;
    if (mutex_init(&(sem->m)) < 0) return -1;

    return 0;
}

/** @brief Attempts to take a resource lock from semaphore, sleeps on failure
 *
 *  @param sem Pointer to the semaphore
 *  @return Void
 */
void sem_wait( sem_t *sem ) {
    if (sem == NULL) return;

    mutex_lock(&(sem->m));

    /* declare interest in a resource from semaphore */
    sem->count--;
    if (sem->count < 0) {
        /* no resources avaliable, sleep until awoken from sem_signal */
        thr_metadata_t metadata;
        if (scheduler_get_current_tid(&sched, &(metadata.tid)) < 0) return;
        metadata.reject = 0;
        /* add self to waiting queue */
        queue_enq(&(sem->q), (void *)&metadata);
        mutex_unlock(&(sem->m));
        /* only deschedules if awoken by sem_signal */
        while(!(metadata.reject))
            lprintf("TODO: implement deschedule for semaphores");
            //syscall_deschedule_c_handler(&(metadata.reject));
    } else {
        /* resource avaliable! */
        mutex_unlock(&(sem->m));
    }
}

/** @brief Signals that a semaphore's resource is now avaliable and the first of
 *         any threads waiting on a resource should be awoken.
 *
 *  @param sem Pointer to the semaphore
 *  @return Void
 */
void sem_signal( sem_t *sem ) {
    if (sem == NULL) return;
    mutex_lock(&(sem->m));

    /* release a resource */
    sem->count++;

    /* if anything is waiting on a resource, wake it up */
    if (sem->count <= 0) {
        thr_metadata_t *temp;
        if(queue_deq(&(sem->q), (void **)(&temp)) < 0) {
            /* Empty queue, shouldn't happen unless semaphore was destroyed
             * in which case we will be lenient and simply unlock and return */
            mutex_unlock(&(sem->m));
            return;
        }
        // Wake up waiting thread if necessary
        temp->reject = 1;
        //TODO: make_runnable(temp->tid);
    }
    mutex_unlock(&(sem->m));
}

/** @brief Destroys a semaphore
 *
 *  @param sem Pointer to the semaphore
 *  @return Void
 */
void sem_destroy( sem_t *sem ) {
    if (sem == NULL) return;
    mutex_lock(&(sem->m));
    queue_destroy(&(sem->q));
    sem->count = -1;
    mutex_unlock(&(sem->m));
    mutex_destroy(&(sem->m));
}

