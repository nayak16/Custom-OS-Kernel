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
/* flag sets */
#include <constants.h>

#include <page_directory.h>
#include <kern_internals.h>

/* access to flush_tlb */
#include <special_reg_cntrl.h>


#include <simics.h>

#define NTH_BIT(v,n) (((uint32_t)v >> n) & 1)
/* number of total page table entries in the kernel space */
#define NUM_KERNEL_PTE (USER_MEM_START >> PAGE_SHIFT)
/* number of page directory entries in the kernel space (number page tables) */
#define NUM_KERNEL_PDE (NUM_KERNEL_PTE / PT_NUM_ENTRIES)

#define OFF_SHIFT PAGE_SHIFT
#define PDE_SHIFT 10
#define PTE_SHIFT 10

#define NEW_FLAGS(p,rw,md,glb) ((p << PRESENT_FLAG_BIT) | (rw << RW_FLAG_BIT)\
    | (md << MODE_FLAG_BIT) | (glb << GLOBAL_FLAG_BIT))

#define ADD_FLAGS(v,f) ((uint32_t)v | f)
#define REMOVE_FLAGS(v) ((uint32_t)v & ~0xFFF)
#define EXTRACT_FLAGS(v) ((uint32_t)v & 0xFFF)

#define IS_PAGE_ALIGNED(a) (a % PAGE_SIZE == 0)

#define ENTRY_PRESENT 0
#define ENTRY_NOT_PRESENT 1

#define DIV_ROUND_UP(num, den) ((num + den -1) / den)
#define PAGE_ALIGN_UP(addr) (PAGE_SIZE * DIV_ROUND_UP(addr, PAGE_SIZE))
#define PAGE_ALIGN_DOWN(addr) (PAGE_SIZE * (addr / PAGE_SIZE))

int entry_present(uint32_t v){
    if (NTH_BIT(v,0) == 0) return ENTRY_NOT_PRESENT;
    return ENTRY_PRESENT;
}

int pd_create_mapping(page_directory_t *pd, int32_t v_addr, uint32_t p_addr, uint32_t pte_flags,
        uint32_t pde_flags){
    /* input addresses must be page aligned */
    if (!IS_PAGE_ALIGNED(v_addr) || !IS_PAGE_ALIGNED(p_addr)) return -1;
    uint32_t pde_i, pte_i, pde_value, pte_value;

    /* page directory entry index = top 10 bits of v_addr */
    pde_i = (v_addr >> (OFF_SHIFT + PTE_SHIFT)) & 0x3FF;
    /* page table index = 2nd 10 bits of v_addr */
    pte_i = (v_addr >> OFF_SHIFT) & 0x3FF;
    /* page table entry value is the physical address with flags */
    pte_value = ADD_FLAGS(p_addr, pde_flags);

    /* create a table if needed */
    if (entry_present(pd->directory[pde_i]) != ENTRY_PRESENT){
        if ((pde_value = (uint32_t)memalign(PAGE_SIZE, PT_SIZE)) == 0)
            return -2;
        pde_value = ADD_FLAGS(pde_value, pde_flags);
        pd->directory[pde_i] = pde_value;
    } else {
        pde_value = pd->directory[pde_i];
    }

    /* get address of page table and save mapping */
    uint32_t *page_table = (uint32_t *)REMOVE_FLAGS(pde_value);
    page_table[pte_i] = pte_value;

    return 0;
}


int is_sufficient_memory(mem_section_t *secs, uint32_t num_secs) {
    int total_pages = 0;
    int s;
    for (s = 0 ; s < num_secs ; s++) {
        int n_pages = DIV_ROUND_UP(secs[s].len, PAGE_SIZE);
        total_pages += n_pages;
    }
    return total_pages <= fm_num_free_frames(&fm);
}


int pd_map_sections(page_directory_t *pd, mem_section_t *secs,
        uint32_t num_secs) {

    if(!is_sufficient_memory(secs, num_secs)) return -1;

    int s;
    /* Loop through all memory sections to allocate */
    for (s = 0 ; s < num_secs ; s++) {
        mem_section_t ms = secs[s];

        uint32_t len = ms.len;
        /* cur_addr must be page aligned */
        uint32_t cur_addr = ms.v_addr_start;
        /* Allocate and map each page */
        while(len > 0) {
            //TODO: handle for errors (revert)
            void *p_addr;
            if (fm_alloc(&fm, &p_addr) < 0) //Should never happen
                return -2;

            if (pd_create_mapping(pd, PAGE_ALIGN_DOWN(cur_addr), (uint32_t)p_addr,
                        ms.pte_f, ms.pde_f) < 0) return -1;

            /* note: len is unsigned so we must check for underflow */
            if (len < PAGE_SIZE) {
                break;
            }
            cur_addr += PAGE_SIZE;
            len -= PAGE_SIZE;
        }
    }
    return 0;

}

int initialize_kernel(page_directory_t *pd){
    /* present, rw enabled, supervisor mode, dont flush */
    uint32_t pte_flags = NEW_FLAGS(SET,SET,UNSET,SET);
    /* present, rw enabled, supervisor mode */
    uint32_t pde_flags = NEW_FLAGS(SET,SET,UNSET,DONT_CARE);
    uint32_t i;
    /* for the first num_kernel_entries, set the vpn==ppn for direct map */
    /* Leave 0th page unmapped */
    for (i = 1; i < NUM_KERNEL_PTE; i++){
        uint32_t direct_addr = i << PAGE_SHIFT;
        pd_create_mapping(pd, direct_addr, direct_addr, pte_flags, pde_flags);
    }
    return 0;
}


void *pd_get_base_addr(page_directory_t *pd){
    return (void *)(pd->directory);
}

int pd_init(page_directory_t *pd){
    /* page align allocation and clear out all present bits */
    pd->directory = memalign(PAGE_SIZE, PD_SIZE);
    if (pd->directory == NULL) return -1;
    memset(pd->directory, 0, PD_SIZE);
    initialize_kernel(pd);
    return 0;
}

void *get_page_address(uint32_t pd_i, uint32_t pt_i){
    return (void *)(pd_i << 22 | pt_i << 12);
}

int p_copy(void **target_pte, void *v_addr, void **new_phys_addr){
    if (target_pte == NULL || v_addr == NULL || new_phys_addr == NULL)
        return -1;
    /* allocate a new buffer to hold our page contents locally */
    void *buffer = malloc(PAGE_SIZE);
    if (buffer == NULL) return -2;
    /* original page table entry */
    void *original_pte = *target_pte;
    uint32_t flags = EXTRACT_FLAGS(original_pte);
    /* target virtual address */
    memcpy(buffer, v_addr, PAGE_SIZE);
    if (fm_alloc(&fm, new_phys_addr) < 0) return -3;
    /* remap our virtual address to new physical address */
    *target_pte = (void *)(ADD_FLAGS(*new_phys_addr,flags));
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

/** @brief Deep copy page table from src to dest */
int pt_copy(uint32_t *pt_dest, uint32_t *pt_src, uint32_t pd_i){
    uint32_t i;
    /* copy each page table entry */
    for (i = 0; i < PT_NUM_ENTRIES; i++){
        uint32_t entry = pt_src[i];
        uint32_t flags = entry & 0xFFF;
        /* copy over present mappings */
        if (entry_present(entry) == ENTRY_PRESENT){
            void *p_addr;
            /* calculate the virtual address of current page being copied so
             * that p_copy can *v_addr to write to the new physical address
             * after remapping*/
            void *v_addr = get_page_address(pd_i, i);
            /* pass in address to the current page table entry so that p_copy
             * can use it to copy into the new physical address */
            if (p_copy((void **)&(pt_src[i]), v_addr ,&p_addr) < 0)
                return -1;
            /* assign pte to new physical address with flags */
            pt_dest[i] = ADD_FLAGS(p_addr, flags);
        }
    }
    return 0;
}

int pd_copy(page_directory_t *pd_dest, page_directory_t *pd_src){
    if (pd_dest == NULL || pd_src == NULL) return -1;
    /* copy the upper level page directory */
    uint32_t i;
    /* copy over non-kernel space */
    for (i = NUM_KERNEL_PDE; i < PD_NUM_ENTRIES; i++){
        /* for each present entry, create a new page table  */
        uint32_t entry = pd_src->directory[i];
        if (entry_present(entry) == ENTRY_PRESENT){
            /* allocate a new page table */
            uint32_t *new_pt = memalign(PAGE_SIZE, PT_SIZE);
            uint32_t flags = entry & 0xFFF;
            /* map page directory to new page table */
            pd_dest->directory[i] = (uint32_t)new_pt | flags;
            /* copy pages, pass in page directory index for
             * to compute virtual address of an entry */
            pt_copy(new_pt, (uint32_t*)(entry & ~0xFFF), i);
        }
    }
    return 0;
}


