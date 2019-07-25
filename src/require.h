#ifndef _NANORESOURCE_REQURE_H
#define _NANORESOURCE_REQURE_H

#include <errno.h>

#define require(op, code) { \
  if (0 == (op)) {          \
    errno = code;           \
    return -errno;          \
  }                         \
}

#endif
