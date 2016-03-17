#include <stddef.h>
#include <malloc.h>
#include <malloc_internal.h>
#include <stdlib.h>
#include <kern_internals.h>

/* safe versions of malloc functions */
void *malloc(size_t size)
{

    mutex_lock(&heap_lock);
    void *ptr = _malloc(size);
    mutex_unlock(&heap_lock);
    return ptr;
}

void *memalign(size_t alignment, size_t size)
{

    mutex_lock(&heap_lock);
    void *ptr = _memalign(alignment, size);
    mutex_unlock(&heap_lock);
    return ptr;
}

void *calloc(size_t nelt, size_t eltsize)
{
    mutex_lock(&heap_lock);
    void *ptr = _calloc(nelt, eltsize);
    mutex_unlock(&heap_lock);
    return ptr;
}

void *realloc(void *buf, size_t new_size)
{
    mutex_lock(&heap_lock);
    void *ptr = _realloc(buf, new_size);
    mutex_unlock(&heap_lock);
    return ptr;
}

void free(void *buf)
{
    mutex_lock(&heap_lock);
    _free(buf);
    mutex_unlock(&heap_lock);
    return;
}

void *smalloc(size_t size)
{
    mutex_lock(&heap_lock);
    void *ptr = _smalloc(size);
    mutex_unlock(&heap_lock);
    return ptr;
}

void *smemalign(size_t alignment, size_t size)
{
    mutex_lock(&heap_lock);
    void *ptr = _smemalign(alignment, size);
    mutex_unlock(&heap_lock);
    return ptr;
}

void sfree(void *buf, size_t size)
{
    mutex_lock(&heap_lock);
    _sfree(buf, size);
    mutex_unlock(&heap_lock);
    return;
}


