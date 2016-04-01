/** @file ll.h
 *  @brief Defines interface for linked list
 *  @author Christopher Wei (cjwei)
 *  @author Aatish Nayak (aatishn)

 *  @bug No known bugs
 */


#ifndef _LL_H_
#define _LL_H_

/**
 * @brief Struct representing a doubly linked linked list node
 */
typedef struct ll_node {
    /** @brief Data the ll_node_t holds */
    void *e;

    /** @brief Pointer pointing to next node */
    struct ll_node *next;

    /** @brief Pointer pointing to prev node */
    struct ll_node *prev;

} ll_node_t;

int ll_node_init(ll_node_t *node, void *data);
int ll_node_get_data(ll_node_t *node, void **datap);

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

int ll_add_last(ll_t *ll, void *value);
int ll_add_first(ll_t *ll, void *value);
int ll_remove_last(ll_t *ll, void **value_ptr);
int ll_remove_first(ll_t *ll, void **value_ptr);

int ll_peek(ll_t *ll, void **value_ptr);
int ll_cycle(ll_t *ll);

int ll_link_node_first(ll_t *ll, ll_node_t *node);
int ll_link_node_last(ll_t *ll, ll_node_t *node);
int ll_link_node_sorted(ll_t *ll, ll_node_t *new_node,
        int (*cmp)(void *, void *));
int ll_unlink_node(ll_t *ll, ll_node_t *node);

int ll_head(ll_t *ll, ll_node_t **node);
int ll_tail(ll_t *ll, ll_node_t **node);

int ll_find(ll_t *ll, void *(*func)(void*), void *c_val, void **val_ptr);
int ll_remove(ll_t *ll, void *(*func)(void*), void *c_val, void **valp);

int ll_size(ll_t *ll);
void ll_destroy(ll_t *ll);

void ll_foreach(ll_t *ll, void (*f)(void *));


#endif /* _LL_H_ */
