#include "Log.h"

#include <sys/stat.h>
#include <unistd.h>

#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define LOG_DIR "logs/"

Log* make_log(void) {
    if (mkdir(LOG_DIR, S_IRWXU | S_IRWXG | S_IRWXO) == -1 && errno != EEXIST) {
        return NULL;
    }

    Log* const log = malloc(sizeof(Log));
    if (!log) {
        return NULL;
    }

    strncpy(log->filename, LOG_DIR, LOG_FILENAME_MAX);

    char* const filename_it = log->filename + sizeof LOG_DIR - 1UL;  // null byte
    struct tm time_info;
    if (!strftime(
        filename_it,
        LOG_FILENAME_MAX - sizeof LOG_DIR - 1UL,
        "%Y-%b-%d-%T.log",
        localtime_r(&(time_t){ time(NULL) }, &time_info)))
    {
        free(log);
        return NULL;
    }

    if (access(log->filename, F_OK) != -1
     && truncate(log->filename, 0L) == -1) {
        // file exists and couldn't be cleared
        free(log);
        return NULL;
    }

    log->out_file = fopen(log->filename, "a");
    if (!log->out_file) {
        free(log);
        return NULL;
    }

    fprintf(log->out_file, "Start of log %s\n", filename_it);
    return log;
}

int write_to_log(Log* log, char const* fmt, ...) {
    char entry_buf[LOG_ENTRY_MAX];
    char* entry_buf_it = entry_buf;
    size_t available_size = LOG_ENTRY_MAX;

    *entry_buf_it++ = '\n';

    struct tm time_info;
    entry_buf_it += strftime(
        entry_buf,
        available_size,
        "[%Y-%b-%d %T]: ",
        localtime_r(&((time_t){ time(NULL) }), &time_info));

    if (entry_buf_it == entry_buf + 1UL) {
        //  no characters were written besides '\n'
        return EXIT_FAILURE;
    }

    va_list args;
    va_start(args, fmt);
    int const written_bytes =
        vsnprintf(entry_buf_it, available_size, fmt, args);
    va_end(args);

    if (written_bytes < 0 || written_bytes + 1 > available_size) {
        return EXIT_FAILURE;
    }

    entry_buf_it += written_bytes;
    *entry_buf_it = '\n';

    size_t const total_written_bytes = entry_buf_it - entry_buf;
    if (fwrite(entry_buf, sizeof(char), total_written_bytes, log->out_file)
      != total_written_bytes) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int flush_log(Log* log) {
    return fflush(log->out_file);
}

int clear_log(Log* log) {
    if (fflush(log->out_file) == EOF
     || ftruncate(fileno(log->out_file), 0L) == -1) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int rm_log(Log* log) {
    if (fclose(log->out_file) == EOF) {
        free(log);
        return EXIT_FAILURE;
    }

    free(log);
    return EXIT_SUCCESS;
}
