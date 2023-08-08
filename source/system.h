#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdint.h>

struct system_information
{
        uint64_t opened_files_limit;
};

extern struct system_information system_information;

#endif
