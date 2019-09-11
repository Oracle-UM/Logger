#ifndef LOG_H
#define LOG_H

// fileno(), truncate()
#define _POSIX_C_SOURCE 200809L

#include "utils/BriefIntTypes.h"

#include <stdio.h>

enum LOG_STR_LIMITS {
    LOG_FILENAME_MAX = FILENAME_MAX,
    LOG_ENTRY_MAX = U64(1024),
};

typedef struct {
    char filename[LOG_FILENAME_MAX];
    FILE* out_file;
} Log;

Log* make_log(void);

int write_to_log(Log* log, char const* fmt, ...);

int flush_log(Log* log);

int clear_log(Log* log);

int rm_log(Log* log);

// uses ##__VA_ARGS__ extension
#define WRITE_TO_LOG_DETAILED(LOG, FMT, ...)     \
    do {                                         \
        write_to_log(LOG, FMT, ##__VA_ARGS__);   \
        fprintf(                                 \
            (LOG)->out_file,                     \
            "file: %s\n"                         \
            "function %s\n"                      \
            "line: %u\n",                        \
            __FILE__,                            \
            __func__,                            \
            __LINE__);                           \
    } while (0)

#endif  // LOG_H
