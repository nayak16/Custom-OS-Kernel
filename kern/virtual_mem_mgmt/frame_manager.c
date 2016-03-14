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
#include <page.h>
#include <stdbool.h>
/* malloc, NULL define */
#include <stdlib.h>
/* uint32_t */
#include <stdint.h>

#include <frame_manager.h>

/** @brief Allocates a frame
 *  @param fm The frame manager
 *  @param addr Result pointer to be stored to if needed
 *  @return 0 on success, 1 on failure */
int fm_alloc(frame_manager_t *fm, void **addr){
    return queue_deq(&(fm->free_frames), addr);
}

int fm_nxt_free_frame(frame_manager_t *fm, void **addr) {
    return queue_peek(&(fm->free_frames), addr);
}

/** @brief Deallocates a frame
 *  @param fm The frame manager
 *  @param addr Physical address that is to be inserted into free frames
 *  @return 0 on success, 1 on failure */
int fm_dealloc(frame_manager_t *fm, void *addr){
    if (((uint32_t)addr % PAGE_SIZE) || (uint32_t)addr < USER_MEM_START)
        return -1;
    return queue_enq(&(fm->free_frames), addr);
}


/** @brief Initializes a frame manager
 */
int fm_init(frame_manager_t *fm){
    if (fm == NULL) return -1;
    int i = USER_MEM_START/PAGE_SIZE;
    int n = machine_phys_frames();

    /* not enough memory to store up to USER_MEM_START */
    if (n < i) return -1;

    if (queue_init(&(fm->free_frames)) < 0) return -1;
    for (; i < n; i++){
        void *addr = (void *)(i * PAGE_SIZE);
        queue_enq(&(fm->free_frames), addr);
    }
    return 0;
}

void fm_destroy(frame_manager_t *fm){
    if (fm == NULL) return;
    queue_destroy(&(fm->free_frames));
    return;
}

int fm_num_free_frames(frame_manager_t *fm){
    return queue_size(&(fm->free_frames));
}
