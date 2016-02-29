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

typedef struct frame_manager{
    ll_t free_frames;
} frame_manager_t;

int fm_alloc(frame_manager_t *fm, void **addr);
int fm_dealloc(frame_manager_t *fm, void *addr);
int fm_init(frame_manager_t *fm);
void fm_destroy(frame_manager_t *fm);
int fm_get_size(frame_manager_t *fm);

#endif /* _FRAME_MANAGER_H_ */