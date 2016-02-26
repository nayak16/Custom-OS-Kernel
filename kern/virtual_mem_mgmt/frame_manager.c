/** @file frame_manager.c
 *  @brief implementation for a vm frame manager
 *
 *  @author Aatish Nayak (aatishn)
 *  @author Christopher Wei (cjwei)
 *  @bug No known bugs.
 */

/* USER_MEM_START */
#include <common_kern.h>
/* PAGE_SIZE */
#include <syscall.h>
#include <stdbool.h>
/* malloc, NULL define */
#include <stdlib.h>

#include <frame_manager.h>

/** @brief Initializes a frame */
int frame_init(frame_t *f, int i){
    if (f == NULL) return -1;
    void *addr = (void *)(i * PAGE_SIZE);
    f->addr_low = addr;
    f->allocated = false;
    if ((int)addr < (int)USER_MEM_START){
        f->type = user_frame;
    } else {
        f->type = kernel_frame;
    }
    return 0;
}

/** @brief Initializes a frame manager
 */
int frame_manager_init(frame_manager_t *fm){
    if (fm == NULL) return -1;
    int n, i;
    ll_init(&(fm->free_user_frames));
    ll_init(&(fm->free_kernel_frames));
    n = machine_phys_frames();
    for (i = 0; i < n; i++){
        void *addr = i * PAGE_SIZE;
        if ((int)addr < (int)USER_MEM_START){
            /* kernel frame */
            ll_enq(fm->free_kernel_frames, addr);
        } else {
            /* user frame */
            ll_enq(fm->free_user_frames, addr);
        }
    }
    return 0;
}

void frame_manager_destroy(frame_manager_t *fm){
    return;
}