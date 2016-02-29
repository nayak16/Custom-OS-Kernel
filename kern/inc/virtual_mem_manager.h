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

int create_mapping(uint32_t vpn, uint32_t ppn, uint32_t pte_flags,
                   uint32_t pde_flags, page_directory_t *pd){

#endif /* _VIRTUAL_MEM_MANAGER_H_ */


