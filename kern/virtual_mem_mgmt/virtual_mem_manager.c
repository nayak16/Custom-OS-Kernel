/** @file virtual_mem_manager.c
 *  @brief Implements virtual memory manipulation functions
 *
 *  @author Aatish Nayak (aatishn)
 *  @author Christopher Wei (cjwei)
 *  @bug No known bugs.
 */


/* memalign */
#include <malloc.h>
/* memset */
#include <string.h>

/* Page constants */
#include <x86/page.h>

#define NTH_BIT(v,n) (((uint32_t)v >> n) & 1)

#define NUM_ENTRIES (PAGE_SIZE/sizeof(uint32_t))


int create_mapping(uint32_t vpn, uint32_t ppn, uint32_t pte_flags,
                   uint32_t pde_flags, page_directory_t *pd){
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


