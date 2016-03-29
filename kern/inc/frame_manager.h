/** @file frame_manager.h
 *  @brief interface for a vm frame manager
 *
 *  @author Aatish Nayak (aatishn)
 *  @author Christopher Wei (cjwei)
 *  @bug No known bugs.
 */

#ifndef _FRAME_MANAGER_H_
#define _FRAME_MANAGER_H_

#include <ll.h>
#include <stdbool.h>
#include <mutex.h>

typedef struct frame_manager{
    mutex_t m;
    /* largest page size = 2^(num_bin-1) */
    uint32_t num_bins;
    ll_t **frame_bins;
    ll_t *alloc_frames;
} frame_manager_t;

typedef struct frame{
    ll_node_t *buddy;
    ll_node_t *parent;
    uint32_t addr;
    uint32_t num_pages;
    uint32_t allocated;
    uint32_t i;
} frame_t;


int fm_alloc(frame_manager_t *fm, uint32_t num_pages, void **addr);
int fm_dealloc(frame_manager_t *fm, void *addr);
int fm_init(frame_manager_t *fm, uint32_t num_bins);
void fm_destroy(frame_manager_t *fm);

#endif /* _FRAME_MANAGER_H_ */
