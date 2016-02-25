/*
 * these functions should be thread safe.
 * It is up to you to rewrite (or steal your P2 code)
 * to make them thread safe
 *
 */

#include <stdlib.h>
#include <types.h>
#include <stddef.h>

void *malloc(size_t __size)
{
  return NULL;
}

void *calloc(size_t __nelt, size_t __eltsize)
{
  return NULL;
}

void *realloc(void *__buf, size_t __new_size)
{
  return NULL;
}

void free(void *__buf)
{
  return;
}
