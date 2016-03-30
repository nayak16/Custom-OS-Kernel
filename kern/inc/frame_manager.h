/** @file frame_manager.h
 *  @brief interface for a vm frame manager
 *
 *  @author Aatish Nayak (aatishn)
 *  @author Christopher Wei (cjwei)
 *  @bug No known bugs.
 */

#ifndef _FRAME_MANAGER_H_
#define _FRAME_MANAGER_H_

#include <stdbool.h>
#include <mutex.h>
#include <ht.h>


typedef struct frame_manager{
    mutex_t m;
    ht_t *allocated;
    ht_t *deallocated;
    ht_t *parents;
    ll_t **frame_bins;
    uint32_t num_bins;
} frame_manager_t;

int fm_alloc(frame_manager_t *fm, uint32_t num_pages, void **addr);
int fm_dealloc(frame_manager_t *fm, void *addr);
int fm_init(frame_manager_t *fm, uint32_t num_bins);
void fm_destroy(frame_manager_t *fm);

#endif /* _FRAME_MANAGER_H_ */
