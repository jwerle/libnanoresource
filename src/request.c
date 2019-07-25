#include "nanoresource/allocator.h"
#include "nanoresource/resource.h"
#include "nanoresource/request.h"
#include "require.h"
#include <string.h>

static int
nanoresource_request_dequeue(
  struct nanoresource_request_s *request,
  struct nanoresource_s *resource,
  enum nanoresource_request_type type,
  unsigned int err
);

struct nanoresource_request_s *
nanoresource_request_alloc() {
  return nanoresource_allocator_alloc(sizeof(struct nanoresource_request_s));
}

int
nanoresource_request_init(
  struct nanoresource_request_s *request,
  const struct nanoresource_request_options_s options
) {
  require(request, EFAULT);
  require(options.resource, EINVAL);
  require(memset(request, 0, sizeof(struct nanoresource_request_s)), EFAULT);

  request->callback = nanoresource_request_callback;
  request->pending = 0;
  request->resource = options.resource;
  request->before = options.before;
  request->after = options.after;
  request->alloc = 0;
  request->type = options.type;
  request->data = options.data;
  request->done = options.callback;
  request->user = options.user;
  request->err = 0;
  return 0;
}

struct nanoresource_request_s *
nanoresource_request_new(const struct nanoresource_request_options_s options) {
  struct nanoresource_request_s *request = nanoresource_request_alloc();

  if (0 != request) {
    if (nanoresource_request_init(request, options) < 0) {
      nanoresource_allocator_free(request);
      request = 0;
    } else if (0 != request) {
      request->alloc = 1;
    }
  }

  return request;
}

void
nanoresource_request_free(struct nanoresource_request_s *request) {
  if (0 != request && 1 == request->alloc) {
    request->alloc = 0;
    nanoresource_allocator_free(request);
    request = 0;
  }
}

int
nanoresource_request_run(struct nanoresource_request_s *request) {
  require(request, EFAULT);
  require(request->resource, EFAULT);

  if (0 != request->err) {
    return nanoresource_request_callback(request, request->err);
  }

  request->resource->pending++;

  if (0 != request->before) {
    request->before(request, request->err);
  }

  if (
    NANORESOURCE_REQUEST_CLOSE == request->type ||
    NANORESOURCE_REQUEST_DESTROY == request->type
  ) {
    if (request->resource->actives > 0 || 1 == request->resource->opening) {
      return 0;
    }
  }

  memcpy(
    &(request->resource->last_request),
    request,
    sizeof(struct nanoresource_request_s));

  request->resource->last_request.data = 0;
  request->resource->last_request.done = 0;
  request->resource->last_request.after = 0;
  request->resource->last_request.before = 0;
  request->resource->last_request.callback = 0;

  switch (request->type) {
    case NANORESOURCE_REQUEST_OPEN:
      if (1 == request->resource->opened && 0 == request->resource->needs_open) {
        return nanoresource_request_callback(request, 0);
      } else if (0 != request->resource->options.open) {
        request->resource->opening = 1;
        request->resource->options.open(request);
      } else {
        request->resource->opening = 1;
        return nanoresource_request_callback(request, 0);
      }
      break;

    case NANORESOURCE_REQUEST_CLOSE:
      if (1 == request->resource->closed || 0 == request->resource->opened) {
        return nanoresource_request_callback(request, 0);
      } else if (0 != request->resource->options.close) {
        request->resource->closing = 1;
        request->resource->options.close(request);
      } else {
        request->resource->closing = 1;
        return nanoresource_request_callback(request, 0);
      }
      break;

    case NANORESOURCE_REQUEST_DESTROY:
      if (1 == request->resource->destroyed) {
        return nanoresource_request_callback(request, 0);
      } else if (0 != request->resource->options.destroy) {
        request->resource->options.destroy(request);
      } else {
        return nanoresource_request_callback(request, 0);
      }
      break;

    case NANORESOURCE_REQUEST_USER:
      if (0 != request->user) {
        request->user(request);
      } else {
        return nanoresource_request_callback(request, 0);
      }
      break;

    case NANORESOURCE_REQUEST_NONE:
      return nanoresource_request_callback(request, ENOSYS);
  }

  return 0;
}

int
nanoresource_request_dequeue(
  struct nanoresource_request_s *request,
  struct nanoresource_s *resource,
  enum nanoresource_request_type type,
  unsigned int err
) {
  require(request, EFAULT);
  require(request->resource, EFAULT);

  int needs_free = 0;

  // maybe open error?
  if (err > 0) {
    if (NANORESOURCE_REQUEST_OPEN == type) {
      for (int i = 0; i < resource->queued; ++i) {
        if (0 != resource->queue[i]) {
          resource->queue[i]->err = err;
        }
      }
    }
  } else {
    switch (type) {
      case NANORESOURCE_REQUEST_OPEN:
        if (0 == resource->opened) {
          resource->opened = 1;
          resource->needs_open = 0;
          // @TODO(jwerle): HOOK(open)
        }
        break;

      case NANORESOURCE_REQUEST_CLOSE:
        if (0 == resource->closed) {
          resource->opened = 0;
          resource->closed = 1;
          // @TODO(jwerle): HOOK(close)
        }
        break;

      case NANORESOURCE_REQUEST_DESTROY:
        if (0 == resource->destroyed) {
          resource->destroyed = 1;
          // @TODO(jwerle): HOOK(destroy)
        }
        break;

      default:
        // NOOP
        (void)(0);
    }
  }

  struct nanoresource_request_s *head = resource->queue[0];
  unsigned int queued = resource->queued;
  if (queued > 0 && head == request) {
    nanoresource_queue_shift(resource);
    needs_free = 1;
    request = 0;
    head = 0;
  }

  // drain queue
  if (resource->pending > 0u && 0u == --resource->pending) {
    while (resource->queued > 0) {
      if (0 == resource->queue[0]) {
        nanoresource_queue_shift(resource);
      }

      if (nanoresource_request_run(resource->queue[0]) < 0) {
        break;
      }

      if (type >= NANORESOURCE_REQUEST_OPEN) { // [ open, close, destroy ]
        break;
      } else {
        nanoresource_request_free(nanoresource_queue_shift(resource));
      }
    }
  }

  return needs_free;
}

int
nanoresource_request_callback(
  struct nanoresource_request_s *request,
  unsigned int err
) {
  require(request, EFAULT);
  require(request->resource, EFAULT);

  request->err = err;

  struct nanoresource_s *resource = request->resource;
  nanoresource_request_result_callback_t *after = request->after;

  unsigned int type = request->type;
  void *done = request->done;

  int needs_free = nanoresource_request_dequeue(request, resource, type, err);

  switch (type) {
    case NANORESOURCE_REQUEST_OPEN:
      resource->opening = 0;
      if (0 != done) {
        ((nanoresource_open_callback_t *)done)(resource, err);
      }
      break;

    case NANORESOURCE_REQUEST_CLOSE:
      resource->closing = 0;
      if (0 != done) {
        ((nanoresource_close_callback_t *)done)(resource, err);
      }
      break;

    case NANORESOURCE_REQUEST_DESTROY:
      resource->destroying = 0;
      if (0 != done) {
        ((nanoresource_destroy_callback_t *)done)(resource, err);
      }
      break;

    case NANORESOURCE_REQUEST_USER:
      if (0 != done) {
        ((nanoresource_user_callback_t *)done)(resource, err);
      }
      break;
  }

  if (0 != after) {
    after(request, err);
  }

  if (1 == needs_free) {
    nanoresource_request_free(request);
  }

  return err;
}
