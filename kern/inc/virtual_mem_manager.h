/** @file virtual_mem_manager.h
 *  @brief Specifies interface for virtual memory manipulation
 *
 *  @author Aatish Nayak (aatishn)
 *  @author Christopher Wei (cjwei)
 *  @bug No known bugs.
 */

#ifndef _VIRTUAL_MEM_MANAGER_H_
#define _VIRTUAL_MEM_MANAGER_H_

#include <stdint.h>

#include <page_directory.h>
#include <frame_manager.h>
#include <mem_section.h>

#define NEW_FLAGS(p,rw,md,glb) ((p << PRESENT_FLAG_BIT) | (rw << RW_FLAG_BIT)\
    | (md << MODE_FLAG_BIT) | (glb << GLOBAL_FLAG_BIT))

#define NUM_ENTRIES (PAGE_SIZE/sizeof(uint32_t))

int vmm_create_mapping(uint32_t vpn, uint32_t ppn, uint32_t pte_flags,
                   uint32_t pde_flags, page_directory_t *pd);

int vmm_user_mem_alloc(page_directory_t *pd, frame_manager_t *fm,
                     mem_section_t *secs, uint32_t num_secs);

#endif /* _VIRTUAL_MEM_MANAGER_H_ */


