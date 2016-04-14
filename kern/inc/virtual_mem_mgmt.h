/** @file virtual_mem_mgmt.h
 *  @brief Defines the interface for a virtual memory manager
 *
 *  @author Christopher Wei (cjwei)
 *  @author Aatish Nayak (aatishn)
 */


#ifndef _VIRTUAL_MEM_MGMT_H_
#define _VIRTUAL_MEM_MGMT_H_

#include <page_directory.h>
#include <mem_section.h>

int vmm_map_sections(page_directory_t *pd, mem_section_t *secs, int num_secs);
int vmm_deep_copy(page_directory_t *pd_dest);
int vmm_new_user_page(page_directory_t *pd, uint32_t base, uint32_t num_pages);
int vmm_remove_user_page(page_directory_t *pd, uint32_t base);
int vmm_clear_user_space(page_directory_t *pd);

#endif /* _VIRTUAL_MEM_MGMT_H_ */
