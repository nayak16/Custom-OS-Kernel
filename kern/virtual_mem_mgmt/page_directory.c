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


#define NTH_BIT(v,n) (((uint32_t)v >> n) & 1)

#define NUM_ENTRIES (PAGE_SIZE/sizeof(uint32_t))

void *pd_get_base_addr(page_directory_t *pd){
    return (void *)(pd->directory);
}

int pd_init(page_directory_t *pd){
    /* page align allocation and clear out all present bits */
    pd->directory = memalign(PAGE_SIZE, NUM_ENTRIES * sizeof(uint32_t));
    if (pd->directory == NULL) return -1;
    memset(pd->directory, 0, NUM_ENTRIES * sizeof(uint32_t));
    return 0;
}

int pd_initialize_kernel(page_directory_t *pd){
    /* 4 page tables worth of entries = 4096 entries */
    uint32_t num_kernel_ptes = (USER_MEM_START >> PAGE_SHIFT);
    /* present, rw enabled, supervisor mode, dont flush */
    uint32_t pte_flags = NEW_FLAGS(SET,SET,UNSET,SET);
    /* present, rw enabled, supervisor mode */
    uint32_t pde_flags = NEW_FLAGS(SET,SET,UNSET,DONT_CARE);
    uint32_t i;
    /* for the first num_kernel_entries, set the vpn==ppn for direct map */
    /* Leave 0th page unmapped */
    for (i = 1; i < num_kernel_ptes; i++){
        vmm_create_mapping(i, i, pte_flags, pde_flags, pd);
    }

    return 0;
}

