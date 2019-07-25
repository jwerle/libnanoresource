#include "nanoresource/allocator.h"
#include <stdlib.h>

#ifndef NANORESOURCE_ALLOCATOR_ALLOC
#define NANORESOURCE_ALLOCATOR_ALLOC 0
#endif

#ifndef NANORESOURCE_ALLOCATOR_FREE
#define NANORESOURCE_ALLOCATOR_FREE 0
#endif

static void *(*alloc)(unsigned long int) = NANORESOURCE_ALLOCATOR_ALLOC;
static void (*dealloc)(void *) = NANORESOURCE_ALLOCATOR_FREE;

static struct nanoresource_allocator_stats_s stats = { 0 };

const struct nanoresource_allocator_stats_s
nanoresource_allocator_stats() {
  return (struct nanoresource_allocator_stats_s) {
    .alloc = stats.alloc,
    .free = stats.free
  };
}

int
nanoresource_allocator_alloc_count() {
  return stats.alloc;
}

int
nanoresource_allocator_free_count() {
  return stats.free;
}

void
nanoresource_allocator_set(void *(*allocator)(unsigned long int)) {
  alloc = allocator;
}

void
nanoresource_deallocator_set(void (*deallocator)(void *)) {
  dealloc = deallocator;
}

void *
nanoresource_allocator_alloc(unsigned long int size) {
  if (0 == size) {
    return 0;
  } else if (0 != alloc) {
    (void) stats.alloc++;
    return alloc(size);
  } else {
    (void) stats.alloc++;
    return malloc(size);
  }
}

void
nanoresource_allocator_free(void *ptr) {
  if (0 == ptr) {
    return;
  } else if (0 != dealloc) {
    (void) stats.free++;
    dealloc(ptr);
  } else {
    (void) stats.free++;
    free(ptr);
  }
}
