#ifndef NANORESOURCE_PLATFORM_H
#define NANORESOURCE_PLATFORM_H

#if defined(_WIN32)
#  define NANORESOURCE_EXPORT __declspec(dllimport)
#elif defined(__GNUC__) && (__GNUC__ * 100 + __GNUC_MINOR) >= 303
#  define NANORESOURCE_EXPORT __attribute__((visibility("default")))
#  define NANORESOURCE_INLINE inline
#else
#  define NANORESOURCE_EXPORT
#  define NANORESOURCE_INLINE
#endif

#ifndef NANORESOURCE_ALIGNMENT
#  define NANORESOURCE_ALIGNMENT sizeof(unsigned long) // platform word
#endif

#ifndef NANORESOURCE_MAX_ENUM
#  define NANORESOURCE_MAX_ENUM 0x7FFFFFFF
#endif

#endif
