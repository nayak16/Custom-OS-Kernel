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
/* requires target_pte is in the current page_directory */
int copy_page(page_directory_t *pd_dest, uint32_t *target_pte,
        void *v_addr, void *p_addr){
    if (pd_dest == NULL || target_pte == NULL)
        return -1;
    /* allocate a new buffer to hold our page contents locally */
    void *buffer = malloc(PAGE_SIZE);
    if (buffer == NULL) return -2;
    /* original page table entry */
    uint32_t original_pte = *target_pte;
    uint32_t flags = EXTRACT_FLAGS(original_pte);
    /* target virtual address */
    memcpy(buffer, v_addr, PAGE_SIZE);
    /* remap our virtual address to new physical address */
    *target_pte = ADD_FLAGS(p_addr, flags);
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


int vmm_deep_copy(page_directory_t *pd_dest){
    if (pd_dest == NULL)
        return -1;

    /* Grab scheduler lock */
    mutex_lock(&scheduler_lock);
    /* Get current running pcb */
    pcb_t *cur_pcb;
    if(scheduler_get_current_pcb(&sched, &cur_pcb) < 0) {
        return -2;
    }
    mutex_unlock(&scheduler_lock);

    page_directory_t *pd_src = &(cur_pcb->pd);
    /* shallow copy page directory structure */
    pd_shallow_copy(pd_dest, pd_src);
    uint32_t v_addr;
    /* v_addr != 0 takes advantage of overflowing after finishing
     * mapping page 0xFFFFF000 -> 0xFFFFFFFF.
     * This assumes that USER_MEM_START != 0, and is page aligned
     * which are reasonable assumptions */
    for (v_addr = USER_MEM_START; v_addr != 0; v_addr += PAGE_SIZE){
        /* see if we need to copy a page */
        uint32_t *pte;
        uint32_t p_addr;
        if (pd_get_mapping(pd_src, v_addr, &pte) == 0){
            //TODO: implement some sort of queue that we can
            // eventually reverse
            if (fm_alloc(&fm, (void **)&p_addr) < 0){
                panic("Not enough frames; TODO: give back allocated frames");
            }
            copy_page(pd_dest, pte, (void *)v_addr, (void *)p_addr);

            /* don't care about pde_flags since the directory
             * should already have been mapped and all new page tables
             * have been allocated already */
            uint32_t pte_flags = EXTRACT_FLAGS(*pte);
            uint32_t pde_flags = 0; //DONT CARE
            pd_create_mapping(pd_dest, (uint32_t)v_addr, (uint32_t)p_addr,
                pte_flags, pde_flags);
        }
    }
    return 0;
}

int vmm_map_sections(page_directory_t *pd, mem_section_t *secs,
        uint32_t num_secs) {
    if (pd == NULL || secs == NULL || num_secs == 0) return -1;

    uint32_t v_addr_low, v_addr_high;
    ms_get_bounding_addr(secs, num_secs, &v_addr_low, &v_addr_high);

    v_addr_low = PAGE_ALIGN_DOWN(v_addr_low);
    v_addr_high = PAGE_ALIGN_UP(v_addr_high)-1;

    uint32_t num_pages =((v_addr_high-v_addr_low)+1)/PAGE_SIZE;
    /* check for enough frames */
    if (num_pages > fm_num_free_frames(&fm)) return -2;
    uint32_t cur_addr = v_addr_low;

    /* for each page allocate a frame and map it */
    int i;
    for (i = 0; i < num_pages; i++){
        uint32_t p_addr, pte_f, pde_f;
        mem_section_t *ms = NULL;
        if (fm_alloc(&fm, (void **)&p_addr) < 0){
            panic("Cannot allocate enough frames despite\
                    having enough frames avaliable");
        }
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
    }
    /* zero out newly mapped memory */
    memset((void *)v_addr_low, 0, num_pages*PAGE_SIZE);
    return 0;
}

int vmm_new_user_page(page_directory_t *pd, uint32_t base, uint32_t num_pages){
    if (pd == NULL || num_pages == 0)
        return -1;
    /* check for enough physical frames */
    if (fm_num_free_frames(&fm) < num_pages)
        return -2;
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
    v_addr = base;
    for (i = 0; i < num_pages; i++){
        uint32_t p_addr;
        if (fm_alloc(&fm, (void **)&p_addr) < 0){
            panic("Uh oh...");
        }
        pd_create_mapping(pd, v_addr, p_addr, USER_WR, USER_WR);
        v_addr += PAGE_SIZE;
    }
    return 0;
}
