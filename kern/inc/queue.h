/** @file queue.h
 *  @brief Defines interface for a queue
 *  @author Christopher Wei (cjwei)
 *  @author Aatish Nayak (aatishn)

 *  @bug No known bugs
 */


#ifndef _QUEUE_H_
#define _QUEUE_H_

#include <ll.h>

/**
 * @brief Struct representing a singly linked linked list node
 */
typedef struct queue {
    ll_t *ll;
} queue_t;

int queue_init(queue_t *q);
int queue_enq(queue_t *q, void *value);
int queue_deq(queue_t *q, void **value_ptr);
void queue_destroy(queue_t *q);
int queue_peek(queue_t *q, void **value_ptr);
int queue_size(queue_t *q);

#endif /* _QUEUE_H_ */
