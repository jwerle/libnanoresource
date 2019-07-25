#ifndef NANORESOURCE_VERSION_H
#define NANORESOURCE_VERSION_H

#include "platform.h"

/**
 * Returns the version string for the library.
 */
NANORESOURCE_EXPORT const char *
nanoresource_version_string();

/**
 * Returns the 32 bit version number that encodes the
 * major, minor, patch, and revision version parts.
 */
NANORESOURCE_EXPORT const unsigned long int
nanoresource_version();

/**
 * Returns the major version part.
 */
NANORESOURCE_EXPORT const unsigned long int
nanoresource_version_major();

/**
 * Returns the minor version part.
 */
NANORESOURCE_EXPORT const unsigned long int
nanoresource_version_minor();

/**
 * Returns the minor patch part.
 */
NANORESOURCE_EXPORT const unsigned long int
nanoresource_version_patch();

/**
 * Returns the minor revision part.
 */
NANORESOURCE_EXPORT const unsigned long int
nanoresource_version_revision();

#endif
