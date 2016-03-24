/** @file queue.h
 *  @brief Defines interface for a queue
 *  @author Christopher Wei (cjwei)
 *  @author Aatish Nayak (aatishn)

 *  @bug No known bugs
 */


#ifndef _STACK_H_
#define _STACK_H_

#include <ll.h>

/**
 * @brief Struct representing a singly linked linked list node
 */
typedef struct stack {
    ll_t *ll;
} stack_t;

int stack_init(stack_t *stk);
int stack_push(stack_t *stk, void *value);
int stack_pop(stack_t *stk, void **value_ptr);
void stack_destroy(stack_t *stk);
int stack_peek(stack_t *stk, void **value_ptr);
int stack_size(stack_t *stk);

#endif /* _STACK_H_ */
