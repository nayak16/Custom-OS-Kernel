/** @file misc_handlers.c
 *  @brief implements miscellaneous syscall handlers
 *
 *  @author Christopher Wei (cjwei)
 *  @author Aatish Nayak (aatishn)
 *  @bug No known bugs.
 */

#include <simics.h>
#include <kern_internals.h>
#include <common_kern.h>
#include <virtual_mem_mgmt.h>

/** @brief Implements the new_pages system call
 *  @param base The virtual address of the base for the new page
 *  @param len The length of the requested pages
 *  @return 0 on success, negative integer code on failure
 */
int syscall_new_pages_c_handler(void *base, int len){
    /* check for invalid base address */
    if ((uint32_t)base < USER_MEM_START) return -1;
    /* check for non-positive and non-page aligned lengths */
    if (len <= 0 || (len % PAGE_SIZE) != 0) return -1;

    /* Get current running pcb */
    pcb_t *cur_pcb;
    if(scheduler_get_current_pcb(&sched, &cur_pcb) < 0) {
        return -2;
    }

    page_directory_t *pd = &(cur_pcb->pd);

    if (vmm_new_user_page(pd, (uint32_t)base, (uint32_t)(len/PAGE_SIZE)) < 0){
        return -2;
    }
    return 0;
}

/** @brief Implements the remove_pages system call
 *  @param base The virtual address of the base for the page to remove
 *  @return 0 on success, negative integer code on failure
 *  */
int syscall_remove_pages_c_handler(void *base){
    /* Get current running pcb */
    pcb_t *cur_pcb;
    if(scheduler_get_current_pcb(&sched, &cur_pcb) < 0) {
        return -2;
    }
    page_directory_t *pd = &(cur_pcb->pd);
    if (vmm_remove_user_page(pd, (uint32_t)base) < 0) return -1;
    return 0;
}
