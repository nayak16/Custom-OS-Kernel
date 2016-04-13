/** @file virtual_mem_manager.h
 *  @brief Specifies interface for manipulating threads inside the
 *  kernel
 *
 *  @author Aatish Nayak (aatishn)
 *  @author Christopher Wei (cjwei)
 *  @bug No known bugs.
 */

#ifndef _THR_HELPERS_H_
#define _THR_HELPERS_H_

#include <scheduler.h>

int thr_deschedule(uint32_t old_esp, int *reject);
int thr_make_runnable(int tid);
int thr_yield(uint32_t old_esp, int tid);
int thr_gettid(void);
int thr_sleep(uint32_t old_esp, int ticks);
void thr_vanish(void);
void thr_set_status(int status);

/**
 * @brief Yields execution to the thread with the specified tcb
 *
 * Used while a thread is in the kernel and needs to yield to another
 * thread. Normally, when a thread calls an INT from user land to execute
 * the syscall yield, its context information is saved on its stack by the
 * syscall handler and the INT. When yielding from inside the kernel, we
 * simulate saving this context information on the stack with a series of
 * assembly instructions
 *
 * @return 0 on success, negative error code otherwise
 *
 *
 */
int thr_kern_yield(int tid);

/**
* @brief Deschedules the current thread, ending its execution until a
* make_runnable call tells it to wake up.
*
* Used while a thread is in the kernel and needs to deschedule itself.
* Normally, when a thread calls an INT from user land to execute
* the syscall deschedule, its context information is saved on its
* stack by the syscall handler and the INT. When descheduling from inside
* the kernel, we simulate saving this context information on the stack
* with a series of assembly instructions
*
* @return 0 on success, negative error code otherwise
*
*/
int thr_kern_deschedule(int *reject);

#endif /* _THR_HELPERS_H_ */


