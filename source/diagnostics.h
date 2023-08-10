#ifndef DEBUG_H
#define DEBUG_H

#include <chrono>
#include <format>
#include <iostream>

#include "utilities.h"

enum class record_type
{
    note    = 36,
    warning = 33,
    failure = 31,
};

void _record(std::string_view file,
             std::size_t      line,
             record_type      type,
             std::string_view message);

template<bool Assertion>
void _assert(std::string_view file,
             std::size_t      line,
             std::string_view assertion)
{
    if constexpr(!Assertion) {
        _record(file, line, record_type::failure, assertion);
        std::abort();
    }

    _record(file, line, record_type::note, assertion);
}

#define IGNORE static_cast<void>(0)

#if BUILD_MODE == DEBUG

#define RECORD(T, ...) _record(__FILE__, __LINE__, T, std::format(__VA_ARGS__))

#ifdef assert
#   error `assert` is already defined
#endif
#define assert(E) _assert<E>(__FILE__, __LINE__, "Assertion failed: `" #E "`" )

#else

#define RECORD(E) IGNORE
#define assert(E) IGNORE

#endif  // BUILD_MODE == DEBUG

#define record_note(...)    RECORD(record_type::note, __VA_ARGS__);
#define record_warning(...) RECORD(record_type::warning, __VA_ARGS__);
#define record_failure(...) RECORD(record_type::failure, __VA_ARGS__);

#endif  // DEBUG_H
