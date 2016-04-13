/** @file queue.c
 *  @brief Implements a queue using linked lists
 *  @author Christopher Wei (cjwei)
 *  @author Aatish Nayak (aatishn)

 *  @bug No known bugs
 */

#include <queue.h>
#include <ll.h>
#include <stdlib.h>

/**
 * @brief initializes a queue
 *
 * @param q The queue
 *
 * @return 0 on success, negative erorr code otherwise
 */
int queue_init(queue_t *q){
    if (q == NULL) return -1;
    ll_t *ll_new = malloc(sizeof(ll_t));
    q->ll = ll_new;
    return ll_init(q->ll);
}
/**
 * @brief enqueues an element to the queue
 *
 * @param q The queue
 * @param value The value to enqueue
 *
 * @return 0 on success, negative erorr code otherwise
 */
int queue_enq(queue_t *q, void *value){
    if (q == NULL) return -1;
    return ll_add_last(q->ll, value);
}
/**
 * @brief dequeues an element from the queue
 *
 * @param q The queue
 * @param value_ptr The pointer to store the value that is dequeued
 *
 * @return 0 on success, negative erorr code otherwise
 */
int queue_deq(queue_t *q, void **value_ptr){
    if (q == NULL) return -1;
    return ll_remove_first(q->ll, value_ptr);
}
/**
 * @brief Destroys a queue
 *
 * @param q The queue
 *
 * @return 0 on success, negative erorr code otherwise
 */
void queue_destroy(queue_t *q){
    if (q == NULL) return;
    ll_destroy(q->ll);
    free(q->ll);
}
/**
 * @brief Peeks at the next element in a queue
 *
 * @param q The queue
 * @param value_ptr Where to store the value
 *
 * @return 0 on success, negative erorr code otherwise
 */
int queue_peek(queue_t *q, void **value_ptr){
    if (q == NULL) return -1;
    return ll_peek(q->ll, value_ptr);
}
/**
 * @brief Returns the number of elements in a queue
 *
 * @param q The queue
 *
 * @return number of elements in a queue
 */
int queue_size(queue_t *q){
    if (q == NULL) return -1;
    return ll_size(q->ll);
}
