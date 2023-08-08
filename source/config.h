#ifndef CONFIG_H
#define CONFIG_H

#ifndef __GNUC__
#       error "Compile using GCC or Clang"
#endif

#ifndef __x86_64__
#ifndef __unix__
#       error "Compile for x86-64 UNIX"
#endif
#endif

#ifdef ENABLE_DEBUG
#       define NDEBUG
#endif

#endif
