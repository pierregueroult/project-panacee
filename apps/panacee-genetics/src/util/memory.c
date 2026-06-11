/**
 * @file memory.c
 * @brief Implementation of the allocation helpers (see memory.h).
 */

#include "memory.h"

#include <stdio.h>
#include <stdlib.h>

void *xmalloc(size_t size)
{
    void *ptr = malloc(size);
    if (!ptr)
    {
        fprintf(stderr, "fatal: out of memory (malloc of %lu bytes)\n", (unsigned long)size);
        exit(EXIT_FAILURE);
    }
    return ptr;
}

void *xcalloc(size_t count, size_t size)
{
    void *ptr = calloc(count, size);
    if (!ptr)
    {
        fprintf(stderr, "fatal: out of memory (calloc of %lu x %lu bytes)\n", (unsigned long)count,
                (unsigned long)size);
        exit(EXIT_FAILURE);
    }
    return ptr;
}
