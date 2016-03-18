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
#include <virtual_mem_manager.h>

/* access to frame manager */
#include <kern_internals.h>

#include <simics.h>

/* access to flush_tlb */
#include <special_reg_cntrl.h>

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
    if (pd_entry_present(page_directory_value) != 0){
        /* page table does not exist, create it and assign it to pd */
        page_table_addr = memalign(PAGE_SIZE, PT_SIZE);
        if (page_table_addr == NULL) return -1;
        memset(page_table_addr, 0, PT_SIZE);
        pd->directory[page_directory_i] = ((uint32_t)page_table_addr | pde_flags);
    } else {
        /* get address of page table from page directory */
        page_table_addr = (uint32_t *)(page_directory_value & MSB_20_MASK);
    }
    /* assign page table entry */
    page_table_addr[page_table_i] = page_table_entry_value;
    return 0;
}

#define PAGE_ALIGN_UP(a) (PAGE_SIZE * DIV_ROUND_UP(a,PAGE_SIZE))
#define PAGE_ALIGN_DOWN(a) (PAGE_SIZE * (a/PAGE_SIZE))
int get_bounding_v_addr(mem_section_t *secs, uint32_t num_secs,
        uint32_t *v_addr_low, uint32_t *v_addr_high){
    if (secs == NULL || num_secs <= 0) return -1;
    uint32_t i, low, high;
    low = 0xFFFFFFFF;
    high = 0;
    for (i=0; i<num_secs; i++){
        uint32_t c_low = secs[i].v_addr_start;
        uint32_t c_high = c_low + secs[i].len;
        if (c_low < low) low = c_low;
        if (c_high > high) high = c_high;
    }
    if (high < low) return -1;
    /* save results */
    *v_addr_low = low;
    *v_addr_high = high;
    return 0;
}


int get_bounding_section(mem_section_t *secs, uint32_t num_secs,
        uint32_t v_addr_low, uint32_t v_addr_high,
        mem_section_t *ms){
    if (secs == NULL || num_secs <= 0 || ms == NULL) return -1;
    int i;
    for (i = 0; i < num_secs; i++){
        uint32_t s_low = secs[i].v_addr_start;
        uint32_t s_high = s_low + secs[i].len;
        if ((s_low <= v_addr_low && v_addr_low < s_high) ||
            (s_low < v_addr_high && v_addr_high <= s_high)){
            *ms = secs[i];
            return 0;
        }
    }
    // no matches found
    return 1;
}

int vmm_mem_alloc(page_directory_t *pd,
                       mem_section_t *secs, uint32_t num_secs) {
    uint32_t v_addr_low, v_addr_high, num_pages;
    /* gets the highest and lowest virtual address that need to be mapped */
    if (get_bounding_v_addr(secs, num_secs, &v_addr_low, &v_addr_high) < 0)
        return -1;

    /* align to page */
    v_addr_high = PAGE_ALIGN_UP(v_addr_high);
    v_addr_low = PAGE_ALIGN_DOWN(v_addr_low);

    /* number of pages needed to map memory sections */
    num_pages = (v_addr_high - v_addr_low)/PAGE_SIZE;

    /* if not enough frames avaliable to map area */
    if (num_pages > fm_num_free_frames(&fm)) return -2;

    int i;
    uint32_t v_addr = v_addr_low;
    MAGIC_BREAK;
    for (i = 0; i < num_pages; i++){
        void *p_addr;
        mem_section_t ms;

        /* get a new physical frame */
        if (fm_alloc(&fm, &p_addr) < 0) return -2;

        /* get corresponding section to that page */
        int status = get_bounding_section(secs, num_secs,
                v_addr, v_addr+PAGE_SIZE, &ms);
        if (status == -1){
            /* some error occured */
            return -1;
        } else if (status == 1){
            /* no matching sections found
             * use default flag values */
            ms.pde_f = NEW_FLAGS(SET, SET, SET, UNSET);
            ms.pde_f = NEW_FLAGS(SET, SET, SET, DONT_CARE);
        }

        /* create a mapping between virtual to phys */
        vmm_create_mapping(v_addr >> PAGE_SHIFT,
            ((uint32_t)p_addr) >> PAGE_SHIFT,
            ms.pde_f, ms.pte_f, pd);

        v_addr += PAGE_SIZE;
    }
    return 0;
}


int vmm_deep_copy_page(void **target_pte, void *v_addr, void **new_phys_addr){
    if (target_pte == NULL || v_addr == NULL || new_phys_addr == NULL)
        return -1;
    /* allocate a new buffer to hold our page contents locally */
    void *buffer = malloc(PAGE_SIZE);
    if (buffer == NULL) return -2;
    /* original page table entry */
    void *original_pte = *target_pte;
    uint32_t flags = ((uint32_t)original_pte & 0xFFF);
    /* target virtual address */
    memcpy(buffer, v_addr, PAGE_SIZE);
    if (fm_alloc(&fm, new_phys_addr) < 0) return -3;
    /* remap our virtual address to new physical address */
    *target_pte = (void *)((uint32_t)(*new_phys_addr) | flags);
    /* flush cached mappings for our virtual address */
    flush_tlb((uint32_t)v_addr);
    /* copy contents into new phys page */
    memcpy(v_addr, buffer, PAGE_SIZE);
    /* restore mapping */
    *target_pte = original_pte;
    /* flush out any false mappings */
    flush_tlb((uint32_t)v_addr);
    free(buffer);
    return 0;
}

