/** @file stack.c
 *  @brief Implements a stack using linked lists
 *  @author Christopher Wei (cjwei)
 *  @author Aatish Nayak (aatishn)

 *  @bug No known bugs
 */

#include <stack.h>
#include <ll.h>
#include <stdlib.h>

int stack_init(stack_t *stk){
    if (stk == NULL) return -1;
    ll_t *ll_new = malloc(sizeof(ll_t));
    stk->ll = ll_new;
    return ll_init(stk->ll);
}
int stack_push(stack_t *stk, void *value){
    if (stk == NULL) return -1;
    return ll_add_first(stk->ll, value);
}
int stack_pop(stack_t *stk, void **value_ptr){
    if (stk == NULL) return -1;
    return ll_remove_first(stk->ll, value_ptr);
}
void stack_destroy(stack_t *stk){
    if (stk == NULL) return;
    ll_destroy(stk->ll);
    free(stk);
}
int stack_peek(stack_t *stk, void **value_ptr){
    if (stk == NULL) return -1;
    return ll_peek(stk->ll, value_ptr);
}
int stack_size(stack_t *stk){
    if (stk == NULL) return -1;
    return ll_size(stk->ll);
}
