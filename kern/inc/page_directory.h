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
/* Constants */
#include <constants.h>
#include <stdint.h>
#include <mem_section.h>
#include <frame_manager.h>

#include <queue.h>

#define PRESENT_FLAG_BIT 0
#define RW_FLAG_BIT 1
#define MODE_FLAG_BIT 2
#define WRITE_THROUGH_FLAG_BIT 3
#define GLOBAL_FLAG_BIT 8

/* custom non-x86 flag bits */
#define USER_START_FLAG_BIT 9
#define USER_END_FLAG_BIT 10

/* p - SET implies page is present, UNSET implies page is unpresent
 * rw - SET implies page is read writable, UNSET implies read only
 * md - SET implies user, UNSET implies supervisor
 * glb - SET implies global, UNSET implies local
 */
#define NEW_FLAGS(p,rw,md,glb) ((p << PRESENT_FLAG_BIT) | (rw << RW_FLAG_BIT)\
    | (md << MODE_FLAG_BIT) | (glb << GLOBAL_FLAG_BIT))

#define ADD_USER_START_FLAG(flags) (flags | (SET << USER_START_FLAG_BIT))
#define ADD_USER_END_FLAG(flags) (flags | (SET << USER_END_FLAG_BIT))

#define IS_USER_START(pte) ((pte >> USER_START_FLAG_BIT) & 1)
#define IS_USER_END(pte) ((pte >> USER_END_FLAG_BIT) & 1)

#define USER_RO NEW_FLAGS(SET, UNSET, SET, UNSET)
#define USER_WR NEW_FLAGS(SET, SET, SET, UNSET)


/* User RO */
#define PDE_FLAG_DEFAULT (NEW_FLAGS(SET, UNSET, SET, DONT_CARE))
/* User RO */
#define PTE_FLAG_DEFAULT (NEW_FLAGS(SET, UNSET, SET, UNSET))


#define PD_SIZE PAGE_SIZE
#define PD_NUM_ENTRIES (PD_SIZE / sizeof(uint32_t))

#define PT_SIZE PAGE_SIZE
#define PT_NUM_ENTRIES (PT_SIZE / sizeof(uint32_t))


#define IS_PAGE_ALIGNED(a) (a % PAGE_SIZE == 0)

#define DIV_ROUND_UP(num, den) ((num + den -1) / den)
#define PAGE_ALIGN_UP(addr) (PAGE_SIZE * DIV_ROUND_UP(addr, PAGE_SIZE))
#define PAGE_ALIGN_DOWN(addr) (PAGE_SIZE * (addr / PAGE_SIZE))

#define ADD_FLAGS(v,f) ((uint32_t)v | f)
#define REMOVE_FLAGS(v) ((uint32_t)v & ~0xFFF)
#define EXTRACT_FLAGS(v) ((uint32_t)v & 0xFFF)

typedef struct page_directory {
    uint32_t *directory;
    uint32_t num_pages;
    ll_t *p_addr_list;
} page_directory_t;

int pd_init(page_directory_t *pd);
int pd_get_mapping(page_directory_t *pd, uint32_t v_addr, uint32_t *pte);
int pd_create_mapping(page_directory_t *pd, uint32_t v_addr, uint32_t p_addr, uint32_t pte_flags, uint32_t pde_flags);
int pd_get_permissions(page_directory_t *pd, uint32_t v_addr,
        uint32_t *priv, uint32_t *access);
int pd_is_user_read_write(page_directory_t *pd, uint32_t v_addr);
int pd_is_user_readable(page_directory_t *pd, uint32_t v_addr);
int pd_remove_mapping(page_directory_t *pd, uint32_t v_addr);
int pd_entry_present(uint32_t v);
int pd_deep_copy(page_directory_t *pd_dest, page_directory_t *pd_src, uint32_t p_addr_start);
int pd_map_sections(page_directory_t *pd, mem_section_t *secs,
        uint32_t num_secs);
void *pd_get_base_addr(page_directory_t *pd);

int pd_alloc_frame(page_directory_t *pd, uint32_t p_addr, uint32_t num_pages);
int pd_dealloc_frame(page_directory_t *pd, uint32_t p_addr);
int pd_dealloc_all_frames(page_directory_t *pd, uint32_t *addr_list);
int pd_num_frames(page_directory_t *pd);

void pd_destroy(page_directory_t *pd);

#endif /* _PAGE_DIRECTORY_H_ */


