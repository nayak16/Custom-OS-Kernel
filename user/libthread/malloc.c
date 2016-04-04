/** @file malloc.c
 *  @brief This file defines the implementation of thread safe memory
 *         allocation functions
 *
 *  We make heap memory writes thread safe by using a global heap mutex that
 *  only allows one thread to make changes to the heap. Although this design
 *  choice seems to be slow (sequentially slow, to be exact) it would be
 *  impossible to predict which memory locations _malloc may look through
 *  without direct access to the implementation and thus improbable to allow
 *  mutual exclusion to select sections of the heap.
 *  
 *  @author Christopher Wei (cjwei), Aatish Nayak (aatishn)
 *  @bug No known bugs
 */

#include <stdlib.h>
#include <types.h>
#include <stddef.h>
#include <mutex.h>
#include <thr_internals.h>

/** @brief Thread safe malloc
 *  @param __size Size of memory to be allocated
 *  @return Pointer to allocated memeory
 */
void *malloc(size_t __size)
{
    mutex_lock(&heap_mutex);
    void *ptr = _malloc(__size);
    mutex_unlock(&heap_mutex);
    return ptr;
}

/** @brief Thread safe calloc
 *  @param __nelt Number of elements to be allocated
 *  @param __eltsize Size of memory to be allocated per element
 *  @return Pointer to allocated memeory
 */
void *calloc(size_t __nelt, size_t __eltsize)
{
    mutex_lock(&heap_mutex);
    void *ptr = _calloc(__nelt, __eltsize);
    mutex_unlock(&heap_mutex);
    return ptr;
}

/** @brief Thread safe realloc
 *  @param __buf Pointer to source buffer
 *  @param __new_size New size to be reallocated
 *  @return Pointer to allocated memeory
 */
void *realloc(void *__buf, size_t __new_size)
{
    mutex_lock(&heap_mutex);
    void *ptr = _realloc(__buf, __new_size);
    mutex_unlock(&heap_mutex);
    return ptr;
}

/** @brief Thread safe free
 *  @param __buf Pointer to source buffer to be freed
 *  @return Void
 */
void free(void *__buf)
{
    mutex_lock(&heap_mutex);
    _free(__buf);
    mutex_unlock(&heap_mutex);
}
