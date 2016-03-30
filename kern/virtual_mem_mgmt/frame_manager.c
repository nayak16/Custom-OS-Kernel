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
#include <simics.h>
#include "contracts.h"
#include <frame_manager.h>

#define MAX(a,b) ((a) < (b) ? (b) : (a))
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define TWO_POW(i) (1 << (i))

typedef struct frame{
    uint32_t addr;
    uint32_t num_pages;
    int status;
    int i;
    struct frame *buddy;
    struct frame *parent;
} frame_t;

#define FRAME_ALLOC 0
#define FRAME_DEALLOC 1
#define FRAME_PARENT 2

int frame_init(frame_t *frame, uint32_t addr, uint32_t num_pages,
               int status, int i, frame_t *buddy, frame_t *parent){
    if (frame == NULL) return -1;
    frame->addr = addr;
    frame->num_pages = num_pages;
    frame->status = status;
    frame->i = i;
    frame->buddy = buddy;
    frame->parent = parent;
    return 0;
}






int address_hash(key_t addr){
    return (int)addr;
}

int request_split(frame_manager_t *fm, int i){
    if (i >= fm->num_bins || i == 0 || fm == NULL) return -1;
    if (ll_size(fm->frame_bins[i]) < 0) panic("Invalid linked list!");
    if (ll_size(fm->frame_bins[i]) == 0){
        if (request_split(fm, i+1) < 0){
            lprintf("No blocks of size %d found", TWO_POW(i));
            return -2;
        }
    }

    ASSERT(ll_size(fm->frame_bins[i]) > 0);

    frame_t *parent_frame;
    ll_node_t *parent_node;

    /* get the next avaliable frame in the frame bin and remove it */
    ll_head(fm->frame_bins[i], &parent_node);
    ll_unlink_node(fm->frame_bins[i], parent_node);
    ll_node_get_data(parent_node, (void **)&parent_frame);
    ASSERT(parent_frame->status == FRAME_DEALLOC);

    /* deregister node from deallocated and register in parent */
    ht_remove(fm->deallocated, parent_frame->addr, NULL);
    ht_put(fm->parents, parent_frame->addr, parent_node);
    parent_frame->status = FRAME_PARENT;

    /* create two new nodes and register in dealloc */
    frame_t *left_frame, *right_frame;
    ll_node_t *left_node, *right_node;

    /* create and initialize new frames */
    left_frame = malloc(sizeof(frame_t));
    right_frame = malloc(sizeof(frame_t));

    frame_init(left_frame,
            parent_frame->addr,
            parent_frame->num_pages/2,
            FRAME_DEALLOC,
            i-1,
            right_frame,
            parent_frame);

    frame_init(right_frame,
            parent_frame->addr+((parent_frame->num_pages/2)*PAGE_SIZE),
            parent_frame->num_pages/2,
            FRAME_DEALLOC,
            i-1,
            left_frame,
            parent_frame);

    left_node = malloc(sizeof(ll_node_t));
    right_node = malloc(sizeof(ll_node_t));

    /* link nodes to appropriate frame bin */
    ll_node_init(left_node, left_frame);
    ll_link_node_last(fm->frame_bins[i-1], left_node);
    ll_node_init(right_node, right_frame);
    ll_link_node_last(fm->frame_bins[i-1], right_node);

    /* register nodes in the dealloc ht */
    ht_put(fm->deallocated, left_frame->addr, left_node);
    ht_put(fm->deallocated, right_frame->addr, right_node);

    return 0;

}

int fm_alloc(frame_manager_t *fm, uint32_t num_pages, void **p_addr){
    int j;
    uint32_t frame_size = TWO_POW(fm->num_bins-1);
    if (num_pages > frame_size){
        lprintf("Requested %d pages, which exceeds maximum frame size of %d",
                (unsigned int)num_pages, (unsigned int)frame_size);
        return -1;
    }
    if (num_pages == 0){
        return -1;
    }
    for (j = fm->num_bins-1; j > 0; j--){
        frame_size = TWO_POW(j);
        if (num_pages <= frame_size && num_pages > TWO_POW(j-1)) break;
    }
    frame_size = TWO_POW(j);
    lprintf("Requested %d pages, finding frame of size %d",
            (unsigned int)num_pages, (unsigned int)frame_size);

    if (ll_size(fm->frame_bins[j]) < 0) panic("Invalid linked list!");
    if (ll_size(fm->frame_bins[j]) == 0){
        if (request_split(fm, j+1) < 0){
            lprintf("No blocks of size %d found", (unsigned int)frame_size);
            return -2;
        }
    }

    ASSERT(ll_size(fm->frame_bins[j]) > 0);

    frame_t *frame;
    ll_node_t *node;

    /* get the next avaliable frame in the frame bin and remove it */
    ll_head(fm->frame_bins[j], &node);
    ll_unlink_node(fm->frame_bins[j], node);
    ll_node_get_data(node, (void **)&frame);

    /* deregister node from deallocated and register in allocated */
    ht_remove(fm->deallocated, frame->addr, NULL);
    ht_put(fm->allocated, frame->addr, node);
    frame->status = FRAME_ALLOC;

    *p_addr = (void *)frame->addr;

    return 0;
}

int fm_dealloc(frame_manager_t *fm, void *p_addr){
    if (fm == NULL) return -1;
    ll_node_t *node;
    if (ht_remove(fm->allocated, (key_t)p_addr, (void **)&node) < 0){
        lprintf("Could not locate address in allocated ht");
        return -2;
    }
    frame_t *frame;
    ll_node_get_data(node, (void **)&frame);
    frame_t *buddy = frame->buddy;

    if (buddy != NULL && buddy->status == FRAME_DEALLOC){
        lprintf("Coalescing!");
        frame_t *parent_frame, *buddy_frame;
        ll_node_t *parent_node, *buddy_node;
        /* remove mapping in parents ht */
        if (ht_remove(fm->parents, (key_t)((frame->parent)->addr), (void **)&parent_node) < 0){
            panic("Could not locate parent in parent ht");
        }
        /* remove buddy mapping in deallocated pool */
        if (ht_remove(fm->deallocated, (key_t)buddy->addr, (void **)&buddy_node) < 0){
            panic("Could not remove buddy from dealloc ht");
        }
        /* remove from bin pool */
        ll_unlink_node(fm->frame_bins[buddy->i], buddy_node);
        ll_node_get_data(buddy_node,(void **)&buddy_frame);
        ll_node_get_data(parent_node, (void **)&parent_frame);

        /* free resources */
        free(frame);
        free(buddy_frame);
        free(buddy_node);
        free(node);
        if (ht_put(fm->deallocated, parent_frame->addr, (void **)&parent_node) < 0){
            panic("Could not register parent to deallocated");
        }
        parent_frame->status = FRAME_DEALLOC;
        ll_link_node_last(fm->frame_bins[parent_frame->i], parent_node);
        //TODO: coalesece recursive
    } else {
        /* register with dealloc and put into the linked list */
        ht_put(fm->deallocated, (key_t)frame->addr, node);
        ll_link_node_last(fm->frame_bins[frame->i], node);
        frame->status = FRAME_DEALLOC;
    }
    return 0;
}

int fm_init_user_space(frame_manager_t *fm, uint32_t num_frames){
    if (fm == NULL || num_frames == 0) return -1;
    int i;
    uint32_t num_bins = fm->num_bins;
    int frames_remaining = num_frames;
    uint32_t p_addr = USER_MEM_START;

    for (i = num_bins-1; i != -1; i--){
        uint32_t frame_size = TWO_POW(i);
        while(frames_remaining >= frame_size){
            lprintf("Allocating a frame with %d pages; %d remaining",
                    (unsigned int)frame_size,
                    (unsigned int)(frames_remaining - frame_size));
            /* create a new frame */
            frame_t *new_frame = malloc(sizeof(frame_t));
            frame_init(new_frame,
                    p_addr,
                    frame_size,
                    FRAME_DEALLOC,
                    i,
                    NULL,
                    NULL);
            /* link the frame to its respective bin */
            ll_node_t *node = malloc(sizeof(ll_node_t));
            ll_node_init(node, (void *)new_frame);
            ll_link_node_last(fm->frame_bins[i], node);

            /* create a mapping between paddr->node */
            ht_put(fm->deallocated, p_addr, node);

            frames_remaining -= frame_size;
            p_addr += PAGE_SIZE * frame_size;
        }
    }
    return 0;
}


int fm_init(frame_manager_t *fm, uint32_t num_bins){
    if (fm == NULL) return -1;

    int i = USER_MEM_START/PAGE_SIZE;
    int n = machine_phys_frames();
    uint32_t n_addressable = (0xFFFFFFFF - USER_MEM_START + 1)/PAGE_SIZE;
    uint32_t num_frames;
    /* not enough memory to store up to USER_MEM_START */
    if (n < i) return -1;
    n -= i;
    num_frames = MIN(n, n_addressable);
    if (mutex_init(&(fm->m)) < 0) return -1;

    /* initialize hash tables */
    fm->allocated = malloc(sizeof(ht_t));
    ht_init(fm->allocated, 64, &address_hash);
    fm->deallocated = malloc(sizeof(ht_t));
    ht_init(fm->deallocated, 64, &address_hash);
    fm->parents = malloc(sizeof(ht_t));
    ht_init(fm->parents, 64, &address_hash);

    /* initialize bins */
    fm->frame_bins = malloc(sizeof(ll_t *) * num_bins);
    uint32_t j;
    for (j = 0; j < num_bins; j++){
        fm->frame_bins[j] = malloc(sizeof(ll_t));
        ll_init(fm->frame_bins[j]);
    }
    fm->num_bins = num_bins;

    if (fm_init_user_space(fm, num_frames) < 0){
        panic("Could not initialize frame manager!");
    }

    return 0;
}
void fm_destroy(frame_manager_t *fm){
    if (fm == NULL) return;
    panic("fm_destroy: TODO");
    return;
}

