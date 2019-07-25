#ifndef NANORESOURCE_RESOURCE_H
#define NANORESOURCE_RESOURCE_H

#include "platform.h"
#include "request.h"

// Forward declarations
struct nanoresource_s;
struct nanoresource_options_s;
struct nanoresource_request_s;

/**
 * The maximum queued requests.
 */
#ifndef NANORESOURCE_MAX_REQUEST_QUEUE
#define NANORESOURCE_MAX_REQUEST_QUEUE 512
#endif

/**
 * The `nanoresource_open_callback_t` callback represents the user callback
 * for a resource operation open request.
 */
typedef void (nanoresource_open_callback_t)(
  struct nanoresource_s *resource,
  int err);

/**
 * The `resource_close_callback_t` callback represents the user callback
 * for a resource operation close request.
 */
typedef void (nanoresource_close_callback_t)(
  struct nanoresource_s *resource,
  int err);

/**
 * The `resource_destroy_callback_t` callback represents the user callback
 * for a resource operation destroy request.
 */
typedef void (nanoresource_destroy_callback_t)(
  struct nanoresource_s *resource,
  int err);

/**
 * The `resource_inactive_callback_t` callback represents the user callback
 * for a inactive call.
 */
typedef void (nanoresource_inactive_callback_t)(
  struct nanoresource_s *resource,
  int err,
  void *value);

typedef void (nanoresource_user_callback_t)(
  struct nanoresource_s *resource,
  int err);

/**
 */
#define NANORESOURCE_OPTIONS_FIELDS              \
  nanoresource_request_work_callback_t *open;    \
  nanoresource_request_work_callback_t *close;   \
  nanoresource_request_work_callback_t *destroy; \
  void *data;


/**
 */
#define NANORESOURCE_FIELDS                                         \
  unsigned int alloc:1;                                         \
  unsigned int sync:1;                                          \
  unsigned int queued;                                          \
  unsigned int pending;                                         \
  unsigned int actives;                                         \
  unsigned int opened:1;                                        \
  unsigned int closed:1;                                        \
  unsigned int opening:1;                                       \
  unsigned int closing:1;                                       \
  unsigned int destroyed:1;                                     \
  unsigned int destroying:1;                                    \
  unsigned int needs_open:1;                                    \
  unsigned int fast_close:1;                                    \
  struct nanoresource_request_s last_request;                       \
  struct nanoresource_request_s *queue[NANORESOURCE_MAX_REQUEST_QUEUE]; \
  struct nanoresource_options_s options;                            \
  void *data;                                                   \



/**
 */
struct nanoresource_options_s {
  NANORESOURCE_OPTIONS_FIELDS
};

/**
 */
struct nanoresource_s {
  NANORESOURCE_FIELDS
};

NANORESOURCE_EXPORT struct nanoresource_s *
nanoresource_alloc();

/**
 */
NANORESOURCE_EXPORT struct nanoresource_s *
nanoresource_new(const struct nanoresource_options_s options);

/**
 */
NANORESOURCE_EXPORT int
nanoresource_init(
  struct nanoresource_s *resource,
  const struct nanoresource_options_s options);

/**
 */
NANORESOURCE_EXPORT void
nanoresource_free(struct nanoresource_s *resource);

/**
 */
NANORESOURCE_EXPORT int
nanoresource_open(
  struct nanoresource_s *resource,
  nanoresource_open_callback_t *callback);

/**
 */
NANORESOURCE_EXPORT int
nanoresource_close(
  struct nanoresource_s *resource,
  nanoresource_close_callback_t *callback);

/**
 */
NANORESOURCE_EXPORT int
nanoresource_destroy(
  struct nanoresource_s *resource,
  nanoresource_destroy_callback_t *callback);

/**
 * Returns the head of the queue pointing to a `struct nanoresource_request_s`
 * type and shifts the remaining elements over to the left by 1.
 */
NANORESOURCE_EXPORT struct nanoresource_request_s *
nanoresource_queue_shift(struct nanoresource_s *resource);

/**
 * Pushes a `struct nanoresource_request_s` pointer on to the queue returning
 * the new queue length.
 */
NANORESOURCE_EXPORT int
nanoresource_queue_push(
  struct nanoresource_s *resource,
  struct nanoresource_request_s *request);

/**
 */
NANORESOURCE_EXPORT int
nanoresource_active(struct nanoresource_s *resource);

/**
 */
NANORESOURCE_EXPORT int
nanoresource_inactive(struct nanoresource_s *resource);

#endif
