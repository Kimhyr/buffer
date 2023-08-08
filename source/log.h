#ifndef LOG_H
#define LOG_H

enum log_type
{
        LOG_TYPE_NOTE,
        LOG_TYPE_WARNING,
        LOG_TYPE_ERROR,
};

void _log(enum log_type type,
          const char    function[],
          const char    format[],
          ...);

#define _LOG(TYPE, FORMAT, ...) \
        _log(TYPE, __FUNCTION__, FORMAT, __VA_ARGS__)

#define log_note(FORMAT, ...) \
        _LOG(LOG_TYPE_NOTE, FORMAT, __VA_ARGS__)

#define log_warning(FORMAT, ...) \
        _LOG(LOG_TYPE_WARNING, FORMAT, __VA_ARGS__)

#define log_error(FORMAT, ...) \
        _LOG(LOG_TYPE_ERROR, FORMAT, __VA_ARGS__)

#endif
