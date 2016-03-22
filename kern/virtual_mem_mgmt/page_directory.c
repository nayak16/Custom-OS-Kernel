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

/* Predeclaration */

int pd_init(page_directory_t *pd);
int pd_get_mapping(page_directory_t *pd, uint32_t v_addr, uint32_t **pte);
int pd_create_mapping(page_directory_t *pd, uint32_t v_addr, uint32_t p_addr,
        uint32_t pte_flags, uint32_t pde_flags);
int pd_remove_mapping(page_directory_t *pd, uint32_t v_addr);
int pd_entry_present(uint32_t v);
int pd_shallow_copy(page_directory_t *pd_dest, page_directory_t *pd_src);
int pd_map_sections(page_directory_t *pd, mem_section_t *secs,
        uint32_t num_secs);
void *pd_get_base_addr(page_directory_t *pd);

/* Helper functions */

/** @brief Finds from a pte or pde whether that entry is present
 *  @param v The pte/pte value
 *  @return ENTRY_PRESENT if present, ENTRY_NOT_PRESENT otherwise */
int entry_present(uint32_t v){
    if (NTH_BIT(v,0) == 0) return ENTRY_NOT_PRESENT;
    return ENTRY_PRESENT;
}

/** @brief Initializes the kernel mappings of a page directory
 *  @param pd The page directory
 *  @return 0 on success -1 on failure
 */
int initialize_kernel(page_directory_t *pd){
    if (pd == NULL) return -1;
    /* present, rw enabled, supervisor mode, dont flush */
    uint32_t pte_flags = NEW_FLAGS(SET,SET,UNSET,SET);
    /* present, rw enabled, supervisor mode */
    uint32_t pde_flags = NEW_FLAGS(SET,SET,UNSET,DONT_CARE);
    uint32_t i;
    /* for the first num_kernel_entries, set the vpn==ppn for direct map */
    /* Leave 0th page unmapped */
    for (i = 1; i < NUM_KERNEL_PTE; i++){
        uint32_t direct_addr = i << PAGE_SHIFT;
        if (pd_create_mapping(pd, direct_addr, direct_addr,
                    pte_flags, pde_flags) < 0){
            return -1;
        }
    }
    return 0;
}

/** @brief Gets the address of a page given its page table and page directory
 *         indexes
 *  @param pd_i The page directory index
 *  @param pt_i The page table index
 *  @return The page's address
 */
void *get_page_address(uint32_t pd_i, uint32_t pt_i){
    return (void *)(pd_i << 22 | pt_i << 12);
}

/* Implementation */

/** @brief Finds the mapping of a virtual address if it exists in a page
 *         directory and optionally returns the address of the entry
 *  @param pd The page directory
 *  @param v_addr The virtual address to look up
 *  @param entry_addr Where the address of the pte is stored upon return
 *  @return 0 on success, negative integer code on failure
 */
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


/** @brief Creates a mapping in a page directory with given flags
 *
 *  Requires that both v_addr and p_addr are page aligned
 *
 *  @param pd The page directory
 *  @param v_addr The virtual address to map
 *  @param p_addr The physical frame to map to
 *  @param pte_flags The page table entry flags
 *  @param pde_flags The page directory entry flags (if new page table is made)
 *  @return 0 on success, negative integer code on failure
 */
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
        memset((void *)pde_value, 0, PAGE_SIZE);
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

int pd_remove_mapping(page_directory_t *pd, uint32_t v_addr){
    if (!IS_PAGE_ALIGNED(v_addr) || pd == NULL) return -1;
    uint32_t pde_i, pte_i;
    /* page directory entry index = top 10 bits of v_addr */
    pde_i = (v_addr >> (OFF_SHIFT + PTE_SHIFT)) & 0x3FF;
    /* page table index = 2nd 10 bits of v_addr */
    pte_i = (v_addr >> OFF_SHIFT) & 0x3FF;

    /* check for present page directory entry */
    if (entry_present(pd->directory[pde_i]) != ENTRY_PRESENT)
        return -2;
    uint32_t *page_table = (uint32_t *)REMOVE_FLAGS(pd->directory[pde_i]);
    if (entry_present(page_table[pte_i]) != ENTRY_PRESENT)
        return -3;
    /* clear the mapping */
    page_table[pte_i] = 0;
    return 0;
}
/** @brief Returns the directory in a page directory struct
 *  @param pd The page directory
 *  @return The directory
 */
void *pd_get_base_addr(page_directory_t *pd){
    return (void *)(pd->directory);
}


/** @brief Initializes a page directory
 *  @param The page directory
 *  @return 0 on success, -1 on failure
 */
int pd_init(page_directory_t *pd){
    /* page align allocation and clear out all present bits */
    pd->directory = memalign(PAGE_SIZE, PD_SIZE);
    if (pd->directory == NULL){
        return -1;
    }
    memset(pd->directory, 0, PD_SIZE);
    if (initialize_kernel(pd) < 0){
        free(pd->directory);
        pd->directory = NULL;
        return -1;
    }
    return 0;
}


/** @brief Shallow copies a page directory
 *
 *  Both pd_dest and pd_src are expected to be pd_init'ed. Only copies
 *  page directory entries that are present and are not in the kernel.
 *  After execution, the same page directory entry indexes of pd_dest
 *  and pd_src will be filled but  pd_dest will point to different (and empty)
 *  page tables then that of pd_src.
 *
 *  @param pd_dest The page directory to copy to
 *  @param pd_src The page directory copying from
 *  @return 0 On success -1 on failure
 */
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
            if (new_pt == NULL)
                //TODO: roll back changes
                return -1;
            memset((void *)new_pt, 0, PAGE_SIZE);
            uint32_t flags = EXTRACT_FLAGS(entry);
            /* map page directory to new page table */
            pd_dest->directory[i] = (uint32_t)new_pt | flags;
        }
    }
    return 0;
}


