/** @file page_directory.c
 *  @brief Implements page directory
 *
 *  Our page directory implements an interface to update and modify mappings.
 *  In addition to storing the actual page tables, we also keep track of the
 *  physical addresses of the frames that have been allocated to a particular
 *  page directory as well as the number of pages that are mapped. These must be
 *  updated along side the page directory whenever a frame is allocated on
 *  behalf of the page directory.
 *
 *  In addition to the usual page directory interface, we have implemented a way
 *  to essentially commit multiple mappings if all of them succeed or none at
 *  all. This allows for ease of error handling when looping through a bunch of
 *  pd_create/remove_mapping calls.
 *
 *  @author Aatish Nayak (aatishn)
 *  @author Christopher Wei (cjwei)
 *  @bug No known bugs.
 */

#include <stdbool.h>

/* USER_MEM_START */
#include <common_kern.h>
/* memalign */
#include <malloc.h>
/* memset */
#include <string.h>
/* flag sets */
#include <constants.h>

#include <page_directory.h>

/* access to flush_tlb */
#include <special_reg_cntrl.h>
#include <simics.h>

/** @brief gets the [n]th bit of [v] */
#define NTH_BIT(v,n) (((uint32_t)v >> n) & 1)
/** @brief number of total page table entries in the kernel space */
#define NUM_KERNEL_PTE (USER_MEM_START >> PAGE_SHIFT)
/** @brief number of page directory entries in the kernel space (number page tables) */
#define NUM_KERNEL_PDE (NUM_KERNEL_PTE / PT_NUM_ENTRIES)

/** @brief number of bits in the offset */
#define OFF_SHIFT PAGE_SHIFT
/** @brief number of bits for the page directory offset */
#define PDE_SHIFT 10
/** @brief number of bits for the page table offset */
#define PTE_SHIFT 10

/** @brief Holds a single mapping task to be completed by the page directory
 *         upon pd_commit_mapping or freed upon pd_abort_mapping */
typedef struct mapping_task{
    /** @brief Virtual address to map */
    uint32_t v_addr;
    /** @brief Page table entry to map */
    uint32_t pte;
    /** @brief A resource to free upon failure */
    void *resource;
} mapping_task_t;

/* @brief Global variable that stores the kernel page directory entries. We can
 * use the same page tables for the kernel in all page directories that are
 * ever created */
uint32_t kernel_pde[NUM_KERNEL_PDE];

/* @brief Whether or not the kernel has been initialized or not */
bool is_kernel_initialized = false;

/* Helper functions */

/** @brief Finds from a pte or pde whether that entry is present
 *  @param v The pte/pte value
 *  @return true if present otherwise false */
bool entry_present(uint32_t v){
    return (NTH_BIT(v,PRESENT_FLAG_BIT) != 0);
}

/** @brief Gets the privilege and access of a virtual address mapping
 *  @param v The virtual address
 *  @param priv Where to store the privilege level (optional)
 *  @param access Where to store the access level (optional)
 *  @return 0
 */
int entry_permissions(uint32_t v, uint32_t *priv, uint32_t *access){
    if (priv != NULL) *priv = NTH_BIT(v,MODE_FLAG_BIT);
    if (access != NULL) *access = NTH_BIT(v,RW_FLAG_BIT);
    return 0;
}

/** @brief Initializes the kernel space directory mappings
 *
 *  Should only be called once
 *
 *  @return 0 on success, negative integer code on failure
 */
int pd_init_kernel(){
    if (is_kernel_initialized){
        panic("pd_init_kernel called twice!");
    }

    /* create a temporary page directory and assign its directory
     * to the global variable kernel_pde */
    page_directory_t pd_temp;
    pd_temp.directory = kernel_pde;
    pd_temp.batch_enabled = false;

    /* present, rw enabled, supervisor mode, dont flush */
    uint32_t pte_flags = NEW_FLAGS(SET,SET,UNSET,SET);
    /* present, rw enabled, supervisor mode */
    uint32_t pde_flags = NEW_FLAGS(SET,SET,UNSET,DONT_CARE);
    uint32_t i;
    /* for the first num_kernel_entries, set the vpn==ppn for direct map */
    /* Leave 0th page unmapped */
    for (i = 1; i < NUM_KERNEL_PTE; i++){
        uint32_t direct_addr = i << PAGE_SHIFT;
        if (pd_create_mapping(&pd_temp, direct_addr, direct_addr,
                    pte_flags, pde_flags) < 0){
            return -1;
        }
    }
    is_kernel_initialized = true;
    return 0;
}

/** @brief Initializes the kernel mappings of a page directory
 *  @param pd The page directory
 *  @return 0 on success -1 on failure
 */
int initialize_kernel(page_directory_t *pd){
    if (pd == NULL) return -1;
    if (!is_kernel_initialized){
        panic("Kernel pages have not been preallocated..\
                Call pd_init_kernel()");
    }
    memcpy(pd->directory, kernel_pde, sizeof(uint32_t)*NUM_KERNEL_PDE);
    return 0;
}

/** @brief Gets the address of a page given its page table and page directory
 *         indexes
 *  @param pd_i The page directory index
 *  @param pt_i The page table index
 *  @return The page's address
 */
void *get_page_address(uint32_t pd_i, uint32_t pt_i){
    return (void *)(pd_i << 22 | pt_i << 12);
}

/** @brief Copies contents stored in a page referenced by a virtual address
 *  in the current page directory into the physical address
 *
 *  Slightly hacky. We use the current page directory to copy the contents
 *  stored at virtual address v_addr into a local buffer. Then we update the
 *  mapping so that it points at the new physical address we want to copy into.
 *  Then we memcpy the local buffer into the new physical address, restore the
 *  old mapping and flush tlb.
 *
 *  @param target_pte Pointer to the page table entry that hold the physical
 *  address to copy from
 *  @param v_addr The virtual address represented by target_pte
 *  @param p_addr The new physical address to copy to
 *  @return 0 on success or negative integer code on failure
 */
int p_copy(void **target_pte, void *v_addr, void *p_addr){
    if (target_pte == NULL || v_addr == NULL)
        return -1;
    /* allocate a new buffer to hold our page contents locally */
    void *buffer = malloc(PAGE_SIZE);
    if (buffer == NULL) return -2;
    /* original page table entry */
    void *original_pte = *target_pte;
    uint32_t flags = EXTRACT_FLAGS(original_pte);
    /* target virtual address */
    memcpy(buffer, v_addr, PAGE_SIZE);
    /* remap our virtual address to new physical address */
    *target_pte = (void *)(ADD_FLAGS(p_addr,flags));
    /* flush cached mappings for our virtual address */
    flush_tlb((uint32_t)v_addr);
    /* copy contents into new phys page */
    memcpy(v_addr, buffer, PAGE_SIZE);
    /* restore mapping */
    *target_pte = original_pte;
    /* flush out any false mappings */
    flush_tlb((uint32_t)v_addr);
    free(buffer);
    return 0;
}

/** @brief Copies a page table from pt_src to pt_dest
 *
 *  @param pt_dest The page table to copy into
 *  @param pt_src The page table to copy from
 *  @param pd_i The page directory index (used to find the virtual address)
 *  @param p_addr_p Pointer to the physical address which we should update as
 *                  we consume physical pages.
 *  @return 0 on success, negative integer code on failure
 */
int pt_copy(uint32_t *pt_dest, uint32_t *pt_src, uint32_t pd_i,
        uint32_t *p_addr_p){
    uint32_t i;
    /* copy each page table entry */
    for (i = 0; i < PT_NUM_ENTRIES; i++){
        uint32_t entry = pt_src[i];
        uint32_t flags = entry & 0xFFF;
        /* copy over present mappings */
        if (entry_present(entry)){
            void *p_addr = (void *)(*p_addr_p);
            *p_addr_p += PAGE_SIZE;
            /* calculate the virtual address of current page being copied so
             * that p_copy can *v_addr to write to the new physical address
             * after remapping*/
            void *v_addr = get_page_address(pd_i, i);
            /* pass in address to the current page table entry so that p_copy
             * can use it to copy into the new physical address */
            if (p_copy((void **)&(pt_src[i]), v_addr, p_addr) < 0)
                return -1;
            /* assign pte to new physical address with flags */
            pt_dest[i] = ADD_FLAGS(p_addr, flags);
        }
    }
    return 0;
}
/* Implementation */

/** @brief Finds the mapping of a virtual address if it exists in a page
 *         directory and optionally returns the address of the entry
 *  @param pd The page directory
 *  @param v_addr The virtual address to look up
 *  @param entry_addr Where the address of the pte is stored upon return
 *  @return 0 on success, negative integer code on failure
 */
int pd_get_mapping(page_directory_t *pd, uint32_t v_addr,
        uint32_t *entry_addr){
    if (pd == NULL) return -1;
    uint32_t pde_i = (v_addr >> (OFF_SHIFT + PTE_SHIFT)) & 0x3FF;
    /* page table index = 2nd 10 bits of v_addr */
    uint32_t pte_i = (v_addr >> OFF_SHIFT) & 0x3FF;
    if (!entry_present(pd->directory[pde_i])){
        /* page directory entry not present */
        return -2;
    }
    uint32_t *pt = (uint32_t *)(REMOVE_FLAGS(pd->directory[pde_i]));
    if (!entry_present(pt[pte_i])){
        /* directory exists but table does not */
        return -3;
    }
    /* found mapping */
    if (entry_addr != NULL){
        *entry_addr = pt[pte_i];
    }
    return 0;
}


/** @brief Checks the permissions of an address and optionally
 *  store the privledge and access
 *  @param pd The page directory
 *  @param v_addr The virtual address in question
 *  @return 0 on success, negative integer code on failure
 */
int pd_get_permissions(page_directory_t *pd, uint32_t v_addr,
        uint32_t *priv, uint32_t *access){
    if (pd == NULL) return -1;
    uint32_t pde_i = (v_addr >> (OFF_SHIFT + PTE_SHIFT)) & 0x3FF;
    /* page table index = 2nd 10 bits of v_addr */
    uint32_t pte_i = (v_addr >> OFF_SHIFT) & 0x3FF;
    if (!entry_present(pd->directory[pde_i])){
        return -2;
    }
    uint32_t *pt = (uint32_t *)(REMOVE_FLAGS(pd->directory[pde_i]));
    if (!entry_present(pt[pte_i])){
        return -3;
    }
    uint32_t pd_priv, pd_access, pt_priv, pt_access,
             combined_priv, combined_access;
    entry_permissions(pd->directory[pde_i], &pd_priv, &pd_access);
    entry_permissions(pt[pte_i], &pt_priv, &pt_access);
    /* combined priv = 1 only if both levels are user */
    combined_priv = pd_priv && pt_priv;
    /* if combined priv = user, both access types must be set
     * otherwise always read/write */
    combined_access = combined_priv ? (pd_access && pt_access) : 1;

    if (priv != NULL) *priv = combined_priv;
    if (access != NULL) *access = combined_access;
    return 0;
}
/** @brief Checks if virtual address is user read/write
 *  @param pd The page directory
 *  @param v_addr The virtual address
 *  @return 1 on true 0 on false
 */
int pd_is_user_read_write(page_directory_t *pd, uint32_t v_addr){
    uint32_t priv, access;
    pd_get_permissions(pd, v_addr, &priv, &access);
    return (priv == 1 && access == 1);
}

/** @brief Checks if virtual address is in user space
 *  @param pd The page directory
 *  @param v_addr The virtual address
 *  @return 1 on true 0 on false
 */
int pd_is_user_readable(page_directory_t *pd, uint32_t v_addr){
    uint32_t priv;
    pd_get_permissions(pd, v_addr, &priv, NULL);
    return (priv == 1);
}

/** @brief Begins a batch mapping
 *  @param pd The page directory
 *  @return 0 on success, negative integer code on failure
 */
int pd_begin_mapping(page_directory_t *pd){
    if (pd == NULL || pd->mapping_tasks == NULL) return -1;
    if (pd->batch_enabled) return -2;
    pd->batch_enabled = true;
    return 0;
}
/** @brief Aborts a batch mapping
 *  @param pd The page directory
 *  @return Void
 */
void pd_abort_mapping(page_directory_t *pd){
    mapping_task_t *task;
    while (ll_size(pd->mapping_tasks) > 0){
        ll_remove_first(pd->mapping_tasks, (void **)&task);
        if (task->resource != NULL) free(task->resource);
        free(task);
    }
    pd->batch_enabled = false;
}
/** @brief Commits a batch mapping
 *
 *  Requires that no mapping in the page directory's mapping_tasks will fail
 *
 *  @param pd The page directory
 *  @return Void
 */
void pd_commit_mapping(page_directory_t *pd){
    mapping_task_t *task;
    while (ll_size(pd->mapping_tasks) > 0){
        ll_remove_first(pd->mapping_tasks, (void **)&task);
        uint32_t pde_i = (task->v_addr >> (OFF_SHIFT + PTE_SHIFT)) & 0x3FF;
        uint32_t pte_i = (task->v_addr >> OFF_SHIFT) & 0x3FF;
        uint32_t pde_value = pd->directory[pde_i];
        uint32_t *page_table = (uint32_t *)REMOVE_FLAGS(pde_value);
        page_table[pte_i] = task->pte;
        free(task);
    }
    pd->batch_enabled = false;
}


/** @brief Creates a mapping in a page directory with given flags
 *
 *  Requires that both v_addr and p_addr are page aligned
 *
 *  @param pd The page directory
 *  @param v_addr The virtual address to map
 *  @param p_addr The physical frame to map to
 *  @param pte_flags The page table entry flags
 *  @param pde_flags The page directory entry flags (if new page table is made)
 *  @return 0 on success, negative integer code on failure
 */
int pd_create_mapping(page_directory_t *pd, uint32_t v_addr, uint32_t p_addr,
        uint32_t pte_flags, uint32_t pde_flags){
    /* input addresses must be page aligned */
    if (!IS_PAGE_ALIGNED(v_addr) || !IS_PAGE_ALIGNED(p_addr)) return -1;
    uint32_t pde_i, pte_i, pde_value, pte_value;

    /* page directory entry index = top 10 bits of v_addr */
    pde_i = (v_addr >> (OFF_SHIFT + PTE_SHIFT)) & 0x3FF;
    /* page table index = 2nd 10 bits of v_addr */
    pte_i = (v_addr >> OFF_SHIFT) & 0x3FF;
    /* page table entry value is the physical address with flags */
    pte_value = ADD_FLAGS(p_addr, pte_flags);

    /* create a table if needed */

    void *resource = NULL;

    if (!entry_present(pd->directory[pde_i])){
        if ((pde_value = (uint32_t)memalign(PAGE_SIZE, PT_SIZE)) == 0)
            return -2;
        resource = (void *)(pde_value);
        memset((void *)pde_value, 0, PAGE_SIZE);
        pde_value = ADD_FLAGS(pde_value, pde_flags);
        pd->directory[pde_i] = pde_value;
    } else {
        pde_value = pd->directory[pde_i];
    }
    /* get address of page table and save mapping */
    uint32_t *page_table = (uint32_t *)REMOVE_FLAGS(pde_value);

    /* check if we should create mapping right away or add to mapping tasks */
    if (pd->batch_enabled){
        mapping_task_t *task = malloc(sizeof(mapping_task_t));
        if (task == NULL) return -3;
        task->v_addr = v_addr;
        task->pte = pte_value;
        task->resource = resource;
        if (ll_add_last(pd->mapping_tasks, task) < 0){
            free(task);
            free(resource);
            return -4;
        }
    } else {
        page_table[pte_i] = pte_value;
    }

    return 0;
}

/** @brief Removes mapping from page directory
 *
 *  Requires that both v_addr and p_addr are page aligned
 *
 *  @param pd The page directory
 *  @param v_addr The virtual address to remove
 *  @return 0 on success, negative integer code on failure
 */
int pd_remove_mapping(page_directory_t *pd, uint32_t v_addr){
    if (!IS_PAGE_ALIGNED(v_addr) || pd == NULL) return -1;
    uint32_t pde_i, pte_i;
    /* page directory entry index = top 10 bits of v_addr */
    pde_i = (v_addr >> (OFF_SHIFT + PTE_SHIFT)) & 0x3FF;
    /* page table index = 2nd 10 bits of v_addr */
    pte_i = (v_addr >> OFF_SHIFT) & 0x3FF;

    /* check for present page directory entry */
    if (!entry_present(pd->directory[pde_i]))
        return -2;
    uint32_t *page_table = (uint32_t *)REMOVE_FLAGS(pd->directory[pde_i]);
    if (!entry_present(page_table[pte_i]))
        return -3;
    /* clear the mapping */
    if (pd->batch_enabled){
        mapping_task_t *task = malloc(sizeof(mapping_task_t));
        if (task == NULL) return -3;
        task->v_addr = v_addr;
        task->pte = 0;
        task->resource = NULL;
        if (ll_add_last(pd->mapping_tasks, task) < 0){
            free(task);
            return -4;
        }
    } else {
        page_table[pte_i] = 0;
    }
    return 0;
}


/** @brief Returns the directory in a page directory struct
 *  @param pd The page directory
 *  @return The directory
 */
void *pd_get_base_addr(page_directory_t *pd){
    return (void *)(pd->directory);
}


/** @brief Initializes a page directory
 *  @param The page directory
 *  @return 0 on success, -1 on failure
 */
int pd_init(page_directory_t *pd){
    /* page align allocation and clear out all present bits */
    pd->directory = memalign(PAGE_SIZE, PD_SIZE);
    if (pd->directory == NULL){
        return -1;
    }
    memset(pd->directory, 0, PD_SIZE);
    if (initialize_kernel(pd) < 0){
        free(pd->directory);
        pd->directory = NULL;
        return -2;
    }
    pd->num_pages = 0;
    pd->batch_enabled = false;
    pd->mapping_tasks = malloc(sizeof(ll_t));
    if (pd->mapping_tasks == NULL) return -3;
    pd->p_addr_list = malloc(sizeof(ll_t));
    if (pd->p_addr_list == NULL){
        free(pd->mapping_tasks);
        return -4;
    }
    if (ll_init(pd->p_addr_list) < 0){
        free(pd->mapping_tasks);
        free(pd->p_addr_list);
        return -5;
    }
    if (ll_init(pd->mapping_tasks) < 0){
        ll_destroy(pd->p_addr_list);
        free(pd->mapping_tasks);
        free(pd->p_addr_list);
        return -6;
    }

    return 0;
}


/** @brief Shallow copies a page directory
 *
 *  Both pd_dest and pd_src are expected to be pd_init'ed. Only copies
 *  page directory entries that are present and are not in the kernel.
 *  After execution, the same page directory entry indexes of pd_dest
 *  and pd_src will be filled but  pd_dest will point to different (and empty)
 *  page tables then that of pd_src.
 *
 *  @param pd_dest The page directory to copy to
 *  @param pd_src The page directory copying from
 *  @return 0 On success -1 on failure
 */
int pd_deep_copy(page_directory_t *pd_dest, page_directory_t *pd_src,
        uint32_t p_addr_start){
    if (pd_dest == NULL || pd_src == NULL) return -1;

    /* copy the upper level page directory */
    uint32_t i, j;

    uint32_t p_addr = p_addr_start;

    uint32_t backup_directory[PD_NUM_ENTRIES];

    /* copy over non-kernel space */
    for (i = NUM_KERNEL_PDE; i < PD_NUM_ENTRIES; i++){
        /* for each present entry, create a new page table  */
        uint32_t entry = pd_src->directory[i];
        if (entry_present(entry)){
            /* allocate a new page table */
            uint32_t *new_pt = memalign(PAGE_SIZE, PT_SIZE);
            if (new_pt == NULL){
                /* roll back changes and release resources */
                for (j = i-1; j >= NUM_KERNEL_PDE; j--){
                    if (entry_present(pd_dest->directory[j])){
                        free((void *)(REMOVE_FLAGS(pd_dest->directory[j])));
                    }
                }
                memcpy(&pd_dest->directory[NUM_KERNEL_PDE],
                        &backup_directory[NUM_KERNEL_PDE],
                        (i-NUM_KERNEL_PDE) * sizeof(uint32_t));
                return -2;
            }
            memset((void *)new_pt, 0, PAGE_SIZE);
            uint32_t flags = EXTRACT_FLAGS(entry);
            /* save the old mapping in case of rollbacks */
            backup_directory[i] = pd_dest->directory[i];
            /* map page directory to new page table */
            pd_dest->directory[i] = (uint32_t)new_pt | flags;
            pt_copy(new_pt, (uint32_t *)REMOVE_FLAGS(entry), i, &p_addr);
        }
    }
    /* add new physical address space to our new pd's address list */

    return 0;
}

/** @brief Defines metadata for a frame that should be remembered in a page
 * directory */
typedef struct pd_frame_metadata{
    /** @brief The base address */
    uint32_t p_addr;
    /** @breif Size of frame */
    uint32_t num_pages;
} pd_frame_metadata_t;

/** @brief Returns the frame base address of a metadata */
void *pd_frame_metadata_addr(void *metadata){
    return (void *)((pd_frame_metadata_t *)metadata)->p_addr;
}

/** @brief Gives a page directory a frame to remember
 *  @param pd The page directory
 *  @param p_addr The physical address
 *  @param num_pages The number of pages of the frame
 *  @return 0 On success -1 on failure
 */
int pd_alloc_frame(page_directory_t *pd, uint32_t p_addr, uint32_t num_pages){
    if (pd == NULL) return -1;
    pd->num_pages += num_pages;
    /* add first for better locality */
    pd_frame_metadata_t *metadata = malloc(sizeof(pd_frame_metadata_t));
    if (metadata == NULL) return -2;
    metadata->p_addr = p_addr;
    metadata->num_pages = num_pages;
    if (ll_add_first(pd->p_addr_list, (void *)metadata) < 0){
        free(metadata);
        return -3;
    }
    return 0;
}

/** @brief Removes frame metadata from a page directory with a given address
 *  @param pd The page directory
 *  @param p_addr Base address of the physical frame to be removed
 *  @param frame_size Optional address to store the frame size that was
 *  dealloacated
 *  @return 0 On success -1 on failure
 */
int pd_dealloc_frame(page_directory_t *pd, uint32_t p_addr,
        uint32_t *frame_size){
    if (pd == NULL) return -1;
    pd_frame_metadata_t *metadata;
    if (ll_remove(pd->p_addr_list, &pd_frame_metadata_addr,
            (void *)p_addr, (void **)&metadata, NULL) < 0)
        return -2;
    pd->num_pages -= metadata->num_pages;
    if (frame_size != NULL) *frame_size = metadata->num_pages;
    free(metadata);
    return 0;
}

/** @brief Returns number of frames allocated to a page directory
 *  @param pd The page directory
 *  @return Number of frames in the page directory, or -1 on failure
 */
int pd_num_frames(page_directory_t *pd){
    if (pd == NULL) return -1;
    return ll_size(pd->p_addr_list);
}


/** @brief Removes all frame metadata from a page directory and stores the
 *         starting addresses of each frame in an array
 *  @param pd The page directory
 *  @param addr_list The array to store to
 *  @return 0 on success, negative integer code on failure
 */
int pd_dealloc_all_frames(page_directory_t *pd, uint32_t *addr_list, uint32_t *size_list){
    if (pd == NULL || addr_list == NULL) return -1;
    int arr_i = 0;
    while (pd_num_frames(pd) > 0){
        pd_frame_metadata_t *metadata;
        ll_remove_first(pd->p_addr_list, (void **)&metadata);
        addr_list[arr_i] = metadata->p_addr;
        if (size_list != NULL)
            size_list[arr_i] = metadata->num_pages;
        arr_i++;
    }
    pd->num_pages = 0;
    return 0;
}


/** @brief Removes all mappings from the user space
 *  @param pd The page directory
 *  @return 0 on success, negative integer code on failure
 */
int pd_clear_user_space(page_directory_t *pd){
    int i;
    for (i = NUM_KERNEL_PDE; i < PD_NUM_ENTRIES; i++) {
        uint32_t entry = pd->directory[i];
        if (entry_present(entry)){
            pd->directory[i] = 0;
            /* Free each page table */
            uint32_t pt = REMOVE_FLAGS(entry);
            if (pt == 0) MAGIC_BREAK;
            free((void*) pt);
        }
    }
    return 0;
}


/** @brief Destroys a page directory
 *
 *  Requires that the list of metadata is zero
 *
 *  @param pd The page directory to destroy
 *  @return Void
 */
void pd_destroy(page_directory_t *pd) {
    int i;
    /* At this point we should have already deallocated addresses stored
     * in p_addr_list so it should be safe to destroy */
    if (ll_size(pd->p_addr_list) > 0){
        panic("Destroying page directory before returning all frames!");
    }
    ll_destroy(pd->p_addr_list);
    /* destroy all non-kernel page tables */
    for (i = NUM_KERNEL_PDE ; i < PD_NUM_ENTRIES ; i++) {
        uint32_t entry = pd->directory[i];
        if (entry_present(entry)){
            /* Free each page table */
            uint32_t pt = REMOVE_FLAGS(entry);
            free((void*) pt);
        }
    }
    /* Free whole directory */
    free(pd->directory);
}
