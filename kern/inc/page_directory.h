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
#include <mem_section.h>

#define PRESENT_FLAG_BIT 0
#define RW_FLAG_BIT 1
#define MODE_FLAG_BIT 2
#define WRITE_THROUGH_FLAG_BIT 3
#define GLOBAL_FLAG_BIT 8

#define NEW_FLAGS(p,rw,md,glb) ((p << PRESENT_FLAG_BIT) | (rw << RW_FLAG_BIT)\
    | (md << MODE_FLAG_BIT) | (glb << GLOBAL_FLAG_BIT))

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
int pd_map_sections(page_directory_t *pd, mem_section_t *secs,
        uint32_t num_secs);
void *pd_get_base_addr(page_directory_t *pd);
#endif /* _PAGE_DIRECTORY_H_ */


