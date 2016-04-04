/** @file thr_internals.h
 *  @brief Defines internal variables and assembly functions
 *
 *  @author Christopher Wei (cjwei), Aatish Nayak (aatishn)
 *  @bug No known bugs
 */

#ifndef THR_INTERNALS_H
#define THR_INTERNALS_H

/* Standard C library includes */
#include <stdbool.h>

/* Thread Lib specific includes */
#include <mutex_type.h>
#include <cond_type.h>
#include <rwlock_type.h>
#include <ll.h>

/** @brief Denotes a thread is dead and can be recycled */
#define THR_STATUS_DEAD 1
/** @brief Denotes a thread has exited but has not been joined on */
#define THR_STATUS_ZOMBIE 2
/** @brief Denotes a thread is currently running or sleeping */
#define THR_STATUS_ALIVE 0


/** @brief Used to store metadata about a thread */
typedef struct {
	/**
     * @brief Whether or not a join has already occured on a thread
     *
     * Used to ensure multiple threads can't join on the same thread
     * at the same time
     * */
    bool pending_join;

    /**
     * @brief The kernel thread id
     *
     * The thread library's way of identifying each thread.
     * This is the same as the kernel defined id returned by gettid()
     * */
    int k_tid;

    /**
     * @brief The reject pointer used by semaphores or condition variables
     *
     * This allows atomicity for mutex unlock and deschedule in condition
     * variables and semaphores.
     *
     * */
    int reject;

    /**
     * @brief The thread's current status (running, zombie, dead)
     *
     * */
    int status;

    /**
     * @brief The exit status of a thread
     *
     * Used in thr_join to report exit status back to calling thread
     * */
    void *exit_status;

    /**
     * @brief The condition variable used to signal joining threads
     * about the current thread's status
     * */
    cond_t join_cv;

    /**
     * @brief Mutex for protecting struct accesses
     * */
    mutex_t m;

    /**
     * @brief The top address of a thread's stack
     *
     * If a thread becomes DEAD, subsequent threads can claim this
     * stack space.
     * */
    void *stack_top;

} thread_t;

/*************************************************************
 *              Thread Library Global Variables              *
 * ***********************************************************/

/** @brief Amount of stack space given to each thread */
unsigned int thread_stack_size;

/** @brief The page aligned stack top of the parent thread after converting to
 *         multithreaded mode */
void* parent_stack_top;

/** @brief Reader-Writer lock that protects the thread_pool linked list */
rwlock_t thread_pool_lock;

/** @brief List of all threads that have been allocated space in the stack */
ll_t thread_pool;

/** @brief Pointer to top of allocated user stack space */
void *STACK_TOP;

/** @brief Pointer to bottom of allocated user stack space */
void *STACK_BOTTOM;

/** @brief mutex used to protect the heap from multiple coinciding writes */
mutex_t heap_mutex;

/** @brief Stack used in handling thread exception/crashes  */
void *thr_exception_stack;

/**
 * Docs in thread.c
 */
void install_exception_handler(void);



/*************************************************************
 *                Assembly Functions Headers                 *
 * ***********************************************************/

/** @brief Atomically set *lock = val and return previous value
 *         of lock
 *
 *  @param lock Lock to set
 *  @param val Value to set lock to
 *
 *  @return Old value of lock
 *
 */
int xchng(int* lock, int val);

/** @brief calls system call thread_fork
 *
 *  @param new_esp The new thread's stack pointer that is to be set
 *  @param func The function that is to be called in the new thread
 *  @param args The arguments provided to func
 *
 *  @return thread ID of newly forked thread
 */
int thread_fork(void *new_esp, void *(*func)(void*), void *args);

/** @brief Gets current thread's stack pointer
 *
 *  @return current esp register
 */
void* get_esp(void);

#endif /* THR_INTERNALS_H */
