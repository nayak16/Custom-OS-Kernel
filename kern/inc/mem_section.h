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

typedef struct mem_section_t {
    unsigned long v_addr_start;
    unsigned long len;
    void *src_data;
    uint32_t pde_f;
    uint32_t pte_f;

} mem_section_t;

int mem_section_init(mem_section_t *ms, uint32_t addr,
                     uint32_t len, void *src, uint32_t pde_f, uint32_t pte_f);

#endif /* _MEM_SECTION_H_ */
