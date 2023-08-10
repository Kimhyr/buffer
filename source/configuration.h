#ifndef CONFIG_H
#define CONFIG_H

#ifndef __GNUC__
#   error Compile using GCC or Clang
#endif

#ifndef __x86_64__
#   ifndef __unix__
#      error Compile for x86-64 UNIX
#   endif
#endif

#ifndef BUILD_MODE
#   error `BUILD_MODE` is not defined
#endif

#define DEBUG   0
#define RELEASE 1
#if BUILD_MODE == DEBUG
#elif BUILD_MODE == RELEASE
#   define NDEBUG
#else
#   error Invalid value for `BUILD_MODE`
#endif

#endif  // CONFIG_H
