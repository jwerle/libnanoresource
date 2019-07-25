#ifndef NANORESOURCE_REQUEST_H
#define NANORESOURCE_REQUEST_H

#include "platform.h"

// Forward declarations
struct nanoresource_request_s;
struct nanoresource_request_options_s;

/**
 * The `nanoresource_request_result_callback_t` callback represents the user
 * callback for the request result.
 */
typedef int (nanoresource_request_result_callback_t)(
  struct nanoresource_request_s *request,
  unsigned int err);

/**
 * The `nanoresource_request_work_callback_t` callback represents the user
 * callback for the resource operation work request to be done.
 */
typedef void (nanoresource_request_work_callback_t)(
  struct nanoresource_request_s *request);

/**
 */
enum nanoresource_request_type {
  NANORESOURCE_REQUEST_OPEN = 0,
  NANORESOURCE_REQUEST_CLOSE = 1,
  NANORESOURCE_REQUEST_DESTROY = 2,
  NANORESOURCE_REQUEST_USER = 3,
  NANORESOURCE_REQUEST_NONE = NANORESOURCE_MAX_ENUM
};

/**
 * Fields for `struct nanoresource_request_options_s` that can be used for
 * extending structures that ensure correct memory layout.
 */
#define NANORESOURCE_REQUEST_OPTIONS_FIELDS        \
  enum nanoresource_request_type type;             \
  nanoresource_request_work_callback_t *user;      \
  nanoresource_request_result_callback_t *before;  \
  nanoresource_request_result_callback_t *after;   \
  struct nanoresource_s *resource;                 \
  void *callback;                                  \
  void *data;

/**
 * Represents the initial configurable state for a resource
 * opertion request context.
 */
struct nanoresource_request_options_s {
  NANORESOURCE_REQUEST_OPTIONS_FIELDS
};

/**
 * Fields for `struct nanoresource_request_s` that can be used for
 * extending structures that ensure correct memory layout.
 */
#define NANORESOURCE_REQUEST_FIELDS                 \
  unsigned int alloc:1;                             \
  unsigned int err;                                 \
  unsigned int pending:1;                           \
  enum nanoresource_request_type type;              \
  nanoresource_request_work_callback_t *user;       \
  nanoresource_request_result_callback_t *before;   \
  nanoresource_request_result_callback_t *callback; \
  nanoresource_request_result_callback_t *after;    \
  struct nanoresource_s *resource;                  \
  void *done;                                       \
  void *data;

/**
 * Represents the state for a resource operation context.
 */
struct nanoresource_request_s {
  NANORESOURCE_REQUEST_FIELDS
};

/**
 * Allocates a pointer to 'struct nanoresource_request_s'.
 * Calls 'nanoresource_alloc()' internally and will return 'NULL'
 * on allocation errors.
 */
NANORESOURCE_EXPORT struct nanoresource_request_s *
nanoresource_request_alloc();

/**
 * Initializes a pointer to `struct nanoresource_request_s` with options from
 * `struct nanoresource_request_options_s`. Returns `0` on success, otherwise
 * an error code found in `errno.h` with its sign flipped and `errno` set.
 *
 * Possible Error Codes
 *   * `EFAULT`: The 'struct nanoresource_request_s *request' is `NULL`
 */
NANORESOURCE_EXPORT int
nanoresource_request_init(
  struct nanoresource_request_s *request,
  const struct nanoresource_request_options_s options);

/**
 * Allocates and initializes a pointer to `struct nanoresource_request_s`.
 * Returns `NULL` on error and `errno` is set to an error code found in
 * `errno.h`.
 */
NANORESOURCE_EXPORT struct nanoresource_request_s *
nanoresource_request_new(const struct nanoresource_request_options_s options);

/**
 * Frees a pointer to `struct nanoresource_request_s`.
 */
NANORESOURCE_EXPORT void
nanoresource_request_free(struct nanoresource_request_s *request);

/**
 * Runs the request for a resourceoperation.
 */
NANORESOURCE_EXPORT int
nanoresource_request_run(struct nanoresource_request_s *request);

/**
 * Handles the callback from the resource operation
 * request function given to the implementation.
 */
NANORESOURCE_EXPORT int
nanoresource_request_callback(
  struct nanoresource_request_s *request,
  unsigned int err);

#endif
