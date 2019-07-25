#include "nanoresource/version.h"

#define SX(x) #x
#define S(x) SX(x)

#ifndef NANORESOURCE_VERSION
#define NANORESOURCE_VERSION 0
#endif

#ifndef NANORESOURCE_NAME
#define NANORESOURCE_NAME "libnanoresource"
#endif

#ifndef NANORESOURCE_VERSION_MAJOR
#define NANORESOURCE_VERSION_MAJOR 0
#endif

#ifndef NANORESOURCE_VERSION_MINOR
#define NANORESOURCE_VERSION_MINOR 0
#endif

#ifndef NANORESOURCE_VERSION_PATCH
#define NANORESOURCE_VERSION_PATCH 0
#endif

#ifndef NANORESOURCE_VERSION_REVISION
#define NANORESOURCE_VERSION_REVISION 0
#endif

#ifndef NANORESOURCE_DATE_COMPILED
#define NANORESOURCE_DATE_COMPILED ""
#endif

const char *
nanoresource_version_string() {
  return NANORESOURCE_NAME
    "@" S(NANORESOURCE_VERSION_MAJOR)
    "." S(NANORESOURCE_VERSION_MINOR)
    "." S(NANORESOURCE_VERSION_PATCH)
    "." S(NANORESOURCE_VERSION_REVISION) " (" NANORESOURCE_DATE_COMPILED ")";
}

const unsigned long int
nanoresource_version() {
  return (const unsigned long int) NANORESOURCE_VERSION;
}

const unsigned long int
nanoresource_version_major() {
  return NANORESOURCE_VERSION >> 24 & 0xff;
}

const unsigned long int
nanoresource_version_minor() {
  return NANORESOURCE_VERSION >> 16 & 0xff;
}

const unsigned long int
nanoresource_version_patch() {
  return NANORESOURCE_VERSION >> 8 & 0xff;
}

const unsigned long int
nanoresource_version_revision() {
  return NANORESOURCE_VERSION & 0xff;
}
