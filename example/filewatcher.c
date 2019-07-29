#include <nanoresource/nanoresource.h>
#include <sys/stat.h>
#include <pthread.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>

#define MAX_THREADS 16
#define SHARED_MEMORY_BYTES 64

typedef struct thread_s thread_t;
typedef struct thread_context_s thread_context_t;
typedef struct watch_context_s watch_context_t;

typedef void *(pthread_start_routine_t)(void *);
typedef void (watch_callback_t)(nanoresource_t *resource, int err);
typedef void (watch_change_callback_t)(
  nanoresource_t *resource,
  const char *filename,
  const struct stat *stats);

struct thread_s {
  pthread_t id;
  pthread_attr_t attr;
  unsigned int index;
  nanoresource_request_t *request;
};

struct thread_context_s {
  thread_t **threads;
  unsigned int count;
};

struct watch_context_s {
  const char *filename;
  watch_change_callback_t *onchange;
};

static void
forward(nanoresource_request_t *request, pthread_start_routine_t *start) {
  thread_context_t *context = (thread_context_t *) request->resource->data;
  thread_t **threads = context->threads;
  thread_t *thread = malloc(sizeof(*thread));

  *thread = (thread_t){ 0 };
  thread->index = context->count;
  thread->request = request;
  context->threads[context->count++] = thread;

  pthread_attr_init(&thread->attr);
  pthread_create(&thread->id, &thread->attr, start, thread);
}

static void *
open_resource_thread(void *arg) {
  thread_t *thread = (thread_t *) arg;
  nanoresource_request_t *request = thread->request;
  thread_context_t *context = (thread_context_t *) request->resource->data;

  printf("opening...\n");
  request->callback(request, 0);

  pthread_exit(0);
  return 0;
}

static void *
close_resource_thread(void *arg) {
  thread_t *thread = (thread_t *) arg;
  nanoresource_request_t *request = thread->request;
  thread_context_t *context = (thread_context_t *) request->resource->data;

  printf("closing...\n");
  request->callback(request, 0);

  pthread_exit(0);
  return 0;
}

static void *
destroy_resource_thread(void *arg) {
  thread_t *thread = (thread_t *) arg;
  nanoresource_request_t *request = thread->request;
  thread_context_t *context = (thread_context_t *) request->resource->data;

  printf("destroying...\n");
  request->callback(request, 0);
  pthread_exit(0);
  return 0;
}

static void *
watch_resource_thread(void *arg) {
  thread_t *thread = (thread_t *) arg;
  nanoresource_request_t *request = thread->request;
  thread_context_t *context = (thread_context_t *) request->resource->data;

  watch_context_t *ctx = request->data;
  const char *filename = ctx->filename;
  unsigned int changes = 0;
  unsigned int max = 2;
  time_t now = time(0);

  printf("watching... %s\n", ctx->filename);
  while (changes < max) {
    sleep(1);

    struct stat stats = { 0 };
    stat(filename, &stats);

    if (stats.st_mtim.tv_sec > now) {
      (void) changes++;
      now = time(0);
      ctx->onchange(request->resource, ctx->filename, &stats);
    }
  }


  request->callback(request, 0);
  pthread_exit(0);
  return 0;
}

static void
open_resource(nanoresource_request_t *request) {
  forward(request, open_resource_thread);
}

static void
close_resource(nanoresource_request_t *request) {
  forward(request, close_resource_thread);
}

static void
destroy_resource(nanoresource_request_t *request) {
  forward(request, destroy_resource_thread);
}

static void
watch_resource(nanoresource_request_t *request) {
  forward(request, watch_resource_thread);
}

static int
watch_after(
  struct nanoresource_request_s *request,
  unsigned int err
) {
  free(request->data);
  free(request);
  return 0;
}

static void
watch(
  nanoresource_t *resource,
  const char *filename,
  watch_callback_t *callback,
  watch_change_callback_t *onchange
) {
  watch_context_t *ctx = malloc(sizeof(*ctx));
  *ctx = (watch_context_t){ filename, onchange };

  nanoresource_request_t *request = nanoresource_request_new(
    (nanoresource_request_options_t) {
      .callback = callback,
      .resource = resource,
      .type = NANORESOURCE_REQUEST_USER,
      .user = watch_resource,
      .after = watch_after,
      .data = (void *) ctx
    });

  nanoresource_request_run(request);
}

static void
onclose(nanoresource_t *resource, int err) {
  printf("onclose(err=%d)\n", err);
}

static void
ondestroy(nanoresource_t *resource, int err) {
  printf("ondestroy(err=%d)\n", err);
}

static void
onwatch(nanoresource_t *resource, int err) {
  printf("onwatch(err=%d)\n", err);
  nanoresource_destroy(resource, ondestroy);
}

static void
onchange(
  nanoresource_t *resource,
  const char *filename,
  const struct stat *stats
) {
  printf("change!\n");
}

static void
onopen(nanoresource_t *resource, int err) {
  printf("onopen(err=%d)\n", err);
  watch(resource, "Makefile", onwatch, onchange);
}

int
main(void) {
  thread_context_t context = { 0 };
  nanoresource_t resource = { 0 };

  context.threads = malloc(MAX_THREADS * sizeof(context.threads));
  memset(context.threads, 0, sizeof(*context.threads));

  nanoresource_init(&resource,
    (nanoresource_options_t) {
      .data = &context,
      .open = open_resource,
      .close = close_resource,
      .destroy = destroy_resource
    });

  const char *buffer = "hello";
  nanoresource_open(&resource, onopen);

  for (int i = 0; i < context.count; ++i) {
    thread_t *thread = context.threads[i];
    pthread_join(thread->id, 0);
    context.threads[i] = 0;
    free(thread);
  }


  free(context.threads);
  return 0;
}
