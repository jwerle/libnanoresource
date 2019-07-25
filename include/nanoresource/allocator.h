#ifndef NANORESOURCE_ALLOCATOR_H
#define NANORESOURCE_ALLOCATOR_H

#include "platform.h"

// Forward declarations
struct nanoresource_allocator_stats_s;

/**
 * Allocator stats
 */
struct nanoresource_allocator_stats_s {
  unsigned int alloc;
  unsigned int free;
};

/**
 * Returns allocator stats.
 */
NANORESOURCE_EXPORT const struct nanoresource_allocator_stats_s
nanoresource_allocator_stats();

/**
 * Returns number of allocations returned by `nanoresource_alloc()`.
 */
NANORESOURCE_EXPORT int
nanoresource_allocator_alloc_count();

/**
 * Returns number of deallocations by `nanoresource_free()`.
 */
NANORESOURCE_EXPORT int
nanoresource_allocator_free_count();

/**
 * Set the allocator function used in the library.
 * `malloc()=`
 */
NANORESOURCE_EXPORT void
nanoresource_allocator_set(void *(*allocator)(unsigned long int));

/**
 * Set the deallocator function used in the library.
 * `free()=`
 */
NANORESOURCE_EXPORT void
nanoresource_deallocator_set(void (*allocator)(void *));

/**
 * The allocator function used in the library.
 * Defaults to `malloc()`.
 */
NANORESOURCE_EXPORT void *
nanoresource_allocator_alloc(unsigned long int);

/**
 * The deallocator function used in the library.
 * Defaults to `free()`.
 */
NANORESOURCE_EXPORT void
nanoresource_allocator_free(void *);

#endif
