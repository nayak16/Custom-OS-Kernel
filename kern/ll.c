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
    if (ll == NULL)
        return -1;

    /* make a new node */
    ll_node_t *new_node = (ll_node_t *)malloc(sizeof(ll_node_t));
    if (new_node == NULL)
        return -2;
    new_node->e = value;
    new_node->next = NULL;

    if (ll->head == NULL){
        ASSERT(ll->tail == NULL);
        /* no elements in queue yet */
        ll->head = new_node;
        ll->tail = new_node;
    } else {
        /* link new node and update queue's back pointer */
        ll->tail->next = new_node;
        ll->tail = new_node;
    }
    ll->size++;
    return 0;
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
    if (ll == NULL || val == NULL){
        return -1;
    }
    /* save head of queue */
    ll_node_t *head = ll->head;
    if (ll->size == 0)
        return -1;
    if (head == ll->tail){
        ll->tail = NULL;
    }
    /* update head of queue */
    ll->head = head->next;
    /* save value before freeing struct */
    *val = head->e;
    free(head);
    ll->size--;
    return 0;
}

/**
 * @brief Remove a linked list node with data matching the argument
 *
 * @param ll Pointer to linked list to remove from
 * @param data Data to search for
 *
 * @return 0 on success, negative error code if node not found or error
 *
 */
int ll_remove(ll_t *ll, void* data) {
    if (ll == NULL) return -1;
    ll_node_t *node = ll->head;
    while(node != NULL && node->next != NULL && node->next->e != data) {
        node = node->next;
    }
    if (node == NULL) return -1;
    // Only one node with data desired
    else if (node->e == data) {
        free(node);
        ll->head = NULL;
        ll->tail = NULL;
    } else if (node->next == NULL) {
        return -1;
    } else {
        ll_node_t *next = node->next;
        node->next = next->next;
        free(next);
    }
    ll->size--;
    return 0;

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
    return -1; /* not found */
}


