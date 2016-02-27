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

int pd_init(page_directory_t *pd){
    /* clear out all present bits */
    memset(pd->directory, 0, sizeof(pd->directory));
    return 0;
}

int set_pte(int vpn, int ppn, int flags, page_directory_t *pd){
    /* the ppn to be stored in the page table entry */
    int page_table_entry_value = (ppn << PAGE_SHIFT) | flags;
    /* converts ith page table entry to page directory index */
    int page_directory_i = vpn / PAGE_SIZE;
    /* converts ith page table entry to page table index */
    int page_table_i = vpn % PAGE_SIZE;
    /* get the value stored in the correct page directory entry*/
    int page_table_value = pd->directory[page_directory_i];
    if (NTH_BIT(page_table_value, 0) == 0){
        /* page table does not exist, create it */
        int *page_table = memalign(PAGE_SIZE, sizeof(int) * PAGE_SIZE);
        pd->directory[page_directory_i] = ((int)page_table | flags);
    } else {
        /* mask out flags */
        int *page_table_addr = (int *)(page_table_value & 0xFFFFF000);
        page_table_addr[page_table_i] = page_table_entry_value;
    }
    return 0;
}

int pd_initialize_kernel(page_directory_t *pd){
    /* 4 page tables worth of entries = 4096 entries */
    int num_kernel_entries = (USER_MEM_START >> PAGE_SHIFT);
    int flag = 0;
    int i;
    /* for the first num_kernel_entries, set the vpn==ppn for direct map */
    for (i = 0; i < num_kernel_entries; i++){
        set_pte(i, i, flag, pd);
    }
    return 0;
}