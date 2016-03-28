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
int thr_kern_deschedule(int *reject);
int thr_make_runnable(int tid);
int thr_yield(uint32_t old_esp, int tid);
int thr_gettid(void);
int thr_sleep(uint32_t old_esp, int ticks);


int thr_kern_deschedule(int *reject);

#endif /* _THR_HELPERS_H_ */


