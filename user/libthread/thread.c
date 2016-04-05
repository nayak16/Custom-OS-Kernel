/**
 * @file thread.c
 *
 * @brief Implementation of thread library
 *
 * Implementation details explained in README.dox
 *
 * @author Christopher Wei (cjwei) and Aatish Nayak (aatishn)
 *
 */


/* C Standard Lib includes */
#include <stdlib.h>
#include <stdio.h>
#include <syscall.h>
#include <stdint.h>
#include <stdbool.h>

/* Debugging */
#include <simics.h>


/* Thread Lib specific includes */
#include <thr_internals.h>
#include <mutex.h>
#include <cond.h>
#include <thread.h>
#include <rwlock.h>


extern void *exception_stack;


/** @brief Code indicating a thread encountered an unexpected exception */
#define ERROR_CODE -1

/** @brief Error message to report when thread crashes unexpectedly */
#define ERROR_MSG "Error %d: Thread %d crashed unexpectedly!"


/*************************************************************
 *               Thread Library Helper Functions             *
 * ***********************************************************/

/**
 * @brief Allocates a len bytes on the stack for the current thread
 *
 * Only allocates len bytes if there is not enough stack space
 *
 * @param top base pointer to start allocating memory
 * @param len number of bytes to allocate
 *              (must be integral multiple of PAGE_SIZE)
 *
 * @return 0 on success, otherwise, negative integer error code
 *
 */
int new_thr_page(void* top, uint32_t len) {

    /* Only allocate if past bottom of stack */
    if ((uint32_t) top - len < (uint32_t) STACK_BOTTOM) {

        int ret = new_pages((void*)((uint32_t) STACK_BOTTOM - len), len);
        if (ret < 0) return ret;

        /* Update new stack bottom */
        STACK_BOTTOM = (void *)((uint32_t)STACK_BOTTOM - len);
    }
    return 0;

}

/**
 * @brief Safely (_s) adds a new thread to the global thread pool
 *
 * @param k_tid kernel assigned tid of thread to add
 * @param stack_top stack top of thread (esp/ebp)
 *
 * @return 0 on success, -1 on error
 */
int add_to_pool_s(int k_tid, void *stack_top) {

    thread_t *thr = malloc(sizeof(thread_t));
    if(thr == NULL) return -1;

    /* Initialize thread values */
    thr->k_tid = k_tid;
    thr->stack_top = stack_top;
    thr->status = THR_STATUS_ALIVE;
    cond_init(&(thr->join_cv));
    mutex_init(&(thr->m));

    /* Lock the thread pool as a writer */
    rwlock_lock(&thread_pool_lock, RWLOCK_WRITE);

    /* Add thread to pool */
    if(ll_add(&thread_pool, (void*) thr) < 0){
        rwlock_unlock(&thread_pool_lock);
        free(thr);
        return -1;
    }
    /* Release */
    rwlock_unlock(&thread_pool_lock);
    return 0;
}

/**
 * @brief Gets value of status field of a thread_t
 *
 * Used functionally to search the thread pool
 *
 * @param thr generic pointer that points to a thread_t
 *
 * @return Value of status field of a thread_t
 *
 */
void *get_struct_status(void* thr) {
    return (void*) ((thread_t *) thr)->status;
}

/**
 * @brief Gets value of k_tid field of a thread_t
 *
 * Used functionally to search the thread pool
 *
 * @param thr generic pointer that points to a thread_t
 *
 * @return Value of k_tid field of a thread_t
 *
 */
void *get_struct_k_tid(void *thr){
    return (void*) ((thread_t *) thr)->k_tid;
}

/**
 * @brief Searches thread pool for a thread with k_tid matching desired tid
 *
 * @param k_tid Desired tid to find
 * @param thread Pointer to found thread
 *
 * @return 0 on success, -1 if thread not found
 *
 */
int find_thread_by_k_tid(int k_tid, thread_t **thread){
    if (ll_find(&thread_pool, get_struct_k_tid,
                (void *)k_tid, (void **)thread) < 0)
        return -1;
    return 0;
}

/**
 * @brief Reap contents of a zombie thread
 *
 * @param zombie pointer to zombie thread
 *
 * @return 0 on success, -1 on error
 */
int reap_zombie(thread_t *zombie){
    if (zombie == NULL) return -1;

    zombie->status = THR_STATUS_DEAD;
    zombie->k_tid = -1;
    cond_destroy(&(zombie->join_cv));
    return 0;
}

/**
 * @brief Handler that handles unexpected thread crashes
 *
 * @param args Opaque arguments to the handler
 * @param ureg Register set and context information prior to the exception
 *
 * @return void
 *
 */
void thread_exception_handler(void *args, ureg_t *ureg) {

    /* Alert user */
    if (ureg != NULL)
        printf(ERROR_MSG, ureg->cause, gettid());

    /* Exit all threads with error */
    task_vanish(ERROR_CODE);
}

/**
 * @brief Installs thread exception handler
 *
 * @return void
 */
void install_exception_handler(void) {
    swexn(thr_exception_stack, thread_exception_handler, NULL, NULL);
}

/*************************************************************
 *               Thread Library Core Functions               *
 * ***********************************************************/

/**
 * @brief Initialize user thread library
 *
 * @param size stack space each thread should have allocated
 *
 */
int thr_init( unsigned int size ){
    if (size == 0) return -1;

    /* Page align stack size */
    thread_stack_size = (((size - 1) / PAGE_SIZE) + 1)* PAGE_SIZE;

    /* Init heap lock */
    mutex_init(&heap_mutex);

    /* Calculate top of parent stack and page align */
    void* esp_reg = get_esp();
    parent_stack_top =
        (void*)((uint32_t) esp_reg - ((uint32_t) esp_reg % PAGE_SIZE));

    /* Allocate stack space for parent */
    if(new_thr_page(parent_stack_top, thread_stack_size) < 0) return -1;

    /* Init global thread pool and associated rwlock */
    ll_init(&thread_pool);
    rwlock_init(&thread_pool_lock);

    /* Add parent to thread pool */
    if(add_to_pool_s(gettid(), parent_stack_top) < 0) return -1;

    /* Allocate space for thread crash exception stack */
    thr_exception_stack = malloc(sizeof(void*) * PAGE_SIZE);

    /* Register new exception handler */
    install_exception_handler();
    return 0;
}

/**
 * @brief Creates a new thread to run func(args)
 *
 * A new stack frame of a previously set size
 * All accesses to shared variables and data structures are mutually exclusive
 * In order to reuse stack space, the function reaps dead threads' stack space.
 * Adds newly created thread to global thread pool.
 *
 * @param func Pointer to function thread should run
 * @param args Arguement to func
 *
 * @return tid of created thread, negative error code otherwise
 *
 */
int thr_create( void *(*func)(void *), void *args ){
    void *new_esp;
    int found;
    thread_t *dead_thread;

    /* Before we search, lock thread pool as a reader */
    rwlock_lock(&thread_pool_lock, RWLOCK_READ);
    while (1){
        /* attempt to find a thread that is dead and read it into dead_thread */
        found = ll_find(&thread_pool, get_struct_status,
                            (void *)THR_STATUS_DEAD, (void**) &dead_thread);
        if (found < 0){
            /* if nothing found, calculate new_esp and break */
            new_esp = (void*) (parent_stack_top - thread_pool.size * \
                           thread_stack_size);
            break;
        }
        else{
            /* lock the thread struct, double check that it hasn't changed
             * status since we found it */
            mutex_lock(&(dead_thread->m));

            if (dead_thread->status == THR_STATUS_DEAD){
                /* success! update the dead_thread and break */
                new_esp = dead_thread->stack_top;
                cond_init(&(dead_thread->join_cv));

                /* Resurrect thread */
                dead_thread->status = THR_STATUS_ALIVE;
                mutex_unlock(&(dead_thread->m));
                break;
            }
            mutex_unlock(&(dead_thread)->m);
            /* only gets here if the status has changed since finding it
             * (by another thread), so try again. Loop ensures race conditions
             * are handled properly */
        }

    }
    rwlock_unlock(&thread_pool_lock);

    /* Allocate a new thread page only if needed */
    if(found < 0 && new_thr_page(new_esp, thread_stack_size) < 0){
        return -1;
    }

    /* Fork a new child thread */
    int child_tid = thread_fork(new_esp, func, args);

    if (child_tid < 0){
        /* Revert changes if error */
        if (found >= 0){
            mutex_lock(&(dead_thread->m));
            dead_thread->status = THR_STATUS_DEAD;
            mutex_lock(&(dead_thread->m));
        }
        return -1;
    }

    /* Add child to thread_pool if necessary */
    if (found < 0){
        if (add_to_pool_s(child_tid, new_esp) < 0){
            panic("Can't add child to thread pool!");
        }
    }
    /* Otherwise set new tid of resurrected thread */
    else {
        mutex_lock(&(dead_thread->m));
        dead_thread->k_tid = child_tid;
        mutex_unlock(&(dead_thread->m));
    }

    return child_tid;
}

/**
 * @brief Exits the calling thread with exit status
 *
 * Sets self status to ZOMBIE to allow joining threads to reap self
 *
 * @param status status to exit current thread with
 *
 * @return void (should never return)
 */
void thr_exit( void *status ) {

    /* Lock thread pool as reader while searching */
    rwlock_lock(&thread_pool_lock, RWLOCK_READ);

    thread_t *target_thr;
    /* Loop until parent adds you to the thread pool */
    while (find_thread_by_k_tid(gettid(), &target_thr) < 0){
        /* Allow parent to add self to pool */
        rwlock_unlock(&thread_pool_lock);
        yield(-1);
        rwlock_lock(&thread_pool_lock, RWLOCK_READ);
    }

    /* Ensure mutual exclusion on zombie thread */
    mutex_lock(&(target_thr->m));
    target_thr->status = THR_STATUS_ZOMBIE;
    target_thr->exit_status = status;

    /* Signal to joining threads */
    cond_signal(&(target_thr->join_cv));

    /* Release lock on zombie thread */
    mutex_unlock(&(target_thr->m));

    /* Release lock on thread pool */
    rwlock_unlock(&thread_pool_lock);

    set_status((int) status);
    vanish();
}

/**
 * @brief Allows calling thread to block until thread with k_tid exits,
 * while optionally returning the thread's exit status in statusp
 *
 * @param k_tid Thread to join on
 * @param statusp Address to store thread's exit status
 *
 * @return 0 on success. negative error code otherwise
 */
int thr_join( int k_tid, void **statusp ) {

    /* Lock thread pool as reader while searching */
    rwlock_lock(&thread_pool_lock, RWLOCK_READ);

    thread_t *target_thr;
    if (find_thread_by_k_tid(k_tid, &target_thr) < 0){
        rwlock_unlock(&thread_pool_lock);
        return -1;
    }
    /* Release lock */
    rwlock_unlock(&thread_pool_lock);

    /* This gap in the mutual exclusion ensures there is no order
     * in multiple threads attempting to join a single thread. Essentially,
     * calling thr_join before another thread does not guarantee joining
     * before that thread. It depends on which thread the scheduler "chooses"
     * to acquire the following thread lock "first". This should not be the thread
     * library's responsibility.                                             */

    mutex_lock(&(target_thr->m));

    if (target_thr->status == THR_STATUS_DEAD){
        /* The thread k_tid has already been joined on */
        mutex_unlock(&(target_thr->m));
        return -1;
    }

    /* Wait on target_thr to become a zombie */
    while(target_thr->status == THR_STATUS_ALIVE){

        /* Check if another thread is already in the process of joining */
        if (target_thr->pending_join){
            mutex_unlock(&(target_thr->m));
            return -1;
        } else {
            target_thr->pending_join = true;
            /* Block until thread becomes a ZOMBIE */
            cond_wait(&(target_thr->join_cv), &(target_thr->m));
        }
    }
    /* save status if needed */
    if (statusp != NULL) *statusp = target_thr->exit_status;

    /* Indicate there is no pending join */
    target_thr->pending_join = false;

    /* Reap the zombie thread */
    reap_zombie(target_thr);

    mutex_unlock(&(target_thr->m));
    return 0;
}

/**
 * @brief Obtains the tid of the currently running thread
 *
 * The thread library's notion of an id matches the kernel's notion
 * of a user space thread id
 *
 * @return tid of the currently running thread
 *
 */
int thr_getid( void ) {
    return gettid();
}

/**
 * @brief Defers execution of the calling thread to thread with tid
 *        if tid = -1, defer to a thread specified by the scheduler
 *
 * @param tid thread to yield to
 *
 * @return 0 on success, negative error code otherwise
 *
 */
int thr_yield( int tid ) {

    return yield(tid);
}
