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
#include <stdbool.h>
#include <queue.h>

/** @brief the bit flag representing the present flag */
#define PRESENT_FLAG_BIT 0
/** @brief the bit flag representing the read write flag */
#define RW_FLAG_BIT 1
/** @brief the bit flag representing the mode flag */
#define MODE_FLAG_BIT 2
/** @brief the write through flag bit */
#define WRITE_THROUGH_FLAG_BIT 3
/** @brief the global flag bit */
#define GLOBAL_FLAG_BIT 8
/** @brief the cached disabled bit */
#define CACHE_DISABLED_BIT 4

/* custom non-x86 flag bits */

/** @brief defines the flag bit that we use to denote the beginning of a user
 * allocated memory space */
#define USER_START_FLAG_BIT 9

/** @brief defines the flag bit that we use to denote the end of a user
 * allocated memory space */
#define USER_END_FLAG_BIT 10

/* p - SET implies page is present, UNSET implies page is unpresent
 * rw - SET implies page is read writable, UNSET implies read only
 * md - SET implies user, UNSET implies supervisor
 * glb - SET implies global, UNSET implies local
 * chc - SET implies cache disabled, UNSET implies cache enabled
 */
/** @brief creates the flags with the given parameters */
#define NEW_FLAGS(p,rw,md,glb,ch) ((p << PRESENT_FLAG_BIT) | (rw << RW_FLAG_BIT)\
    | (md << MODE_FLAG_BIT) | (glb << GLOBAL_FLAG_BIT) | (ch << CACHE_DISABLED_BIT))

/** @brief adds the user_start flag to a set of flags */
#define ADD_USER_START_FLAG(flags) (flags | (SET << USER_START_FLAG_BIT))
/** @brief adds the user_end flag to a set of flags */
#define ADD_USER_END_FLAG(flags) (flags | (SET << USER_END_FLAG_BIT))

/** @brief checks if a page table entry is user start */
#define IS_USER_START(pte) ((pte >> USER_START_FLAG_BIT) & 1)
/** @brief checks if a page table entry is user end */
#define IS_USER_END(pte) ((pte >> USER_END_FLAG_BIT) & 1)

/** @brief defines a user read only flags */
#define USER_RO NEW_FLAGS(SET, UNSET, SET, UNSET, UNSET)
/** @brief defines a user read write flags */
#define USER_WR NEW_FLAGS(SET, SET, SET, UNSET, UNSET)


/** @brief defines the default page directory entry flags */
#define PDE_FLAG_DEFAULT (NEW_FLAGS(SET, UNSET, SET, DONT_CARE, UNSET))
/* @brief defines the default page table entry flags */
#define PTE_FLAG_DEFAULT (NEW_FLAGS(SET, UNSET, SET, UNSET, UNSET))

/** @brief defines the size of a page directory */
#define PD_SIZE PAGE_SIZE
/** @brief defines the number of entries in a page directory */
#define PD_NUM_ENTRIES (PD_SIZE / sizeof(uint32_t))

/** @brief defines the size of a page table */
#define PT_SIZE PAGE_SIZE
/** @brief defines the number of entries in a page table */
#define PT_NUM_ENTRIES (PT_SIZE / sizeof(uint32_t))

/** @brief checks whether an address is page aligned */
#define IS_PAGE_ALIGNED(a) (a % PAGE_SIZE == 0)

/** @brief divides and rounds up */
#define DIV_ROUND_UP(num, den) ((num + den -1) / den)
/** @brief page aligns an address upwards */
#define PAGE_ALIGN_UP(addr) (PAGE_SIZE * DIV_ROUND_UP(addr, PAGE_SIZE))
/** @brief page aligns an address downwards */
#define PAGE_ALIGN_DOWN(addr) (PAGE_SIZE * (addr / PAGE_SIZE))

/** @brief adds flags to an entry */
#define ADD_FLAGS(v,f) ((uint32_t)v | f)
/** @brief remove flags from an entry */
#define REMOVE_FLAGS(v) ((uint32_t)v & ~0xFFF)
/** @brief gets the flags from an entry */
#define EXTRACT_FLAGS(v) ((uint32_t)v & 0xFFF)

/** @brief defines user privilege flag */
#define PRIV_USER 1
/** @brief defines kernel privilege flag */
#define PRIV_KERNEL 0
/** @brief defines read only flag */
#define ACC_RO 0
/** @brief defines access for read write */
#define ACC_RW 1

/** @brief defines a page directory struct */
typedef struct page_directory {
    /** @brief the internal directory */
    uint32_t *directory;
    /** @brief the number of pages in the directory */
    uint32_t num_pages;
    /** @brief the list of physical addresses given to the directory */
    ll_t *p_addr_list;
    /** @brief the list of mapping tasks that are to be committed or aborted */
    ll_t *mapping_tasks;
    /** @brief denotes whether or not we should be mapping tasks immediately or
     * during commits only */
    bool batch_enabled;
} page_directory_t;

int pd_init(page_directory_t *pd, int core_num);
int pd_init_kernel(int num_cores);
int pd_get_mapping(page_directory_t *pd, uint32_t v_addr, uint32_t *pte);

int pd_begin_mapping(page_directory_t *pd);
void pd_abort_mapping(page_directory_t *pd);
void pd_commit_mapping(page_directory_t *pd);


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
int pd_dealloc_frame(page_directory_t *pd, uint32_t p_addr,
        uint32_t *frame_size);
int pd_dealloc_all_frames(page_directory_t *pd, uint32_t *addr_list,
        uint32_t *size_list);
int pd_num_frames(page_directory_t *pd);
int pd_clear_user_space(page_directory_t *pd);
void pd_destroy(page_directory_t *pd);

#endif /* _PAGE_DIRECTORY_H_ */


