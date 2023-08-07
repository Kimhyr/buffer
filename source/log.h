#ifndef LOG_H
#define LOG_H

enum log_type
{
        LOG_TYPE_NOTE,
        LOG_TYPE_WARNING,
        LOG_TYPE_ERROR,
};

#ifndef NDEBUG

void _log(
        enum log_type type,
        const char file[],
        unsigned line,
        const char format[],
        ...);

#else

__attribute__((always_inline))
inline void _log(
        enum log_type type,
        const char file[],
        unsigned line,
        const char format[],
        ...)
{ (void)type, (void)file, (void)line, (void)format; }

#endif

#define _LOG(TYPE, FORMAT, ...) \
        _log(TYPE, __FILE__, __LINE__, FORMAT, __VA_ARGS__)

#define log_note(FORMAT, ...) \
        _LOG(LOG_TYPE_NOTE, FORMAT, __VA_ARGS__)

#define log_warning(FORMAT, ...) \
        _LOG(LOG_TYPE_WARNING, FORMAT, __VA_ARGS__)

#define log_error(FORMAT, ...) \
        _LOG(LOG_TYPE_ERROR, FORMAT, __VA_ARGS__)

#endif
