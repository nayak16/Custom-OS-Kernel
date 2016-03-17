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
#define NUM_KERNEL_PDE (NUM_KERNEL_PTE / PD_SIZE);

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

/* */
int pd_copy(page_directory_t *pd_dest, page_directory_t *pd_src){
    return 0;
}
