
#ifndef FAST_EXPORT_H
#define FAST_EXPORT_H

#ifdef FAST_STATIC_DEFINE
#  define FAST_EXPORT
#  define FAST_NO_EXPORT
#else
#  ifndef FAST_EXPORT
#    ifdef FAST_EXPORTS
        /* We are building this library */
#      define FAST_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define FAST_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef FAST_NO_EXPORT
#    define FAST_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef FAST_DEPRECATED
#  define FAST_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef FAST_DEPRECATED_EXPORT
#  define FAST_DEPRECATED_EXPORT FAST_EXPORT FAST_DEPRECATED
#endif

#ifndef FAST_DEPRECATED_NO_EXPORT
#  define FAST_DEPRECATED_NO_EXPORT FAST_NO_EXPORT FAST_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef FAST_NO_DEPRECATED
#    define FAST_NO_DEPRECATED
#  endif
#endif

#endif /* FAST_EXPORT_H */
