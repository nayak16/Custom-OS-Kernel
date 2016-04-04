#include <simics.h>
#include <kern_internals.h>
#include <frame_manager.h>
/* NULL */
#include <stdlib.h>
/* memset */
#include <string.h>
/* USER_MEM_START */
#include <common_kern.h>
/* tlb_flush */
#include <special_reg_cntrl.h>

#include <debug.h>



/** @brief Deep copies the current page directory into pd_dest
 *
 *  Sets pd_dest to the same structure as the current active directory
 *  but with different page directory entries pointing to new page tables,
 *  new page table entries pointing to new physical frames, and new physical
 *  frames containing the same information as it's copy
 *
 *  @param pd_dest The page directory to copy to
 *  @return 0 on success, negative integer code on failure */
int vmm_deep_copy(page_directory_t *pd_dest){
    if (pd_dest == NULL)
        return -1;

    /* Get current running pcb */
    pcb_t *cur_pcb;
    if(scheduler_get_current_pcb(&sched, &cur_pcb) < 0) {
        return -2;
    }

    page_directory_t *pd_src = &(cur_pcb->pd);

    uint32_t p_addr_start;
    if (fm_alloc(&fm, pd_src->num_pages, &p_addr_start) < 0){
        lprintf("Failed allocate %d pages in vmm_deep_copy",
                (unsigned int)pd_dest->num_pages);
        MAGIC_BREAK;
    }
    pd_alloc_frame(pd_dest, p_addr_start, pd_src->num_pages);

    /* deep copy page directory structure */
    if (pd_deep_copy(pd_dest, pd_src, p_addr_start) < 0)
        return -1;
    return 0;
}


/** @brief Maps multiple memory sections into pd
 *
 *  Requires that sections that share the same page table have the same
 *  permissioning
 *
 *  @param pd The page directory to map to
 *  @param secs The array of memory sections
 *  @param num_secs The number of sections to map
 *  @return 0 on success, negative integer code on failure
 */
int vmm_map_sections(page_directory_t *pd, mem_section_t *secs,
        uint32_t num_secs) {
    if (pd == NULL || secs == NULL || num_secs == 0) return -1;

    uint32_t v_addr_low, v_addr_high;
    ms_get_bounding_addr(secs, num_secs, &v_addr_low, &v_addr_high);

    v_addr_low = PAGE_ALIGN_DOWN(v_addr_low);
    v_addr_high = PAGE_ALIGN_UP(v_addr_high)-1;

    uint32_t num_pages =((v_addr_high-v_addr_low)+1)/PAGE_SIZE;
    /* check for enough frames */
    uint32_t cur_addr = v_addr_low;
    uint32_t p_addr;

    /* Allocate all the frames */
    if (fm_alloc(&fm, num_pages, &p_addr) < 0){
        panic("Could not find enough frames to map sections!");
    }
    /* Update PD's frame tracker */
    if (pd_alloc_frame(pd, p_addr, num_pages) < 0){
        panic("Could not allocate frame to page directory!");
    }

    /* for each page allocate a frame and map it */
    int i;
    for (i = 0; i < num_pages; i++){
        uint32_t pte_f, pde_f;
        mem_section_t *ms = NULL;
        if (ms_get_bounding_section(secs, num_secs, cur_addr,
                    cur_addr + (PAGE_SIZE-1), &ms) < 0)
            return -3;
        if (ms == NULL){
            /* a page does not belong to any memory section,
             * but is bounded by v_addr_high and v_addr_low so it should
             * be mapped to read only */
            pte_f = PTE_FLAG_DEFAULT;
            pde_f = PDE_FLAG_DEFAULT;
        } else {
            /* retrieve memory section's flags */
            pte_f = ms->pte_f;
            pde_f = ms->pde_f;
        }
        /* create the mapping */
        if (pd_create_mapping(pd, cur_addr, p_addr,
                    pte_f, pde_f) < 0) return -4;
        cur_addr += PAGE_SIZE;
        p_addr += PAGE_SIZE;
    }
    /* zero out newly mapped memory */
    memset((void *)v_addr_low, 0, num_pages*PAGE_SIZE/sizeof(void*));
    return 0;
}


/** @brief Attempts to allocate a new user space page
 *
 *  Will return negative integer code if pd is NULL, if num_pages is something
 *  unreasonable, or if base is not in the user space.
 *
 *  Requires base is page aligned
 *
 *  Uses non-x86 9th flag bit to signifiy that it is removeable
 *
 *  @param pd The page directory
 *  @param base The starting address to allocate from
 *  @param num_pages The number of pages to allocate
 */
int vmm_new_user_page(page_directory_t *pd, uint32_t base, uint32_t num_pages){
    if (pd == NULL || num_pages == 0 || num_pages > 0xFFFF)
        return -1;
    uint32_t v_addr = base;
    uint32_t i;
    /* check for overflow */
    if (base + (num_pages * PAGE_SIZE) - 1 < base){
        return -2;
    }

    /* check for overlapping regions of memeory */
    for (i = 0; i < num_pages; i++){
        if (pd_get_mapping(pd, v_addr, NULL) == 0){
            return -3;
        }
        v_addr += PAGE_SIZE;
    }
    /* allocate frames and create the mapping */
    uint32_t p_addr;
    if (fm_alloc(&fm, num_pages, &p_addr) < 0){
        lprintf("vmm_new_user_page unable to fm_alloc");
        return -2;
    }
    if (pd_alloc_frame(pd, p_addr, num_pages) < 0){
        panic("Could not allocate frame to page directory");
    }

    v_addr = base;
    for (i = 0; i < num_pages; i++){
        uint32_t pte_f = USER_WR;
        uint32_t pde_f = USER_WR;
        /* add custom flags to denote start and stop of a user allocated
         * page table entry */
        if (i == 0){
            pte_f = ADD_USER_START_FLAG(pte_f);
        }
        if (i == num_pages-1){
            pte_f = ADD_USER_END_FLAG(pte_f);
        }
        if (pd_create_mapping(pd, v_addr, p_addr, pte_f, pde_f) < 0)
            return -1;

        memset((void *)v_addr, 0, PAGE_SIZE);
        v_addr += PAGE_SIZE;
        p_addr += PAGE_SIZE;
    }
    return 0;
}

int vmm_remove_user_page(page_directory_t *pd, uint32_t base){
    if (pd == NULL || base < USER_MEM_START || !IS_PAGE_ALIGNED(base))
        return -1;
    uint32_t pte = 0;
    /* check to see if base is mapped */
    if (pd_get_mapping(pd, base, &pte) < 0){
        return -2;
    }
    /* check to ensure that said page table is the start of a new_pages
     * allocation */
    if (!IS_USER_START(pte)){
        return -3;
    } else {
        /* lookup frame in the fm allocated frame pool */
        if (fm_dealloc(&fm, REMOVE_FLAGS(pte)) < 0){
            lprintf("Could not find frame in allocated frame pool");
            return -4;
        }
        if (pd_dealloc_frame(pd, REMOVE_FLAGS(pte)) < 0){
            lprintf("Could not deallocate user frame");
        }
    }
    /* go through all the addresses until we get an address that signifies
     * the end of a user new_pages */
    uint32_t v_addr = base;
    pte = 0;
    do {
        if (v_addr - PAGE_SIZE > v_addr)
            panic("Overflowed while deallocating stack space!!");
        /* get the mapping so we can check if we're done */
        pd_get_mapping(pd, v_addr, &pte);
        /* remove mapping */
        pd_remove_mapping(pd, v_addr);
        /* flush mapping in tlb */
        flush_tlb((uint32_t)v_addr);
        /* give frame back to frame manager */
        v_addr += PAGE_SIZE;
    } while (!IS_USER_END(pte));

    return 0;
}

int vmm_clear_user_space(page_directory_t *pd){
    uint32_t v_addr, pte, i;
    /* deallocate all frames from page directory; use the resulting list
     * to update the frame manager */
    int num_frames = pd_num_frames(pd);
    uint32_t frames[num_frames];
    pd_dealloc_all_frames(pd, frames);
    for (i = 0; i < num_frames; i++){
        fm_dealloc(&fm, frames[i]);
    }
    /* go until we overflow */
    for (v_addr = USER_MEM_START; v_addr != 0; v_addr+=PAGE_SIZE){
        pd_get_mapping(pd, v_addr, &pte);
        /* remove mapping */
        pd_remove_mapping(pd, v_addr);
    }
    /* flush all mapping in tlb */
    flush_all_tlb();

    return 0;
}
