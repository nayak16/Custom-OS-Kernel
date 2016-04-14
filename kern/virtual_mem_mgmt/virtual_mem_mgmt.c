/** @file virtual_mem_mgmt.c
 *  @brief Implements the virtual memory manager
 *
 *  The virtual memory manager is the main endpoint for the kernel to manage
 *  virtual memory mappings. It combines two data structures: the frame manager
 *  and the page directory and manages finding physical frames from the frame
 *  manager to map in the page directory, as well as removing mappings from the
 *  page directory and returning frames to the frame manager.
 *
 *  @author Christopher Wei (cjwei)
 *  @author Aatish Nayak (aatishn)
 *  @bug Not neccesarily a bug, but at some point we would like to abstract out
 *       the idea of a frame manager and a page directory and simply have a vmm
 *       object.
 *  @bug vmm_clear_user_space does not handle errors
 */


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
    if (pd_dest == NULL) return -1;

    /* Get current running pcb */
    pcb_t *cur_pcb;
    if(scheduler_get_current_pcb(&sched, &cur_pcb) < 0) {
        return -2;
    }

    page_directory_t *pd_src = &(cur_pcb->pd);

    uint32_t p_addr_start;
    if (fm_alloc(&fm, pd_src->num_pages, &p_addr_start) < 0){
        DEBUG_PRINT("Failed allocate %d pages in vmm_deep_copy",
                (unsigned int)pd_dest->num_pages);
        return -3;
    }
    if (pd_alloc_frame(pd_dest, p_addr_start, pd_src->num_pages) < 0){
        fm_dealloc(&fm, p_addr_start);
        return -4;
    }

    /* deep copy page directory structure */
    if (pd_deep_copy(pd_dest, pd_src, p_addr_start) < 0){
        fm_dealloc(&fm, p_addr_start);
        pd_dealloc_frame(pd_dest, p_addr_start, NULL);
        return -5;
    }
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
    if (num_pages == 0) return 0;

    /* check for enough frames */
    uint32_t cur_addr = v_addr_low;
    uint32_t p_addr;

    /* Allocate all the frames */
    if (fm_alloc(&fm, num_pages, &p_addr) < 0){
        return -2;
    }
    /* Update PD's frame tracker */
    if (pd_alloc_frame(pd, p_addr, num_pages) < 0){
        fm_dealloc(&fm, p_addr);
        return -3;
    }

    /* map each page to the corresponding physical page */
    int i;
    uint32_t p_addr_start = p_addr;
    if (pd_begin_mapping(pd) < 0){
        fm_dealloc(&fm, p_addr_start);
        pd_dealloc_frame(pd, p_addr_start, NULL);
        return -3;
    }
    for (i = 0; i < num_pages; i++){
        uint32_t pte_f, pde_f;
        mem_section_t *ms = NULL;
        if (ms_get_bounding_section(secs, num_secs, cur_addr,
                    cur_addr + (PAGE_SIZE-1), &ms) < 0){
            pd_abort_mapping(pd);
            fm_dealloc(&fm, p_addr_start);
            pd_dealloc_frame(pd, p_addr_start, NULL);
            return -3;
        }
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
                    pte_f, pde_f) < 0){
            pd_abort_mapping(pd);
            fm_dealloc(&fm, p_addr_start);
            pd_dealloc_frame(pd, p_addr_start, NULL);
            return -4;
        }
        cur_addr += PAGE_SIZE;
        p_addr += PAGE_SIZE;
    }
    pd_commit_mapping(pd);
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
 *  @return 0 on success, negative integer code on failure
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
        return -2;
    }
    if (pd_alloc_frame(pd, p_addr, num_pages) < 0){
        fm_dealloc(&fm, p_addr);
        return -3;
    }

    v_addr = base;
    uint32_t p_addr_start = p_addr;

    if (pd_begin_mapping(pd) < 0){
        fm_dealloc(&fm, p_addr_start);
        pd_dealloc_frame(pd, p_addr_start, NULL);
        return -4;
    }
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
        if (pd_create_mapping(pd, v_addr, p_addr, pte_f, pde_f) < 0){
            pd_abort_mapping(pd);
            pd_dealloc_frame(pd, p_addr_start, NULL);
            fm_dealloc(&fm, p_addr_start);
            return -4;
        }

        v_addr += PAGE_SIZE;
        p_addr += PAGE_SIZE;
    }
    pd_commit_mapping(pd);
    memset((void *)base, 0, num_pages*PAGE_SIZE);
    return 0;
}
/** @brief Removes user pages created by vmm_new_user_page
 *
 *  We know about how long each page is due to the flags set by
 *  vmm_new_user_page. Therefore, we can use these flags to ensure that the
 *  address given to us is in fact the beginning of a new user page and it also
 *  tells us when to stop removing pages.
 *
 *  @param pd The page directory
 *  @param base The base address of the page to be deallocated
 *  @return 0 on success, negative integer code on failure
 */
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
    }
    /* go through all the addresses until we get an address that signifies
     * the end of a user new_pages */
    uint32_t v_addr = base;
    uint32_t p_addr_base = REMOVE_FLAGS(pte);
    pte = 0;

    pd_begin_mapping(pd);
    do {
        if (v_addr - PAGE_SIZE > v_addr)
            panic("Overflowed while deallocating stack space!!");
        /* get the mapping so we can check if we're done */
        pd_get_mapping(pd, v_addr, &pte);
        /* remove mapping */
        if (pd_remove_mapping(pd, v_addr) < 0){
            pd_abort_mapping(pd);
            return -4;
        }
        /* flush mapping in tlb */
        flush_tlb((uint32_t)v_addr);
        /* give frame back to frame manager */
        v_addr += PAGE_SIZE;
    } while (!IS_USER_END(pte));

    uint32_t frame_size;
    if (pd_dealloc_frame(pd, p_addr_base, &frame_size) < 0){
        pd_abort_mapping(pd);
        return -5;
    }
    if (fm_dealloc(&fm, p_addr_base) < 0){
        pd_alloc_frame(pd, p_addr_base, frame_size);
        pd_abort_mapping(pd);
        return -6;
    }
    pd_commit_mapping(pd);
    return 0;
}

/** @brief Completely removes the user space of a page directory
 *
 *  Deallocates all frames from the page directory, returns the frames to the
 *  frame manager, and clears all mappings from the page directory and from the
 *  tlb
 *
 *  @param pd The page directory
 *  @return 0 on success, negative integer code on failure
 */
int vmm_clear_user_space(page_directory_t *pd){
    uint32_t i;

    int num_frames = pd_num_frames(pd);
    uint32_t frames[num_frames];
    uint32_t sizes[num_frames];

    pd_dealloc_all_frames(pd, frames, sizes);

    /* Note: I have not implemented any way to undo fm_dealloc calls so no
     * errors are for here. If a frame is unable to be deallocated into the
     * frame manager, it is still possible for the kernel to keep running albeit
     * with less pages avaliable */
    for (i = 0; i < num_frames; i++){
        fm_dealloc(&fm, frames[i]);
    }

    /* deallocate all frames from page directory; use the resulting list
     * to update the frame manager */
    pd_clear_user_space(pd);
    /* flush all mapping in tlb */
    flush_all_tlb();

    return 0;
}
