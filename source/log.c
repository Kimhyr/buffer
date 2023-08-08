#ifndef NDEBUG

#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#endif

#include "log.h"

#define LOG_NOTE_SUFFIX    "\x1b[1;36mN"
#define LOG_WARNING_SUFFIX "\x1b[1;33mW"
#define LOG_ERROR_SUFFIX   "\x1b[1;31mE"

// NOTE
// WARNING
// FIXME:

#ifndef NDEBUG

void _log(
        enum log_type type,
        const char file[],
        const char function[],
        unsigned line,
        const char format[],
        ...)
{
        time_t raw_time;
        (void)time(&raw_time);
        struct tm* time_info = localtime(&raw_time);
        char buf[64];
        unsigned len = strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S ", time_info);
        fwrite(buf, 1, len, stderr);

        const char* suffix;
        unsigned suffix_length;
        switch (type) {
        case LOG_TYPE_NOTE:
                suffix = LOG_NOTE_SUFFIX;
                suffix_length = sizeof(LOG_NOTE_SUFFIX);
                break;
        case LOG_TYPE_WARNING:
                suffix = LOG_WARNING_SUFFIX;
                suffix_length = sizeof(LOG_WARNING_SUFFIX);
                break;
        case LOG_TYPE_ERROR:
                suffix = LOG_ERROR_SUFFIX;
                suffix_length = sizeof(LOG_ERROR_SUFFIX);
                break;
        }
        fwrite(suffix, 1, suffix_length, stderr);
        fprintf(stderr, "\x1b[0m %s:%s:%u ", file, function, line);
        va_list args;
        va_start(args, format);
        vfprintf(stderr, format, args);
        va_end(args);
        fputc('\n', stderr);
        fflush(stderr);
}

#endif
