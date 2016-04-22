/** @file kern_internals.h
 *  @brief Specifies the global variables shared across the kernel
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
#include <sched_mutex.h>
#include <keyboard.h>

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
 * @brief Global keyboard buffer
 */
extern keyboard_t keyboard;
/**
 * @brief atomically exchanges val into lock and returns previous value of lock
 */
int xchng(int* lock, int val);

/**
 * @brief lock to protect heap (used in malloc, free, etc)
 *
 */
extern mutex_t heap_lock;

/**
 * @brief Lock to protect scheduler data structures
 *
 */
extern sched_mutex_t sched_lock;

/**
 * @brief Init value of APIC timer (configured using legacy PIT timer)
 *
 */
extern uint32_t apic_calib_init_val;

#endif /* _KERN_INTERNALS_H_ */


