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

#include <simics.h>

#define NTH_BIT(v,n) (((uint32_t)v >> n) & 1)

#define NEW_FLAGS(p,rw,md,glb) ((p << PRESENT_FLAG_BIT) | (rw << RW_FLAG_BIT)\
    | (md << MODE_FLAG_BIT) | (glb << GLOBAL_FLAG_BIT))

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

int set_pte(uint32_t vpn, uint32_t ppn, uint32_t pte_flags, uint32_t pde_flags,
            page_directory_t *pd){
    /* the ppn to be stored in the page table entry */
    uint32_t page_table_entry_value = (ppn << PAGE_SHIFT) | pte_flags;
    /* converts ith page table entry to page directory index */
    uint32_t page_directory_i = vpn / NUM_ENTRIES;
    /* converts ith page table entry to page table index */
    uint32_t page_table_i = vpn % NUM_ENTRIES;
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
        set_pte(i, i, pte_flags, pde_flags, pd);
    }

    return 0;
}

