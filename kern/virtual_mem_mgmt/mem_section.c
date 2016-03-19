/** @file mem_section.c
 *  @brief Implementation of memory section functions
 *
 *  @author Aatish Nayak (aatishn)
 *  @author Christopher Wei (cjwei)
 *  @bug No known bugs.
 */
#include <stdlib.h>

#include <mem_section.h>

int ms_init(mem_section_t *ms, uint32_t addr, uint32_t len,
                     uint32_t pde_flags, uint32_t pte_flags) {
    if (ms == NULL) return -1;
    ms->v_addr_start = addr;
    ms->len = len;
    ms->pde_f = pde_flags;
    ms->pte_f = pte_flags;

    return 0;
}

int ms_get_bounding_addr(mem_section_t *secs, uint32_t num_secs,
        uint32_t *addr_low, uint32_t *addr_high){
    if (secs == NULL || num_secs == 0 || addr_low == NULL ||
            addr_high == NULL) return -1;
    int s;
    uint32_t low = 0xFFFFFFFF;
    uint32_t high = 0x0;
    for (s = 0; s < num_secs; s++){
        if (secs[s].len == 0) return -1;
        uint32_t c_low = secs[s].v_addr_start;
        uint32_t c_high = secs[s].v_addr_start + (secs[s].len - 1);
        if (c_low < low) low = c_low;
        if (c_high > high) high = c_high;
    }
    if (low > high) return -1;
    *addr_low = low;
    *addr_high = high;
    return 0;
}

int ms_get_bounding_section(mem_section_t *secs, uint32_t num_secs,
        uint32_t addr_low, uint32_t addr_high, mem_section_t **result){
    /* NOTE: low is inclusive and high is exclusive */
    if (secs == NULL || num_secs == 0 || (addr_low > addr_high) ||
            result == NULL) return -1;
    int i;
    for (i=0; i < num_secs; i++){
        uint32_t c_low = secs[i].v_addr_start;
        uint32_t c_high = secs[i].v_addr_start + (secs[i].len - 1);
        if ((c_low <= addr_low && addr_low <= c_high) ||
            (c_low <= addr_high && addr_high <= c_high)){
            *result = &(secs[i]);
            return 0;
        }
    }
    /* no result found */
    return 1;
}

