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
#include "contracts.h"

#define FRAME_ALLOCATED 0
#define FRAME_UNALLOCATED 1

#define RND_PWR_2(n){\
    n--; \
    n |= n >> 1; \
    n |= n >> 2; \
    n |= n >> 4; \
    n |= n >> 8; \
    n |= n >> 16;\
    n++;\
}

#define TWO_POW(i) (2 << (i))
#define MIN(x,y) (x < y ? x : y)

int fm_request_split(frame_manager_t *fm, uint32_t i){
    if (fm == NULL) return -1;
    if (i >= fm->num_bins || i == 0) return -2;
    ll_t *frame_bin = fm->frame_bins[i];
    int num_frames = ll_size(frame_bin);
    if (num_frames == 0)
        if (fm_request_split(fm, i+1) < 0) return -3;
    if (num_frames < 0)
        panic("Linked list scrobbled");

    /* atleast one block exists: retrieve pointer to the frame and move it
     * to the alloc pool */

    ll_node_t *node;
    frame_t *frame;
    /* get node and unlink it from our bin */
    ll_get_first(frame_bin, &node);
    ll_unlink_node(frame_bin, node);
    /* get the frame data */
    ll_node_get_data(node, (void **)&frame);
    /* move node to alloc_frames */
    frame->allocated = FRAME_ALLOCATED;
    ll_link_node_last(fm->alloc_frames, node);

    /* create two children */
    frame_t *frame_left = malloc(sizeof(frame_t));
    frame_t *frame_right = malloc(sizeof(frame_t));
    if (frame_left == NULL || frame_right == NULL)
        panic("could not malloc another frame");
    /* create ll nodes for both children */
    ll_node_t *node_left = malloc(sizeof(ll_node_t));
    ll_node_t *node_right = malloc(sizeof(ll_node_t));
    if (node_left == NULL || node_right == NULL)
        panic("Could not malloc nodes for frames");
    ll_node_init(node_left, (void *)frame_left);
    ll_node_init(node_right, (void *)frame_right);


    /* initialize both children */
    frame_left->parent = node;
    frame_left->buddy = node_right;
    frame_left->addr = frame->addr;
    frame_left->num_pages = frame->num_pages/2;
    frame_left->allocated = FRAME_UNALLOCATED;
    frame_left->i = i-1;

    frame_right->parent = node;
    frame_right->buddy = node_left;
    frame_right->addr = frame->addr + ((frame->num_pages/2) * PAGE_SIZE);
    frame_right->num_pages = frame->num_pages/2;
    frame_right->allocated = FRAME_UNALLOCATED;
    frame_left->i = i-1;

    /* add both children to the lower pool */
    ll_link_node_last(fm->frame_bins[i-1], node_right);
    ll_link_node_last(fm->frame_bins[i-1], node_left);

    return 0;
}


int fm_request(frame_manager_t *fm, uint32_t i, void **addr){
    if (fm == NULL) return -1;
    if (i >= fm->num_bins) return -2;

    /* retrieve the requested frame bin */
    ll_t *frame_bin = fm->frame_bins[i];
    int num_frames = ll_size(frame_bin);

    if (num_frames == 0){
        if (fm_request_split(fm, i+1) < 0) return -3;
    }
    /* at this point we should have atleast one frame in our bucket */
    ASSERT(ll_size(frame_bin) > 0);

    ll_node_t *node;
    frame_t *frame;
    /* get node and unlink it from our bin */
    ll_get_first(frame_bin, &node);
    ll_unlink_node(frame_bin, node);
    /* get the frame data */
    ll_node_get_data(node, (void **)&frame);
    *addr = (void *)frame->addr;
    frame->allocated = FRAME_ALLOCATED;
    /* move node to alloc_frames */
    ll_link_node_last(fm->alloc_frames, node);
    return 0;

}


int fm_alloc(frame_manager_t *fm, uint32_t num_pages, void **addr){
    if (fm == NULL) return -1;
    mutex_lock(&(fm->m));
    /* check for enough pages */
    uint32_t i = fm->num_bins-1;
    if (num_pages > TWO_POW(i)){
        mutex_unlock(&(fm->m));
        return -1;
    }

    /* find best fitting bin if possible */
    for (; i >= 1; i--){
        if (num_pages <= TWO_POW(i) && num_pages > TWO_POW(i-1)){
            break;
        }
    }

    int status = fm_request(fm, i, addr);

    mutex_unlock(&(fm->m));

    return status;
}

void *frame_get_addr(void *frame_p){
    return (void *)(((frame_t *)frame_p)->addr);
}

int fm_dealloc(frame_manager_t *fm, void *addr){
    if (fm == NULL) return -1;
    ll_node_t *node;
    frame_t *frame;

    mutex_lock(&(fm->m));

    if (ll_find_node(fm->alloc_frames, &frame_get_addr, addr, &node) < 0){
        mutex_unlock(&(fm->m));
        return -2;
    }
    if (ll_node_get_data(node, (void **)&frame) < 0){
        mutex_unlock(&(fm->m));
        return -3;
    }
    /* get buddy if present */
    frame_t *buddy_frame;
    if (frame->buddy != NULL){
        if (ll_node_get_data(frame->buddy, (void **)&buddy_frame) < 0)
            panic("could not find buddy info!");

        if (buddy_frame->allocated == FRAME_UNALLOCATED){
            ASSERT(buddy_frame->buddy == frame);
            ASSERT(buddy_frame->num_pages == frame->num_pages);

            /* move parent back to its proper pool */
            ll_unlink_node(fm->alloc_frames, frame->parent);
            frame_t *parent_frame;
            ll_node_get_data(frame->parent, (void **)&parent_frame);
            parent_frame->allocated = FRAME_UNALLOCATED;
            ll_link_node_first(fm->frame_bins[parent_frame->i], frame->parent);

            /* unlink and destroy current frame and buddy frame */
            ll_unlink_node(fm->alloc_frames, node);
            ll_unlink_node(fm->frame_bins[frame->i], frame->buddy);
            free(frame->buddy);
            free(node);
        } else {
            ll_unlink_node(fm->alloc_frames, node);
            frame->allocated = FRAME_UNALLOCATED;
            ll_link_node_last(fm->frame_bins[frame->i], node);
        }
    } else {
        //TODO: dont feel like handling this case yet
        panic("fm_dealloc case not implemented");
    }

    mutex_unlock(&(fm->m));

    return 0;
}

int fm_alloc_user_space(frame_manager_t *fm){
    uint32_t addr = USER_MEM_START;
    uint32_t num_pages = (0xFFFFFFFF - USER_MEM_START + 1)/PAGE_SIZE;
    uint32_t i;
    for (i = fm->num_bins-1; i >= 0; i--){
        while (TWO_POW(i) <= num_pages){
            num_pages -= TWO_POW(i);
            frame_t *new_frame = malloc(sizeof(frame_t));
            new_frame->addr = addr;
            addr += TWO_POW(i) * PAGE_SIZE;
            new_frame->num_pages = TWO_POW(i);
            new_frame->allocated = FRAME_UNALLOCATED;
            new_frame->i = i;
            new_frame->parent = NULL;
            new_frame->buddy = NULL;
        }
    }
    return 0;
}

int fm_init(frame_manager_t *fm, uint32_t num_bins){
    if (fm == NULL) return -1;

    /* check if we have enough for USER_MEM_START */
    int n_kern = USER_MEM_START/PAGE_SIZE;
    int n = machine_phys_frames();
    if (n < n_kern) return -1;

    if (mutex_init(&(fm->m)) < 0) return -1;
    fm->num_bins = num_bins;

    fm->alloc_frames = malloc(sizeof(ll_t));
    ll_init(fm->alloc_frames);

    fm->frame_bins = malloc(num_bins * sizeof(ll_t));
    uint32_t i;
    for (i = 0; i < num_bins; i++){
        ll_init(fm->frame_bins[i]);
    }
    return fm_alloc_user_space(fm);
}

void fm_destroy(frame_manager_t *fm){
    if (fm == NULL) return;
    mutex_destroy(&(fm->m));
    ll_destroy(fm->alloc_frames);
    uint32_t i;
    for (i=0; i < fm->num_bins; i++){
        free(fm->frame_bins[i]);
    }
    free(fm->frame_bins);
    return;
}

