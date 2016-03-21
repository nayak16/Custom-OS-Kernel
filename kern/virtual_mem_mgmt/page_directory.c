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

/* access to flush_tlb */
#include <special_reg_cntrl.h>


#include <simics.h>

#define NTH_BIT(v,n) (((uint32_t)v >> n) & 1)
/* number of total page table entries in the kernel space */
#define NUM_KERNEL_PTE (USER_MEM_START >> PAGE_SHIFT)
/* number of page directory entries in the kernel space (number page tables) */
#define NUM_KERNEL_PDE (NUM_KERNEL_PTE / PT_NUM_ENTRIES)

#define OFF_SHIFT PAGE_SHIFT
#define PDE_SHIFT 10
#define PTE_SHIFT 10

#define ENTRY_PRESENT 0
#define ENTRY_NOT_PRESENT 1

int entry_present(uint32_t v){
    if (NTH_BIT(v,0) == 0) return ENTRY_NOT_PRESENT;
    return ENTRY_PRESENT;
}

int pd_get_mapping(page_directory_t *pd, uint32_t v_addr,
        uint32_t **entry_addr){
    if (pd == NULL) return -1;
    uint32_t pde_i = (v_addr >> (OFF_SHIFT + PTE_SHIFT)) & 0x3FF;
    /* page table index = 2nd 10 bits of v_addr */
    uint32_t pte_i = (v_addr >> OFF_SHIFT) & 0x3FF;
    if (entry_present(pd->directory[pde_i]) == ENTRY_NOT_PRESENT){
        /* page directory entry not present */
        return -2;
    }
    uint32_t *pt = (uint32_t *)(REMOVE_FLAGS(pd->directory[pde_i]));
    if (entry_present(pt[pte_i]) == ENTRY_NOT_PRESENT){
        /* directory exists but table does not */
        return -3;
    }
    /* found mapping */
    if (entry_addr != NULL){
        *entry_addr = &(pt[pte_i]);
    }
    return 0;
}

int pd_create_mapping(page_directory_t *pd, uint32_t v_addr, uint32_t p_addr,
        uint32_t pte_flags, uint32_t pde_flags){
    /* input addresses must be page aligned */
    if (!IS_PAGE_ALIGNED(v_addr) || !IS_PAGE_ALIGNED(p_addr)) return -1;
    uint32_t pde_i, pte_i, pde_value, pte_value;

    /* page directory entry index = top 10 bits of v_addr */
    pde_i = (v_addr >> (OFF_SHIFT + PTE_SHIFT)) & 0x3FF;
    /* page table index = 2nd 10 bits of v_addr */
    pte_i = (v_addr >> OFF_SHIFT) & 0x3FF;
    /* page table entry value is the physical address with flags */
    pte_value = ADD_FLAGS(p_addr, pte_flags);

    /* create a table if needed */
    if (entry_present(pd->directory[pde_i]) != ENTRY_PRESENT){
        if ((pde_value = (uint32_t)memalign(PAGE_SIZE, PT_SIZE)) == 0)
            return -2;
        pde_value = ADD_FLAGS(pde_value, pde_flags);
        pd->directory[pde_i] = pde_value;
    } else {
        pde_value = pd->directory[pde_i];
    }

    /* get address of page table and save mapping */
    uint32_t *page_table = (uint32_t *)REMOVE_FLAGS(pde_value);
    page_table[pte_i] = pte_value;

    return 0;
}


int initialize_kernel(page_directory_t *pd){
    /* present, rw enabled, supervisor mode, dont flush */
    uint32_t pte_flags = NEW_FLAGS(SET,SET,UNSET,SET);
    /* present, rw enabled, supervisor mode */
    uint32_t pde_flags = NEW_FLAGS(SET,SET,UNSET,DONT_CARE);
    uint32_t i;
    /* for the first num_kernel_entries, set the vpn==ppn for direct map */
    /* Leave 0th page unmapped */
    for (i = 1; i < NUM_KERNEL_PTE; i++){
        uint32_t direct_addr = i << PAGE_SHIFT;
        pd_create_mapping(pd, direct_addr, direct_addr, pte_flags, pde_flags);
    }
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

void *get_page_address(uint32_t pd_i, uint32_t pt_i){
    return (void *)(pd_i << 22 | pt_i << 12);
}


int pd_shallow_copy(page_directory_t *pd_dest, page_directory_t *pd_src){
    if (pd_dest == NULL || pd_src == NULL) return -1;
    /* copy the upper level page directory */
    uint32_t i;
    /* copy over non-kernel space */
    for (i = NUM_KERNEL_PDE; i < PD_NUM_ENTRIES; i++){
        /* for each present entry, create a new page table  */
        uint32_t entry = pd_src->directory[i];
        if (entry_present(entry) == ENTRY_PRESENT){
            /* allocate a new page table */
            uint32_t *new_pt = memalign(PAGE_SIZE, PT_SIZE);
            uint32_t flags = EXTRACT_FLAGS(entry);
            /* map page directory to new page table */
            pd_dest->directory[i] = (uint32_t)new_pt | flags;
        }
    }
    return 0;
}


