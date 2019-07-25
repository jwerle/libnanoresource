#include <nanoresource/nanoresource.h>
#include <stdio.h>
#include <ok/ok.h>

#ifndef OK_EXPECTED
#define OK_EXPECTED 0
#endif

static void
open(struct nanoresource_request_s *request) {
  ok("open()");
  request->callback(request, 0);
}

static void
close(struct nanoresource_request_s *request) {
  ok("close()");
  request->callback(request, 0);
}

static void
destroy(struct nanoresource_request_s *request) {
  ok("destroy()");
  request->callback(request, 0);
}

static void
onopen(struct nanoresource_s *resource, int err) {
  ok("onopen()");
}

static void
onclose(struct nanoresource_s *resource, int err) {
  ok("onclose()");
}

static void
ondestroy(struct nanoresource_s *resource, int err) {
  ok("ondestroy()");
}

int
main(void) {
  printf("### ok: expecting %d\n", OK_EXPECTED);
  ok_expect(OK_EXPECTED);

  struct nanoresource_s *resource = nanoresource_new(
    (struct nanoresource_options_s) {
      .open = open,
      .close = close,
      .destroy = destroy,
    });

  if (0 != resource) {
    ok("0 != resource");
  }

  if (0 == nanoresource_open(resource, onopen)) {
    ok("nanoresource_open()");
  }

  if (0 == nanoresource_active(resource)) {
    ok("nanoresource_active()");
  }

  if (0 == nanoresource_close(resource, onclose)) {
    ok("nanoresource_close()");
  }

  if (0 == nanoresource_destroy(resource, ondestroy)) {
    ok("nanoresource_destroy()");
  }

  if (3 == nanoresource_inactive(resource)) { // close, [close<-], destroy
    ok("nanoresource_inactive()");
  }

  const struct nanoresource_allocator_stats_s stats = nanoresource_allocator_stats();
  //printf("alloc=%d free=%d\n", stats.alloc, stats.free);
  if (stats.alloc == stats.free) {
    ok("stats.alloc == stats.free");
  }

  printf("%s\n", nanoresource_version_string());
  ok_done();
  return ok_expected() - ok_count();
}
