/** @file page_directory.c
 *  @brief Implements page directory
 *
 *  @author Aatish Nayak (aatishn)
 *  @author Christopher Wei (cjwei)
 *  @bug No known bugs.
 */


/* USER_MEM_START */
#include <common_kern.h>
/* memalign */
#include <malloc.h>
/* memset */
#include <string.h>
/* flag sets */
#include <constants.h>

#include <page_directory.h>
#include <virtual_mem_manager.h>

#include <simics.h>

#define NTH_BIT(v,n) (((uint32_t)v >> n) & 1)
#define NUM_KERNEL_PTE (USER_MEM_START >> PAGE_SHIFT)
#define NUM_KERNEL_PDE (NUM_KERNEL_PTE / PD_SIZE)

int initialize_kernel(page_directory_t *pd){
    /* present, rw enabled, supervisor mode, dont flush */
    uint32_t pte_flags = NEW_FLAGS(SET,SET,UNSET,SET);
    /* present, rw enabled, supervisor mode */
    uint32_t pde_flags = NEW_FLAGS(SET,SET,UNSET,DONT_CARE);
    uint32_t i;
    /* for the first num_kernel_entries, set the vpn==ppn for direct map */
    /* Leave 0th page unmapped */
    for (i = 1; i < NUM_KERNEL_PTE; i++){
        vmm_create_mapping(i, i, pte_flags, pde_flags, pd);
    }
    return 0;
}

int pd_entry_present(uint32_t v){
    if (NTH_BIT(v,0) == 0) return -1;
    return 0;
}

int pt_entry_present(uint32_t v){
    if (NTH_BIT(v,0) == 0) return -1;
    return 0;
}

void *pd_get_base_addr(page_directory_t *pd){
    return (void *)(pd->directory);
}

int pd_init(page_directory_t *pd){
    /* page align allocation and clear out all present bits */
    pd->directory = memalign(PAGE_SIZE, PD_SIZE);
    if (pd->directory == NULL) return -1;
    memset(pd->directory, 0, PD_SIZE);
    initialize_kernel(pd);
    return 0;
}

/** @brief Deep copy page table from src to dest */
int pt_copy(uint32_t *pt_dest, uint32_t *pt_src){
    uint32_t i;
    /* copy each page table entry */
    for (i = 0; i < PT_SIZE; i++){
        uint32_t entry = pt_src[i];
        uint32_t flags = entry & 0xFFF;
        /* copy over present mappings */
        if (pt_entry_present(entry) == 0){
            void *p_addr;
            vmm_deep_copy_page((void **)&entry, &p_addr);
            pt_dest[i] = (uint32_t)p_addr | flags;
        }
    }
    return 0;
}

int pd_copy(page_directory_t *pd_dest, page_directory_t *pd_src){
    /* TODO: handle errors */
    /* copy the upper level page directory */
    uint32_t i;
    /* copy over non-kernel space */
    for (i = NUM_KERNEL_PDE; i < PD_NUM_ENTRIES; i++){
        /* for each present entry, create a new page table  */
        uint32_t entry = pd_src->directory[i];
        if (pd_entry_present(entry) == 0){
            /* allocate a new page table */
            uint32_t *new_pt = memalign(PAGE_SIZE, PT_SIZE);
            uint32_t flags = entry & 0xFFF;
            /* map page directory to new page table */
            pd_dest->directory[i] = (uint32_t)new_pt | flags;
            /* copy pages */
            pt_copy(new_pt, (uint32_t*)(entry & ~0xFFF));
        }
    }
    return 0;
}


