/** @file page_directory.h
 *  @brief Specifies interface for page directory
 *
 *  @author Aatish Nayak (aatishn)
 *  @author Christopher Wei (cjwei)
 *  @bug No known bugs.
 */

#ifndef _PAGE_DIRECTORY_H_
#define _PAGE_DIRECTORY_H_

/* PAGE SIZE */
#include <x86/page.h>
#include <stdint.h>

#define PRESENT_FLAG_BIT 0
#define RW_FLAG_BIT 1
#define MODE_FLAG_BIT 2
#define WRITE_THROUGH_FLAG_BIT 3
#define GLOBAL_FLAG_BIT 8

#define PD_SIZE PAGE_SIZE
#define PD_NUM_ENTRIES (PD_SIZE / sizeof(uint32_t))

#define PT_SIZE PAGE_SIZE
#define PT_NUM_ENTRIES (PT_SIZE / sizeof(uint32_t))

typedef struct page_directory {
    uint32_t *directory;
} page_directory_t;

int pd_init(page_directory_t *pd);
int pd_entry_present(uint32_t v);
int pd_copy(page_directory_t *pd_dest, page_directory_t *pd_src);
void *pd_get_base_addr(page_directory_t *pd);
#endif /* _PAGE_DIRECTORY_H_ */


