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

typedef struct frame_manager{
    ll_t free_user_frames;
    ll_t allocated_user_frames;
    ll_t free_kernel_frames;
    ll_t allocated_kernel_frames;
} frame_manager_t;

int frame_manager_init(frame_manager_t *fm);
void frame_manager_destroy(frame_manager_t *fm);

#endif /* _FRAME_MANAGER_H_ */