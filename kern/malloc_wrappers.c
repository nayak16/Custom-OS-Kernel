#include <stddef.h>
#include <malloc.h>

/* safe versions of malloc functions */
void *malloc(size_t size)
{
  return NULL;
}

void *memalign(size_t alignment, size_t size)
{
  return NULL;
}

void *calloc(size_t nelt, size_t eltsize)
{
  return NULL;
}

void *realloc(void *buf, size_t new_size)
{
  return NULL;
}

void free(void *buf)
{
  return;
}

void *smalloc(size_t size)
{
  return NULL;
}

void *smemalign(size_t alignment, size_t size)
{
  return NULL;
}

void sfree(void *buf, size_t size)
{
  return;
}


