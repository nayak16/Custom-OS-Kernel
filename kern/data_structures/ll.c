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
/**
 * @brief Inits a linked list node
 *
 * @param node Node to init
 * @param data Data to store in node
 *
 * @return 0 on success, negative error code otherwise
 *
 */
int ll_node_init(ll_node_t *node, void *data) {
    if (node == NULL) return -1;
    node->prev = NULL;
    node->next = NULL;
    node->e = data;

    return 0;
}

/**
 * @brief Gets data from node and stores it in datap
 *
 * @param node Node to get data from
 * @param datap Address to store data
 *
 * @return 0 on success, negative error code otherwise
 *
 */
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
int ll_add_last(ll_t *ll, void *value){
    if (ll == NULL) return -1;

    /* make a new node */
    ll_node_t *new_node = (ll_node_t *)malloc(sizeof(ll_node_t));
    if (ll_node_init(new_node, value) < 0 ) return -2;

    /* Link it to the tail of the list */
    return ll_link_node_last(ll, new_node);
}

/**
 * @brief Add to front of linked list provided
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
int ll_add_first(ll_t *ll, void *value){
    if (ll == NULL) return -1;

    /* make a new node */
    ll_node_t *new_node = (ll_node_t *)malloc(sizeof(ll_node_t));
    if (ll_node_init(new_node, value) < 0 ) return -2;

    /* Link it to the tail of the list */
    return ll_link_node_first(ll, new_node);
}

/**
 * @brief Remove an element from the front of a linked list and get its value
 *
 * @param ll Pointer to linked list to dequeue from
 * @param val Address to put the dequeued value
 *
 * @return 0 on success, -1 on error
 */
int ll_remove_first(ll_t *ll, void **val){
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
    if (ll->head != NULL) ll->head->prev = NULL;

    /* save value before freeing struct */
    if (val != NULL) *val = head->e;
    free(head);
    ll->size--;
    return 0;
}

/**
 * @brief Remove an element from the back of a linked list and get its value
 *
 * @param ll Pointer to linked list to dequeue from
 * @param val Address to put the dequeued value
 *
 * @return 0 on success, -1 on error
 */
int ll_remove_last(ll_t *ll, void **val){
    if (ll == NULL) return -1;
    /* Check if empty */
    if (ll->size == 0) return -2;

    /* save tail of queue */
    ll_node_t *tail = ll->tail;
    if (tail == ll->head){
        ll->head = NULL;
    }
    /* update tail */
    ll->tail = tail->prev;
    /* unlink new tail from old tail */
    if (ll->tail != NULL) ll->tail->next = NULL;

    /* save value before freeing struct */
    if (val != NULL) *val = tail->e;
    free(tail);
    ll->size--;
    return 0;
}

/**
 * @brief Peek and return the value at the front of the linked list
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
 * @brief Statically Moves the head of the list to the tail
 *
 * @param ll Linked list to modify
 *
 * @return 0 on success, negative error code otherwise
 *
 */
int ll_cycle(ll_t *ll) {
    if (ll == NULL) return -1;

    /* Check if linked list is empty */
    if (ll->size == 0) return -2;

    /* No need to cycle if only one in ll */
    if (ll->size == 1) return 0;
    /* Save original head */
    ll_node_t *orig_head = ll->head;

    /* Set new head */
    ll->head = orig_head->next;

    /* orig_head next now points to nothing */
    orig_head->next = NULL;

    /* Link up next and prev pointers with tail */
    orig_head->prev = ll->tail;
    ll->tail->next = orig_head;

    /* orig_head is now tail of list */
    ll->tail = orig_head;

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
int ll_link_node_last(ll_t *ll, ll_node_t *new_node) {
    if (ll == NULL || new_node == NULL) return -1;

    if (ll->head == NULL){
        ASSERT(ll->tail == NULL);
        /* no elements in list yet */
        ll->head = new_node;
        ll->tail = new_node;
    } else {
        /* link new node and update list's tail pointer */
        ll->tail->next = new_node;
        new_node->prev = ll->tail;
        ll->tail = new_node;
    }
    ll->size++;
    return 0;
}

int ll_link_node_first(ll_t *ll, ll_node_t *new_node){
    if (ll == NULL || new_node == NULL) return -1;

    if (ll->head == NULL){
        ASSERT(ll->tail == NULL);
        /* no elements in list yet */
        ll->head = new_node;
        ll->tail = new_node;
    } else {
        ll->head->prev = new_node;
        new_node->next = ll->head;
        ll->head = new_node;
    }
    ll->size++;
    return 0;
}

/**
 * @brief Links the specified ll_node_t such that the resulting linked list
 *        is sorted by the comparison function cmp(a,b) which returns -1 if
 *        a < b, 0 if a == b, and 1 if a > b.
 *
 * @param ll Linked list to link to
 * @param node Node to link
 *
 * @return 0 on success, negative erorr code otherwise
 */
int ll_link_node_sorted(ll_t *ll, ll_node_t *new_node,
        int (*cmp)(void *, void *)) {
    if (ll == NULL || new_node == NULL || cmp == NULL) return -1;
    if (new_node->e == NULL) return -2;

    if (ll->head == NULL){
        ASSERT(ll->tail == NULL);
        /* no elements in list yet */
        ll->head = new_node;
        ll->tail = new_node;
    } else {
        ll_node_t *curr_node = ll->head;
        /* check to see if new node is smallest number */
        if ((*cmp)(new_node->e, curr_node->e) < 0){
            return ll_link_node_first(ll, new_node);
        }
        /* go through list until we find a node such that our new value is between
         * the curr_node and curr_node->next */
        while (curr_node->next != NULL){
            /* check for wonky node elements */
            if (curr_node->e == NULL) return -2;
            /* assert that our list is ordered up to this point */
            ASSERT((*cmp)(curr_node->e, curr_node->next->e) <= 0);

            if ((*cmp)(new_node->e, curr_node->e) >= 0 &&
                (*cmp)(new_node->e, curr_node->next->e) <= 0){
                break;
            } else {
                curr_node = curr_node->next;
            }
        }
        /* at this point, curr_node points to the node that the new_node should be
         * inserted after */
        if (curr_node->next == NULL){
            return ll_link_node_last(ll, new_node);
        } else {
            curr_node->next->prev = new_node;
            new_node->next = curr_node->next;
            curr_node->next = new_node;
            new_node->prev = curr_node;
        }
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
    }

    /* Node is in between other nodes */
    if (node->next != NULL) node->next->prev = node->prev;
    if (node->prev != NULL) node->prev->next = node->next;

    /* Remove any old links */
    node->next = NULL;
    node->prev = NULL;
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
int ll_remove(ll_t *ll, void *(*func)(void*), void *c_val, void** valp){
    if (ll == NULL || func == NULL){
        return -1;
    }
    ll_node_t *node = ll->head;

    /* Loop through linked list */
    while (node != NULL){
        if ((*func)(node->e) == c_val){
            /* Save node data */
            if (valp != NULL) *valp = node->e;
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

