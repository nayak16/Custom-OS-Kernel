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

/* true and false defines */
#include <stdbool.h>

/* malloc, NULL define */
#include <stdlib.h>

/* uint32_t */
#include <stdint.h>

#include <frame_manager.h>

/** @brief Allocates a frame
 *  @param fm The frame manager
 *  @param addr Result pointer to be stored to if needed
 *  @return 0 on success, -1 on failure */
int fm_alloc(frame_manager_t *fm, void **addr){
    mutex_lock(&(fm->m));
    int status = queue_deq(&(fm->free_frames), addr);
    mutex_unlock(&(fm->m));
    return status;
}

/** @brief Gets address of next free frame
 *  @param fm The frame manager
 *  @param addr Result pointer to be stored
 *  @return 0 on success, -1 on failure */
int fm_nxt_free_frame(frame_manager_t *fm, void **addr) {
    mutex_lock(&(fm->m));
    int status = queue_peek(&(fm->free_frames), addr);
    mutex_unlock(&(fm->m));
    return status;
}

/** @brief Deallocates a frame
 *  @param fm The frame manager
 *  @param addr Physical address that is to be inserted into free frames
 *  @return 0 on success, -1 on failure */
int fm_dealloc(frame_manager_t *fm, void *addr){
    if (((uint32_t)addr % PAGE_SIZE) || (uint32_t)addr < USER_MEM_START)
        return -1;
    mutex_lock(&(fm->m));
    int status = queue_enq(&(fm->free_frames), addr);
    mutex_unlock(&(fm->m));
    return status;
}


/** @brief Initializes a frame manager
 *  @param fm The frame manager
 *  @return 0 on success, -1 on failure
 */
int fm_init(frame_manager_t *fm){
    if (fm == NULL) return -1;

    int i = USER_MEM_START/PAGE_SIZE;
    int n = machine_phys_frames();

    /* not enough memory to store up to USER_MEM_START */
    if (n < i) return -1;
    if (mutex_init(&(fm->m)) < 0) return -1;
    if (queue_init(&(fm->free_frames)) < 0){
        mutex_destroy(&(fm->m));
        return -1;
    }

    /* not entirely necessary but safe */
    mutex_lock(&(fm->m));
    for (; i < n; i++){
        void *addr = (void *)(i * PAGE_SIZE);
        queue_enq(&(fm->free_frames), addr);
    }
    mutex_unlock(&(fm->m));
    return 0;
}

/** @brief Destroys and cleans up a frame manager
 *  @param fm The frame manager
 *  @return Void */
void fm_destroy(frame_manager_t *fm){
    if (fm == NULL) return;
    mutex_destroy(&(fm->m));
    queue_destroy(&(fm->free_frames));
    return;
}

/** @brief Gets number of frames remaining in frame manager
 *  @param fm The frame manager
 *  @return Number of free frames, -1 on invalid input */
int fm_num_free_frames(frame_manager_t *fm){
    mutex_lock(&(fm->m));
    int n = queue_size(&(fm->free_frames));
    mutex_unlock(&(fm->m));
    return n;
}
