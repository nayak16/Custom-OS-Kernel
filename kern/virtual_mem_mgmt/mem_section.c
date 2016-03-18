/** @file mem_section.c
 *  @brief Implementation of memory section functions
 *
 *  @author Aatish Nayak (aatishn)
 *  @author Christopher Wei (cjwei)
 *  @bug No known bugs.
 */
#include <stdlib.h>

#include <mem_section.h>
/*
typedef struct mem_section_t {
    uint32_t v_addr_start;
    uint32_t len;
    void *src_data;
} mem_section_t; */

int mem_section_init(mem_section_t *ms, uint32_t addr, uint32_t len,
                     uint32_t pde_flags, uint32_t pte_flags) {
    if (ms == NULL) return -1;
    ms->v_addr_start = addr;
    ms->len = len;
    ms->pde_f = pde_flags;
    ms->pte_f = pte_flags;

    return 0;
}


