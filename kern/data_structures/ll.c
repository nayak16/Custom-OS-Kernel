/**
 * @file ll.c
 *
 * @brief Linked List implementation alternatively used as a queue
 *
 * @author Aatish Nayak (aatishn) and Christopher Wei (cjwei)
 * @bug No known bugs
 */

/* C Standard Library includes */
#include <stdio.h>
#include <stdlib.h>
#include <contracts.h>

/* Thread Library includes*/
#include <ll.h>

#include <simics.h>


/************* Linked List Node Functions *************/

int ll_node_init(ll_node_t *node, void *data) {
    if (node == NULL) return -1;
    node->prev = NULL;
    node->next = NULL;
    node->e = data;

    return 0;
}

int ll_node_get_data(ll_node_t *node, void **datap) {
    if (node == NULL) return -1;
    *datap = node->e;
    return 0;
}


/**************** Linked List Functions ***************/
/**
 * @brief Initializes a linked list with appropriate values
 *
 * @param ll Pointer to linked list to initialize
 *
 * @return 0 on success, -1 on error
 */
int ll_init(ll_t *ll) {
    if (ll == NULL) return -1;

    ll->size = 0;
    ll->head = NULL;
    ll->tail = NULL;
    return 0;
}

/**
 * @brief Add to end of linked list provided
 * Can be used as enqueue operation for a linked list queue
 *
 * @param ll Pointer to linked list to add to
 * @param value Value to add to list
 *
 * @return 0 on success, negative error code on failure
 * -1 if q is NULL
 * -2 if malloc() fails
 *
 */
int ll_add(ll_t *ll, void *value){
    if (ll == NULL) return -1;

    /* make a new node */
    ll_node_t *new_node = (ll_node_t *)malloc(sizeof(ll_node_t));
    if (ll_node_init(new_node, value) < 0 ) return -2;

    /* Link it to the tail of the list */
    return ll_link_node(ll, new_node);
}

/**
 * @brief Dequeue an element from the end of a linked list
 *
 * @param ll Pointer to linked list to dequeue from
 * @param val Address to put the dequeued value
 *
 * @return 0 on success, -1 on error
 */
int ll_deq(ll_t *ll, void **val){
    if (ll == NULL) return -1;

    /* Check if empty */
    if (ll->size == 0) return -2;

    /* save head of queue */
    ll_node_t *head = ll->head;
    if (head == ll->tail){
        ll->tail = NULL;
    }
    /* update head of queue */
    ll->head = head->next;
    ll->head->prev = NULL;

    /* save value before freeing struct */
    if (val != NULL) *val = head->e;
    free(head);
    ll->size--;
    return 0;
}

/**
 * @brief Peek and return the last element of the linked list
 *
 * @param ll Pointer to linked list to dequeue from
 * @param val Address to put the dequeued value
 *
 * @return 0 on success, -1 on error
 */
int ll_peek(ll_t *ll, void **val){
    if (ll == NULL || val == NULL || ll->size == 0){
        return -1;
    }
    *val = ll->head->e;
    return 0;
}


/**
 * @brief Links the specified ll_node_t to the tail of ll specified
 *
 * @param ll Linked list to link to
 * @param node Node to link
 *
 * @return 0 on success, negative erorr code otherwise
 */
int ll_link_node(ll_t *ll, ll_node_t *new_node) {
    if (ll == NULL || new_node == NULL) return -1;

    if (ll->head == NULL){
        ASSERT(ll->tail == NULL);
        /* no elements in queue yet */
        ll->head = new_node;
        ll->tail = new_node;
    } else {
        /* link new node and update queue's back pointer */
        ll->tail->next = new_node;
        new_node->prev = ll->tail;
        ll->tail = new_node;
    }
    ll->size++;
    return 0;
}

/**
 * @brief Unlink a linked list node from the ll specified
 *
 * @param ll Pointer to linked list to remove from
 * @param node Node to remove
 *
 * @return 0 on success, negative error code if node not found or error
 *
 */
int ll_unlink_node(ll_t *ll, ll_node_t *node) {
    if (ll == NULL || node == NULL || ll->size == 0) return -1;

    /* Only one in list */
    if (node == ll->head && node == ll->tail) {
        ll->head = NULL;
        ll->tail = NULL;
    } /* Node is head */
    else if (node == ll->head) {
        ll->head = node->next;
    } /* Node is tail */
    else if (node == ll->tail) {
        ll->tail = node->prev;
    } /* Node is in between other nodes */
    else {
        node->next->prev = node->prev;
        node->prev->next = node->next;
    }
    ll->size--;
    return 0;
}

/**
 * @brief Remove a linked list node from ll and free it
 *
 * @param ll Pointer to linked list to remove from
 * @param node Node to remove
 *
 * @return 0 on success, negative error code if node not found or error
 *
 */
int ll_remove_node(ll_t *ll, ll_node_t *node) {
    if (ll == NULL || node == NULL) return -1;
    ll_unlink_node(ll, node);
    free(node);
    return 0;
}


/**
 * @brief Finds data in the linked list that satisfies the condition
 * func(data) == c_val
 *
 * @param ll Pointer to linked list
 * @param func Generic function that transforms data into form desired
 * @param c_val Value to match with return value of func
 * @param val_ptr Address to store found data in
 *
 * @return 0 on success, negative if error or data not found
 *
 */
int ll_find(ll_t *ll, void *(*func)(void*), void *c_val, void **val_ptr){
    if (ll == NULL || func == NULL){
        return -1;
    }
    ll_node_t *node = ll->head;

    /* Loop through linked list */
    while (node != NULL){
        if ((*func)(node->e) == c_val){
            *val_ptr = node->e;
            return 0;
        }
        node = node->next;
    }
    return -2; /* not found */
}


/**
 * @brief Finds data in the linked list that satisfies the condition
 * func(data) == c_val and removes that node
 *
 * @param ll Pointer to linked list
 * @param func Generic function that transforms data into form desired
 * @param c_val Value to match with return value of func
 *
 * @return 0 on success, negative if error or data not found
 *
 */
int ll_remove(ll_t *ll, void *(*func)(void*), void *c_val){
    if (ll == NULL || func == NULL){
        return -1;
    }
    ll_node_t *node = ll->head;

    /* Loop through linked list */
    while (node != NULL){
        if ((*func)(node->e) == c_val){
            /* Remove and free node */
            return ll_remove_node(ll, node);
        }
        node = node->next;
    }
    /* Not found */
    return -2;

}


/**
 * @brief Gets size of linked list
 *
 * @param ll Linked list to get size of
 *
 * @return size on success, negative error code otherwise
 *
 */
int ll_size(ll_t *ll){
    if (ll == NULL) return -1;
    return (ll->size);
}

/**
 * @brief Destroys and frees all nodes of a linked list
 *
 * @param ll Pointer to linked list to destroy
 *
 * @return void
 */
void ll_destroy(ll_t *ll){
    if (ll == NULL) return;
    ll_node_t *ptr = ll->head;
    ll_node_t *next;
    while (ptr != NULL){
        next = ptr->next;
        free(ptr);
        ptr = next;
    }
}

