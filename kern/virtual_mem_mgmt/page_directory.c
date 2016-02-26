/** @file page_directory.c
 *  @brief Implements page directory
 *
 *  @author Aatish Nayak (aatishn)
 *  @author Christopher Wei (cjwei)
 *  @bug No known bugs.
 */


/* USER_MEM_START */
#include <common_kern.h>
/* PAGE_SIZE */
#include <syscall.h>

#include <page_directory.h>

#define NTH_BIT(v,n) (((int)v >> n) & 1)

/*
typedef struct page_directory {
    int directory[1024];
} page_directory_t;*/

int pd_init(page_directory_t *pd){
    pd->directory = {0};
}

int set_pte(int i, int value, int flags, page_directory_t *pd){
    /* the value to be stored in the page table entry */
    int page_table_entry_value = (value << 12) | flags;
    /* converts ith page table entry to page directory index */
    int page_directory_i = i / PAGE_SIZE;
    /* converts ith page table entry to page table index */
    int page_table_i = i % PAGE_SIZE;
    /* get the value stored in the correct page directory entry*/
    int page_table_value = pd->directory[page_directory_i];
    if (NTH_BIT(page_table_value, 0) == 0){
        /* page table does not exist, create it */
        int *page_table = memalign(PAGE_SIZE, sizeof(int) * PAGE_SIZE);
        pd->directory[page_directory_i] = (page_table | flags);
    } else {
        /* mask out flags */
        int *page_table_addr = (int *)(page_table_value & 0xFFFFF000);
        page_table_addr[page_table_i] = page_table_entry_value;
    }
}

int pd_initialize_kernel(page_directory_t *pd){
    USER_MEM_START
    //TODO: replace with not hardcoded number of pte's
    for (i = 0; i < 1024 * 4; i++){
        set_pte(i, i, 0, pd);    
    }
}