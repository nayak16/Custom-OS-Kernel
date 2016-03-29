/** @file queue.c
 *  @brief Implements a queue using linked lists
 *  @author Christopher Wei (cjwei)
 *  @author Aatish Nayak (aatishn)

 *  @bug No known bugs
 */

#include <queue.h>
#include <ll.h>
#include <stdlib.h>

int queue_init(queue_t *q){
    if (q == NULL) return -1;
    ll_t *ll_new = malloc(sizeof(ll_t));
    q->ll = ll_new;
    return ll_init(q->ll);
}
int queue_enq(queue_t *q, void *value){
    if (q == NULL) return -1;
    return ll_add_last(q->ll, value);
}
int queue_deq(queue_t *q, void **value_ptr){
    if (q == NULL) return -1;
    return ll_remove_first(q->ll, value_ptr);
}
void queue_destroy(queue_t *q){
    if (q == NULL) return;
    ll_destroy(q->ll);
    free(q->ll);
}
int queue_peek(queue_t *q, void **value_ptr){
    if (q == NULL) return -1;
    return ll_peek(q->ll, value_ptr);
}
int queue_size(queue_t *q){
    if (q == NULL) return -1;
    return ll_size(q->ll);
}
