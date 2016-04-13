/** @file stack.c
 *  @brief Implements a stack using linked lists
 *  @author Christopher Wei (cjwei)
 *  @author Aatish Nayak (aatishn)

 *  @bug No known bugs
 */

#include <stack.h>
#include <ll.h>
#include <stdlib.h>
/**
 * @brief initializes a stack
 *
 * @param stk The stack
 *
 * @return 0 on success, negative erorr code otherwise
 */
int stack_init(stack_t *stk){
    if (stk == NULL) return -1;
    ll_t *ll_new = malloc(sizeof(ll_t));
    stk->ll = ll_new;
    return ll_init(stk->ll);
}
/**
 * @brief pushes an element to the stack
 *
 * @param stk The stack
 * @param value The value to push
 *
 * @return 0 on success, negative erorr code otherwise
 */
int stack_push(stack_t *stk, void *value){
    if (stk == NULL) return -1;
    return ll_add_first(stk->ll, value);
}
/**
 * @brief pops an element from the stack
 *
 * @param stk The stack
 * @param value_ptr The pointer to store the value that is popped to
 *
 * @return 0 on success, negative erorr code otherwise
 */
int stack_pop(stack_t *stk, void **value_ptr){
    if (stk == NULL) return -1;
    return ll_remove_first(stk->ll, value_ptr);
}
/**
 * @brief Destroys a stack
 *
 * @param stk The stack
 *
 * @return 0 on success, negative erorr code otherwise
 */
void stack_destroy(stack_t *stk){
    if (stk == NULL) return;
    ll_destroy(stk->ll);
    free(stk);
}
/**
 * @brief Peeks at the next element in a stack
 *
 * @param stk The stack
 * @param value_ptr Where to store the value
 *
 * @return 0 on success, negative erorr code otherwise
 */
int stack_peek(stack_t *stk, void **value_ptr){
    if (stk == NULL) return -1;
    return ll_peek(stk->ll, value_ptr);
}

/**
 * @brief Returns the number of elements in a stack
 *
 * @param stk The stack
 *
 * @return number of elements in a stack
 */
int stack_size(stack_t *stk){
    if (stk == NULL) return -1;
    return ll_size(stk->ll);
}
