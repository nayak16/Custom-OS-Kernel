/** @file frame_manager.c
 *  @brief implementation for a vm frame manager
 *
 *  We decided on implementing a binary buddy allocator for our frame manager.
 *  The buddy allocator allows us to represent a large number of pages using a
 *  single frame and allows for faster allocation for large number of pages and
 *  a smaller space footprint, as opposed to a simple linked list of free pages.
 *  Our implementation of hte buddy allocator uses bin sizes of power of two,
 *  which allow us to split and move larger pages easily. Every time a larger
 *  block is split to create smaller blocks, it is moved into a parent pool, and
 *  the two smaller blocks are linked together by a "buddy" link. Upon
 *  deallocating one of the buddy blocks, they can check to see if we should
 *  coalesce to form the larger parent block. This overall reduces external
 *  fragmentation at the cost of internal fragmentation.
 *
 *  We use a bin of linked lists to store our deallocated frames which gives us
 *  constant time deallocation requests if no splitting is required. However,
 *  since we need to be able to remove specific elements from the deallocated
 *  pool (due to coalescing and what not), we store each frame's linked list
 *  node in a particular hash table pool. Each hash table provides constant time
 *  lookup of linked list nodes which allow us to quickly place and remove
 *  linked list nodes into and out of their respective bin pools. In addition to
 *  deallocated and allocated pools, we have a parent pool which stores pointers
 *  to linked list nodes of split blocks. This allows for parents to persist
 *  after being split into smaller blocks and thus allows for multilevel
 *  coalescing.
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
#include <debug.h>
#include "contracts.h"
#include <frame_manager.h>

/** @brief macro for max function */
#define MAX(a,b) ((a) < (b) ? (b) : (a))
/** @brief macro for min function */
#define MIN(a,b) ((a) < (b) ? (a) : (b))
/** @brief macro for pow_2 */
#define TWO_POW(i) (1 << (i))

/** @brief Datastructure representing a single frame */
typedef struct frame{
    /** @brief The starting address of the frame */
    uint32_t addr;
    /** @brief The number of pages in the frame */
    uint32_t num_pages;
    /** @brief The status of the frame (allocated, deallocated, parent) */
    int status;
    /** @brief The index into frame manager deallocated bin */
    int i;
    /** @brief Pointer to the frame's buddy frame if it exists */
    struct frame *buddy;
    /** @brief Pointer to the frame's parent if it exists */
    struct frame *parent;
} frame_t;

/** @brief Status for a frame that is allocated */
#define FRAME_ALLOC 0
/** @brief Status for a frame that is deallocated */
#define FRAME_DEALLOC 1
/** @brief Status for a frame that has been split */
#define FRAME_PARENT 2


/** @brief Initializes a frame with the given parameters
 *  @param frame The frame
 *  @param addr The starting address
 *  @param num_pages The number of pages
 *  @param status The frame status
 *  @param buddy The buddy frame
 *  @param parent The parent frame
 *  @return 0 on success, negative integer code on failure
 */
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

/** @brief Hashing function for addresses
 *  @param addr The input address
 *  @return The hashed address
 */
int address_hash(key_t addr){
    return (int)addr;
}

/** @brief Hashing function for page aligned addresses
 *  @param addr The input address
 *  @return The hashed address
 */
int address_shift_hash(key_t addr){
    return ((int)addr >> PAGE_SHIFT);
}


/** @brief Takes a frame and recursively joins it to the largest sized frame
 *         possible
 *
 *  This function takes a parent frame and checks to see if it has a buddy and
 *  is deallocated. If so, the frame and its buddy are removed and freed,
 *  and the request_join is called on the parent. Otherwise, the parent is
 *  removed from the parent pool and put into the deallocated pool.
 *
 *  @param fm The frame manager
 *  @param frame The frame to join
 *  @return 0 on success, negative integer code on failure
 */
int request_join(frame_manager_t *fm, frame_t *frame){
    if (frame == NULL) return -1;

    ASSERT(frame->status == FRAME_PARENT);

    if (frame->buddy != NULL && frame->buddy->status == FRAME_DEALLOC){
        DEBUG_PRINT("Coalesing again!");
        ASSERT(frame->parent == frame->buddy->parent && frame->parent != NULL);

        frame_t *parent_frame = frame->parent;

        /* destroy current frame and its buddy */
        ll_node_t *buddy_node, *curr_node;
        if (ht_remove(fm->parents, (key_t)(frame->addr | frame->i),
                    (void *)&curr_node, NULL) < 0){
            panic("Could not remove frame from parents!");
        }
        if (ht_remove(fm->deallocated, (key_t)(frame->buddy->addr),
                    (void *)&buddy_node, NULL)< 0){
            panic("Could not remove buddy node from dealloc!");
        }
        ll_unlink_node(fm->frame_bins[frame->i], buddy_node);
        free(frame->buddy);
        free(frame);
        free(buddy_node);
        free(curr_node);

        /* call recursive join on parent */
        if (request_join(fm, parent_frame) < 0){
            panic("Invalid parent pointer!");
        }

    } else {
        /* base case when there is no buddy and no parent to further attempt to
         * join or when the buddy is currently unavaliable to join on */

        ll_node_t *node;
        /* remove mapping in parents ht */
        if (ht_remove(fm->parents, (key_t)(frame->addr | frame->i),
                    (void **)&node, NULL) < 0){
            panic("Could not locate parent in parent ht");
        }
        /* add mapping to deallocated ht */
        if (ht_put(fm->deallocated, frame->addr, node) < 0){
            panic("Could not register parent to deallocated");
        }
        frame->status = FRAME_DEALLOC;
        ll_link_node_last(fm->frame_bins[frame->i], node);
    }
    return 0;
}


/** @brief Recursively splits larger blocks into smaller blocks
 *
 *  Attempts to take a block of size 2^i and split it into two smaller blocks of
 *  size 2^(i-1). If no such block is avaliable, another request_split is called
 *  on blocks of size 2^(i+1). This goes on until there is atleast one block in
 *  the current ith pool, or we run out of bins to check.
 *
 *  @param fm The frame manager
 *  @param i The index into the frame manager's bin pool
 *  @return 0 on success, negative integer code on failure
 */
int request_split(frame_manager_t *fm, int i){
    if (i >= fm->num_bins || i == 0 || fm == NULL) return -1;
    if (ll_size(fm->frame_bins[i]) < 0) panic("Invalid linked list!");
    if (ll_size(fm->frame_bins[i]) == 0){
        if (request_split(fm, i+1) < 0){
            DEBUG_PRINT("No blocks of size %d found", TWO_POW(i));
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
    ht_remove(fm->deallocated, parent_frame->addr, NULL, NULL);
    ht_put(fm->parents, (parent_frame->addr | parent_frame->i), parent_node);
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


/** @brief Requests a frame from the frame manager
 *
 *  Endpoint for the virtual memory manager. Allocates atleast num_pages from
 *  the frame manager by finding a frame that is greater than or equal in size
 *  as the number of pages requested. The base address of that frame is then
 *  stored in p_addr.
 *
 *  @param fm The frame manager
 *  @param num_pages The number of pages requested
 *  @param p_addr The pointer to store the resulting frame address
 *  @return 0 on success, negative integer code on failure
 */
int fm_alloc(frame_manager_t *fm, uint32_t num_pages, uint32_t *p_addr){
    int j;
    mutex_lock(&fm->m);
    uint32_t frame_size = TWO_POW(fm->num_bins-1);
    if (num_pages > frame_size){
        DEBUG_PRINT("Requested %d pages, which exceeds maximum frame size of %d",
                (unsigned int)num_pages, (unsigned int)frame_size);
        mutex_unlock(&fm->m);
        return -1;
    }
    if (num_pages == 0){
        DEBUG_PRINT("Number of pages requested is 0");
        mutex_unlock(&fm->m);
        return -1;
    }
    /* find the right sized bin for the given num_pages */
    for (j = fm->num_bins-1; j > 0; j--){
        frame_size = TWO_POW(j);
        if (num_pages <= frame_size && num_pages > TWO_POW(j-1)) break;
    }
    frame_size = TWO_POW(j);
    if (ll_size(fm->frame_bins[j]) < 0) panic("Invalid linked list!");
    if (ll_size(fm->frame_bins[j]) == 0){
        if (request_split(fm, j+1) < 0){
            DEBUG_PRINT("No blocks of size %d found", (unsigned int)frame_size);
            mutex_unlock(&fm->m);
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
    ASSERT(frame->status == FRAME_DEALLOC);

    /* deregister node from deallocated and register in allocated */
    ht_remove(fm->deallocated, frame->addr, NULL, NULL);
    ht_put(fm->allocated, frame->addr, node);
    frame->status = FRAME_ALLOC;

    DEBUG_PRINT("Allocated %p to %p", (void *)frame->addr, (void *)(frame->addr + (PAGE_SIZE * frame->num_pages)));

    *p_addr = frame->addr;

    mutex_unlock(&fm->m);
    return 0;
}

/** @brief Returns a frame to the frame manager
 *
 *  Endpoint for the virtual memory manager. Find the frame with the given
 *  p_addr and returns it back to the deallocated pool
 *
 *  @param fm The frame manager
 *  @param p_addr The address of the frame to be returned
 *  @return 0 on success, negative integer code on failure
 */
int fm_dealloc(frame_manager_t *fm, uint32_t p_addr){
    if (fm == NULL) return -1;
    mutex_lock(&fm->m);

    /* Get the node from the allocated pool */
    ll_node_t *node;
    if (ht_remove(fm->allocated, (key_t)p_addr, (void **)&node, NULL) < 0){
        DEBUG_PRINT("Could not locate address in allocated ht");
        mutex_unlock(&fm->m);
        return -2;
    }
    frame_t *frame;
    ll_node_get_data(node, (void **)&frame);
    ASSERT(frame->status == FRAME_ALLOC);
    DEBUG_PRINT("Deallocating %p to %p", (void *)frame->addr, (void *)(frame->addr + PAGE_SIZE*frame->num_pages));
    frame_t *buddy_frame = frame->buddy;

    ASSERT( (!buddy_frame) ||
            (buddy_frame->parent == frame->parent
             && buddy_frame->i == frame->i
             && buddy_frame->num_pages == frame->num_pages
             && buddy_frame->buddy == frame));

    /* See if we should coalesce */
    if (buddy_frame != NULL && buddy_frame->status == FRAME_DEALLOC){
        DEBUG_PRINT(">> Coalescing %p with %p to %p", (void *)frame->addr, (void *)buddy_frame->addr,
                (void *)frame->parent);

        frame_t *parent_frame = frame->parent;
        ll_node_t *buddy_node;

        /* remove buddy mapping in deallocated pool */
        if (ht_remove(fm->deallocated, (key_t)buddy_frame->addr,
                        (void **)&buddy_node, NULL) < 0){
            panic("Could not remove buddy from dealloc ht");
        }
        /* remove from deallocated bin pool */
        ll_unlink_node(fm->frame_bins[buddy_frame->i], buddy_node);

        /* free resources */
        free(frame);
        free(buddy_frame);
        free(buddy_node);
        free(node);

        /* recursively coalesce parent frame */
        request_join(fm, parent_frame);
    } else {
        /* register with dealloc and put into the linked list */
        ht_put(fm->deallocated, (key_t)frame->addr, node);
        ll_link_node_last(fm->frame_bins[frame->i], node);
        frame->status = FRAME_DEALLOC;
    }
    mutex_unlock(&fm->m);
    return 0;
}


/** @brief Initializes the frame manager with all the frames that represent the
 *         possible physical addresses avaliable to the user space
 *
 *  Should only be called once and by fm_init. This populates our frame manager
 *  with the physical frames we want to keep track of. Note that the kernel
 *  space is not tracked by the frame manager. These frames should always
 *  persist and should never be "deallocated" or "allocated".
 *
 *  @param fm The frame manager
 *  @param num_frames Number of user space pages to allocate
 *  @return 0 on success, negative integer code on failure
 */
int fm_init_user_space(frame_manager_t *fm, uint32_t num_pages){
    if (fm == NULL || num_pages == 0) return -1;
    mutex_lock(&fm->m);
    int i;
    uint32_t num_bins = fm->num_bins;
    int pages_remaining = num_pages;
    uint32_t p_addr = USER_MEM_START;

    /* for each bin, starting with the largest bin, allocate as many frames as
     * possible */
    for (i = num_bins-1; i != -1; i--){
        uint32_t frame_size = TWO_POW(i);
        while(pages_remaining >= frame_size){
            DEBUG_PRINT("Allocating a frame with %d pages; %d remaining",
                    (unsigned int)frame_size,
                    (unsigned int)(pages_remaining - frame_size));

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

            pages_remaining -= frame_size;
            p_addr += PAGE_SIZE * frame_size;

        }
    }
    mutex_unlock(&fm->m);
    return 0;
}

/** @brief Initializes a frame manager
 *
 *  Initializes a frame manager and fills it with all the addressable frames
 *  provided by physical memory that can be used by the user space.
 *
 *  @param fm The frame manager
 *  @param num_bins The number of bins it should have for its deallocated bin
 *  @return 0 on success, negative integer code on failure
 */
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
    /* allocated and deallocate frames should have unique addresses
     * which are also page aligned. Therefore, for a decent hash
     * function we should ignore the bottom 12 bits since they are
     * all the same */
    fm->allocated = malloc(sizeof(ht_t));
    ht_init(fm->allocated, 64, &address_shift_hash);
    fm->deallocated = malloc(sizeof(ht_t));
    ht_init(fm->deallocated, 64, &address_shift_hash);
    /* Since parent address can be the same, we need a different key to hash
     * We know that there cannot be multiple entries with the same address and
     * the same bin size, we can use the bottom 12 bits of the address to store
     * addition information about the address and thus we should not page shift
     * them away when hashing*/
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

//TODO: implement or remove up to me
void fm_destroy(frame_manager_t *fm){
    if (fm == NULL) return;
    panic("fm_destroy: TODO");
    return;
}

/* Debugging purposes only */

void frame_print(void *ptr){
    frame_t *f = (frame_t *)ptr;
    if (ptr == NULL) lprintf("(NULL_FRAME)");
    else lprintf("<%p>(%d)%d: [%p, %p), parent: %p, buddy: %p",
            ptr,
            (unsigned int)f->i, (unsigned int)f->num_pages, (void *)f->addr,
            (void *)(f->addr + PAGE_SIZE*f->num_pages),
            (void *)(f->parent), (void *)(f->buddy));
}

void fm_print(frame_manager_t *fm){
    if (fm == NULL) return;
    int i;
    for (i = 0; i < fm->num_bins; i++){
        lprintf("******** BIN %d ********", i);
        ll_foreach(fm->frame_bins[i], &frame_print);
    }
}
