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

/**
 * @brief Extern of physical memory manager for the kernel
 */
extern frame_manager_t fm;

/**
 * @brief Extern of global scheduler that manages all kernel PCBs and TCBs
 */
extern scheduler_t sched;




#endif /* _KERN_INTERNALS_H_ */


