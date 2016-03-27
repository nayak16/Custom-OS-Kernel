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

int thr_deschedule(int *reject);
int thr_make_runnable(int tid);
int thr_yield(int tid);
int thr_gettid(void);
int thr_sleep(int ticks);

#endif /* _THR_HELPERS_H_ */


