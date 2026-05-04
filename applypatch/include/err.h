#ifndef IMGPATCHTOOLS_ERR_H
#define IMGPATCHTOOLS_ERR_H

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static inline void err(int eval, const char *fmt, ...) {
    int saved_errno = errno;
    fprintf(stderr, "error: ");
    if (fmt) {
        va_list ap;
        va_start(ap, fmt);
        vfprintf(stderr, fmt, ap);
        va_end(ap);
        fprintf(stderr, ": ");
    }
    fprintf(stderr, "%s\n", strerror(saved_errno));
    exit(eval);
}

static inline void errx(int eval, const char *fmt, ...) {
    fprintf(stderr, "error: ");
    if (fmt) {
        va_list ap;
        va_start(ap, fmt);
        vfprintf(stderr, fmt, ap);
        va_end(ap);
    }
    fprintf(stderr, "\n");
    exit(eval);
}

#endif
