/** @file cleanup.h
 *  @brief Defines interface for a cleanup routine manager
 *  @author Christopher Wei (cjwei)
 *  @author Aatish Nayak (aatishn)

 *  @bug No known bugs
 */


#ifndef _CLEANUP_H_
#define _CLEANUP_H_

#include <ll.h>
#include <stack.h>
/**
 * @brief Struct representing a singly linked linked list node
 */
typedef struct cleanup {
    stack_t *stk;
} cleanup_t;

int cleanup_init(cleanup_t *clu);
int cleanup_push(cleanup_t *clu, void (*routine)(void *), void *arg);
int cleanup_pop(cleanup_t *clu, int execute);
void cleanup_destroy(cleanup_t *clu);
int cleanup_size(cleanup_t *clu);

#endif /* _CLEANUP_H_ */
