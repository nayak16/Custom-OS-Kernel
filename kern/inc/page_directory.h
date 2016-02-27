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

#define PRESENT_FLAG_BIT 0
#define RW_FLAG_BIT 1
#define MODE_FLAG_BIT 2
#define WRITE_THROUGH_FLAG_BIT 3
#define GLOBAL_FLAG_BIT 8

#define NUM_ENTRIES (PAGE_SIZE/sizeof(void *))

typedef struct page_directory {
    int directory[NUM_ENTRIES];
} page_directory_t;

int pd_init(page_directory_t *pd);

int pd_initialize_kernel();

#endif /* _PAGE_DIRECTORY_H_ */


