#include "nanoresource/allocator.h"
#include "nanoresource/resource.h"
#include "require.h"
#include <string.h>
#include <stdlib.h>
#include <errno.h>

static int
queue_and_run(
  struct nanoresource_s *resource,
  struct nanoresource_request_s *request
) {
  nanoresource_queue_push(resource, request);

  if (0 == resource->pending) {
    return - nanoresource_request_run(request);
  } else {
    return - request->err;
  }
}

struct nanoresource_s *
nanoresource_alloc() {
  return nanoresource_allocator_alloc(sizeof(struct nanoresource_s));
}

int
nanoresource_init(
  struct nanoresource_s *resource,
  struct nanoresource_options_s options
) {
  require(resource, EFAULT);
  require(memset(resource, 0, sizeof(struct nanoresource_s)), EFAULT);
  require(memcpy(&resource->options, &options, sizeof(struct nanoresource_options_s)), EFAULT);

  resource->needs_open = 1;
  resource->data = options.data;
  return 0;
}

struct nanoresource_s *
nanoresource_new(struct nanoresource_options_s options) {
  struct nanoresource_s *resource = nanoresource_alloc();
  if (nanoresource_init(resource, options) < 0) {
    nanoresource_allocator_free(resource);
    resource = 0;
  } else {
    resource->alloc = 1;
  }
  return resource;
}

void
nanoresource_free(struct nanoresource_s *resource) {
  if (0 != resource && 1 == resource->alloc) {
    nanoresource_allocator_free(resource);
  }
}

static int
nanoresource_destroy_after(
  struct nanoresource_request_s *request,
  unsigned int err
) {
  if (
    0 != request->resource
  )  {
    nanoresource_free(request->resource);
    request->resource = 0;
  }
  return 0;
}

int
nanoresource_destroy(
  struct nanoresource_s *resource,
  nanoresource_destroy_callback_t *callback
) {
  require(resource, EFAULT);
  require(0 == nanoresource_close(resource, 0), errno);

  struct nanoresource_request_s *request = nanoresource_request_new(
    (struct nanoresource_request_options_s) {
      .callback = callback,
      .resource = resource,
      .after = nanoresource_destroy_after,
      .type = NANORESOURCE_REQUEST_DESTROY,
      .data = 0
    });

  require(request, EFAULT);
  return queue_and_run(resource, request);
}

int
nanoresource_open(
  struct nanoresource_s *resource,
  nanoresource_open_callback_t *callback
) {
  require(resource, EFAULT);

  struct nanoresource_request_s *request = nanoresource_request_new(
    (struct nanoresource_request_options_s) {
      .callback = callback,
      .resource = resource,
      .type = NANORESOURCE_REQUEST_OPEN,
      .data = 0,
    });

  require(request, EFAULT);
  return queue_and_run(resource, request);
}

int
nanoresource_close(
  struct nanoresource_s *resource,
  nanoresource_close_callback_t *callback
) {
  require(resource, EFAULT);

  struct nanoresource_request_s *request = nanoresource_request_new(
    (struct nanoresource_request_options_s) {
      .callback = callback,
      .resource = resource,
      .type = NANORESOURCE_REQUEST_CLOSE,
      .data = 0,
    });

  require(request, EFAULT);
  return queue_and_run(resource, request);
}

struct nanoresource_request_s *
nanoresource_queue_shift(struct nanoresource_s *resource) {
  struct nanoresource_request_s *head = 0;

  if (0 == resource) {
    return 0;
  }

  if (resource->queued > 0) {
    head = resource->queue[0];
  }

  // shift
  for (int i = 1; i < resource->queued; ++i) {
    resource->queue[i - 1] = resource->queue[i];
  }

  if (resource->queued > 0u) {
    (void) --resource->queued;
  }

  return head;
}

int
nanoresource_queue_push(
  struct nanoresource_s *resource,
  struct nanoresource_request_s *request
) {
  require(resource, EFAULT);
  require(request, EFAULT);
  require(resource == request->resource, EINVAL);

  if ((int) resource->queued < 0) {
    resource->queued = 0;
  }

  // push
  resource->queue[resource->queued++] = request;
  request->pending = 1;
  return resource->queued;
}

int
nanoresource_active(struct nanoresource_s *resource) {
  require(resource, EFAULT);

  if (1 == resource->closing) {
    return EAGAIN;
  }

  if (1 == resource->closed) {
    return ENOLCK;
  }

  resource->actives++;
  return 0;
}

int
nanoresource_inactive(struct nanoresource_s *resource) {
  require(resource, EFAULT);
  int released = 0;

  if (resource->actives > 0u && 0u == --resource->actives) {
    int queued = resource->queued;
    while (queued-- > 0) {
      struct nanoresource_request_s *request = resource->queue[0];
      if (
        NANORESOURCE_REQUEST_CLOSE == request->type ||
        NANORESOURCE_REQUEST_DESTROY == request->type
      ) {
        nanoresource_request_run(request);
        (void) released++;
      }
    }
  }

  return released;
}
