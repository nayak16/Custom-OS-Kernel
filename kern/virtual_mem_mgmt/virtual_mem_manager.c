/** @file virtual_mem_manager.c
 *  @brief Implements virtual memory manipulation functions
 *
 *  @author Aatish Nayak (aatishn)
 *  @author Christopher Wei (cjwei)
 *  @bug No known bugs.
 */


/* memalign */
#include <malloc.h>
/* memset */
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

/* Page constants */
#include <x86/page.h>

#include <constants.h>
#include <page_directory.h>
#include <frame_manager.h>
#include <mem_section.h>

#include <simics.h>

#define NTH_BIT(v,n) (((uint32_t)v >> n) & 1)

#define NUM_ENTRIES (PAGE_SIZE/sizeof(uint32_t))

#define DIV_ROUND_UP(num, den) ((num + den -1) / den)

int vmm_create_mapping(uint32_t vpn, uint32_t ppn, uint32_t pte_flags,
                   uint32_t pde_flags, page_directory_t *pd){
    /* the ppn to be stored in the page table entry */
    uint32_t page_table_entry_value = (ppn << PAGE_SHIFT) | pte_flags;
    /* converts ith page table entry to page directory index */
    uint32_t page_directory_i = (vpn >> 10) & 0x3FF;
    /* converts ith page table entry to page table index */
    uint32_t page_table_i = vpn & 0x3FF;
    /* get the value stored in the correct page directory entry*/
    uint32_t page_directory_value = pd->directory[page_directory_i];
    uint32_t *page_table_addr;
    if (NTH_BIT(page_directory_value, 0) == 0){
        /* page table does not exist, create it and assign it to pd */
        page_table_addr = memalign(PAGE_SIZE, sizeof(uint32_t) * NUM_ENTRIES);
        memset(page_table_addr, 0, sizeof(uint32_t) * NUM_ENTRIES);
        if (page_table_addr == NULL) return -1;
        pd->directory[page_directory_i] = ((uint32_t)page_table_addr | pde_flags);
    } else {
        /* get address of page table from page directory */
        page_table_addr = (uint32_t *)(page_directory_value & MSB_20_MASK);
    }
    /* assign page table entry */
    page_table_addr[page_table_i] = page_table_entry_value;
    return 0;
}


int is_sufficient_memory(frame_manager_t *fm,
                         mem_section_t *secs, uint32_t num_secs) {

    int total_pages = 0;
    int s;
    for (s = 0 ; s < num_secs ; s++) {
        int n_pages = DIV_ROUND_UP(secs[s].len, PAGE_SIZE);
        total_pages += n_pages;
    }

    return total_pages <= fm_num_free_frames(fm);

}

int vmm_user_mem_alloc(page_directory_t *pd, frame_manager_t *fm,
                       mem_section_t *secs, uint32_t num_secs) {

    if(!is_sufficient_memory(fm, secs, num_secs)) return -1;

    int s;
    /* Loop through all memory sections to allocate */
    for (s = 0 ; s < num_secs ; s++) {
        mem_section_t ms = secs[s];

        uint32_t len = ms.len;
        /* cur_addr must be page aligned */
        uint32_t cur_addr = ms.v_addr_start;
        /* Allocate and map each page */
        while(len > 0) {
            //TODO: handle for errors (revert)
            void *p_addr;
            if (fm_alloc(fm, &p_addr) < 0) //Should never happen
                return -2;

            if (vmm_create_mapping(cur_addr >> PAGE_SHIFT,
                        ((uint32_t)p_addr) >> PAGE_SHIFT,
                        ms.pde_f, ms.pte_f, pd) < 0) return -1;

            /* note: len is unsigned so we must check for underflow */
            if (len < PAGE_SIZE) {
                break;
            }
            cur_addr += PAGE_SIZE;
            len -= PAGE_SIZE;
        }
        /* Copy over contents of section into newly mapped virtual address */
        if (ms.src_data != NULL){
            memcpy((void*) ms.v_addr_start, ms.src_data, ms.len);
        }
    }
    return 0;

}



