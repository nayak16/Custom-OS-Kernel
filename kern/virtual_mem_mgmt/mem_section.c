/** @file mem_section.c
 *  @brief Implementation of memory section functions
 *
 *  @author Aatish Nayak (aatishn)
 *  @author Christopher Wei (cjwei)
 *  @bug No known bugs.
 */
#include <stdlib.h>

#include <mem_section.h>

/** @brief Initializes a memory section with given parameters
 *  @param ms The memory section
 *  @param addr The starting address of the memory section
 *  @param len The length of the memory section in bytes
 *  @param pde_flags The flags for the page directory entry
 *  @param pte_flags The flags for the page table entry
 *  @return 0 on success, -1 on failure */
int ms_init(mem_section_t *ms, uint32_t addr, uint32_t len,
                     uint32_t pde_flags, uint32_t pte_flags) {
    if (ms == NULL) return -1;
    ms->v_addr_start = addr;
    ms->len = len;
    ms->pde_f = pde_flags;
    ms->pte_f = pte_flags;

    return 0;
}

/** @brief Gets the bounding addresses of multiple memory sections
 *  @param secs The array of sections
 *  @param num_secs The number of sections in the array
 *  @param addr_low Where the lower bound address is stored
 *  @param addr_high Where the upper bound address is stored */
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


/** @brief Given an address range, finds a memory section which contains
 *         part or all of the given address range from an array of sections
 *  @param secs The array of memory sections
 *  @param num_secs The length of secs
 *  @param addr_low The lower bound range of address space
 *  @param addr_high The upper bound range of address space
 *  @param result Where the result is stored
 *  @return 0 on success, -1 on invalid input, 1 on valid input but no bounding
 *          section found */
int ms_get_bounding_section(mem_section_t *secs, uint32_t num_secs,
        uint32_t addr_low, uint32_t addr_high, mem_section_t **result){
    /* low and high are inclusive */
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

