#ifndef CONFIG_H
#define CONFIG_H

// #if !defined(__GNUC__) || !defined(__clang__)
// #       error "Compile using GCC or Clang"
// #endif

#if !defined(__x86_64__) && !defined(__unix__)
#       error "Compile for x86-64 UNIX"
#endif

#ifdef ENABLE_DEBUG
#       define NDEBUG
#endif

#endif
