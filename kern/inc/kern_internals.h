/** @file virtual_mem_manager.h
 *  @brief Specifies interface for virtual memory manipulation
 *
 *  @author Aatish Nayak (aatishn)
 *  @author Christopher Wei (cjwei)
 *  @bug No known bugs.
 */

#ifndef _KERN_INTERNALS_H_
#define _KERN_INTERNALS_H_

#include <scheduler.h>
#include <frame_manager.h>
#include <mutex.h>

/**
 * @brief Extern of physical memory manager for the kernel
 */
extern frame_manager_t fm;

/**
 * @brief Extern of global scheduler that manages all kernel PCBs and TCBs
 */
extern scheduler_t sched;

/**
 * @brief Global mutex for the console
 */
extern mutex_t console_lock;

/**
 * @brief Global mutex for the scheduler
 */
extern mutex_t scheduler_lock;

/**
 * @brief atomically exchanges val into lock and returns previous value of lock
 */
int xchng(int* lock, int val);

/**
 * @brief lock to protect heap (used in malloc, free, etc)
 *
 */
extern mutex_t heap_lock;

#endif /* _KERN_INTERNALS_H_ */


