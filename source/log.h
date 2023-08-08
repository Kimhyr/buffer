#ifndef LOG_H
#define LOG_H

enum log_type
{
        LOG_TYPE_NOTE,
        LOG_TYPE_WARNING,
        LOG_TYPE_ERROR,
};

void _log(
        enum log_type type,
        const char file[],
        const char function[],
        unsigned line,
        const char format[],
        ...);

__attribute__((always_inline))
inline void _no_log(
        enum log_type type,
        const char file[],
        const char function[],
        unsigned line,
        const char format[],
        ...)
{ (void)type, (void)file, (void)function, (void)line, (void)format; }

#ifdef ENABLE_LOGGING

#define _LOG(TYPE, FORMAT, ...) \
        _log(TYPE, __FILE__, __FUNCTION__, __LINE__, FORMAT, __VA_ARGS__)

#else

#define _LOG(TYPE, FORMAT, ...) \
        _no_log(TYPE, __FILE__, __FUNCTION__, __LINE__, FORMAT, __VA_ARGS__)

#endif

#define log_note(FORMAT, ...) \
        _LOG(LOG_TYPE_NOTE, FORMAT, __VA_ARGS__)

#define log_warning(FORMAT, ...) \
        _LOG(LOG_TYPE_WARNING, FORMAT, __VA_ARGS__)

#define log_error(FORMAT, ...) \
        _LOG(LOG_TYPE_ERROR, FORMAT, __VA_ARGS__)

#endif
