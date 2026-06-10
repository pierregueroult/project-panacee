#ifndef MEMORY_H
#define MEMORY_H

/**
 * @file memory.h
 * @brief Allocation helpers that print an error and exit on out-of-memory.
 */

#include <stddef.h>

/**
 * @brief malloc() that prints an error and exits on failure.
 * @param size Number of bytes to allocate.
 * @return Pointer to the allocated memory; never NULL.
 */
void *xmalloc(size_t size);

/**
 * @brief calloc() that prints an error and exits on failure.
 * @param count Number of elements.
 * @param size Size of one element in bytes.
 * @return Pointer to the zero-initialised memory; never NULL.
 */
void *xcalloc(size_t count, size_t size);

#endif
