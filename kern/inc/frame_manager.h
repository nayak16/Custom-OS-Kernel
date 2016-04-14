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

/** @brief defines a frame manager struct */
typedef struct frame_manager{
    /** @brief internal mutex */
    mutex_t m;
    /** @brief hash table for allocated frames */
    ht_t *allocated;
    /** @brief hash table for deallocated frames */
    ht_t *deallocated;
    /** @brief hash table for parent frames */
    ht_t *parents;
    /** @brief array of linked lists that store deallocated frames of various
     * sizes */
    ll_t **frame_bins;
    /** @brief length of frame_bins */
    uint32_t num_bins;
} frame_manager_t;

int fm_alloc(frame_manager_t *fm, uint32_t num_pages, uint32_t *addr);
int fm_dealloc(frame_manager_t *fm, uint32_t addr);
int fm_init(frame_manager_t *fm, uint32_t num_bins);
void fm_destroy(frame_manager_t *fm);
void fm_print(frame_manager_t *fm);

#endif /* _FRAME_MANAGER_H_ */
