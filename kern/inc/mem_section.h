/** @file mem_section.h
 *  @brief interface for a vm memory section
 *
 *  @author Aatish Nayak (aatishn)
 *  @author Christopher Wei (cjwei)
 *  @bug No known bugs.
 */

#ifndef _MEM_SECTION_H_
#define _MEM_SECTION_H_

#include <stdint.h>

/** @brief defines a memory section struct */
typedef struct mem_section_t {
    /** @brief the starting virtual address */
    unsigned long v_addr_start;
    /** @brief the length of the memory sectio n*/
    unsigned long len;
    /** @brief the page directory flags */
    uint32_t pde_f;
    /** @brief the page table flags */
    uint32_t pte_f;
} mem_section_t;

int ms_init(mem_section_t *ms, uint32_t addr,
            uint32_t len, uint32_t pde_f, uint32_t pte_f);
int ms_get_bounding_addr(mem_section_t *secs, uint32_t num_secs,
        uint32_t *addr_low, uint32_t *addr_high);
int ms_get_bounding_section(mem_section_t *secs, uint32_t num_secs,
        uint32_t addr_low, uint32_t addr_high, mem_section_t **result);
#endif /* _MEM_SECTION_H_ */
