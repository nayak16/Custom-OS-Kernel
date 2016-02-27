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

#include <page_directory.h>

#define NTH_BIT(v,n) (((int)v >> n) & 1)

#define SET 1
#define UNSET 0
#define DONTCARE 0

#define NEW_FLAGS(p,rw,md,glb) ((p << PRESENT_FLAG_BIT) | (rw << RW_FLAG_BIT)\
    | (md << MODE_FLAG_BIT) | (glb << GLOBAL_FLAG_BIT))

int pd_init(page_directory_t *pd){
    /* clear out all present bits */
    memset(pd->directory, 0, sizeof(pd->directory));
    return 0;
}

int set_pte(int vpn, int ppn, int pte_flags, int pde_flags,
            page_directory_t *pd){
    /* the ppn to be stored in the page table entry */
    int page_table_entry_value = (ppn << PAGE_SHIFT) | pte_flags;
    /* converts ith page table entry to page directory index */
    int page_directory_i = vpn / PAGE_SIZE;
    /* converts ith page table entry to page table index */
    int page_table_i = vpn % PAGE_SIZE;
    /* get the value stored in the correct page directory entry*/
    int page_table_value = pd->directory[page_directory_i];
    int *page_table_addr;
    if (NTH_BIT(page_table_value, 0) == 0){
        /* page table does not exist, create it and assign it to pd */
        page_table_addr = memalign(PAGE_SIZE, sizeof(int) * PAGE_SIZE);
        pd->directory[page_directory_i] = ((int)page_table_addr | pde_flags);
    } else {
        /* get address of page table from page directory */
        page_table_addr = (int *)(page_table_value & 0xFFFFF000);
    }
    /* assign page table entry */
    page_table_addr[page_table_i] = page_table_entry_value;
    return 0;
}

int pd_initialize_kernel(page_directory_t *pd){
    /* 4 page tables worth of entries = 4096 entries */
    int num_kernel_entries = (USER_MEM_START >> PAGE_SHIFT);
    /* present, rw enabled, supervisor mode, dont flush */
    int pte_flags = NEW_FLAGS(SET,SET,UNSET,SET);
    /* present, rw enabled, supervisor mode */
    int pde_flags = NEW_FLAGS(SET,SET,UNSET,DONTCARE);
    int i;
    /* for the first num_kernel_entries, set the vpn==ppn for direct map */
    for (i = 0; i < num_kernel_entries; i++){
        set_pte(i, i, pte_flags, pde_flags, pd);
    }
    return 0;
}