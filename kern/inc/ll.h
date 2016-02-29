/** @file ll.h
 *  @brief Defines interface for linked list
 *  @author Christopher Wei (cjwei)
 *  @author Aatish Nayak (aatishn)

 *  @bug No known bugs
 */


#ifndef _LL_H_
#define _LL_H_

/**
 * @brief Struct representing a singly linked linked list node
 */
typedef struct ll_node {
    /** @brief Data the ll_node_t holds */
    void *e;

    /** @brief Pointer pointing to next node */
    struct ll_node *next;

    /** @brief Pointer pointing to next node */
    struct ll_node *prev;

} ll_node_t;

/**
 * @brief Struct representing a linked list
 */
typedef struct ll {

    /** @brief Pointer to head of the linked list */
    ll_node_t *head;

    /** @brief Pointer to tail of the linked list */
    ll_node_t *tail;

    /** @brief Size of the linked list */
    unsigned int size;
} ll_t;

int ll_init(ll_t *ll);
int ll_add(ll_t *ll, void *value);
int ll_deq(ll_t *ll, void **value_ptr);
void ll_destroy(ll_t *ll);

int ll_find(ll_t *ll, void *(*func)(void*), void *c_val, void **val_ptr);
int ll_remove(ll_t *ll, void* data);

int ll_size(ll_t *ll);

#endif /* _LL_H_ */
