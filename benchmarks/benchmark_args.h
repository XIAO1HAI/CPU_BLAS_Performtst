#ifndef BENCHMARK_ARGS_H
#define BENCHMARK_ARGS_H

#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static inline uint64_t bench_u64(const char *text, uint64_t minimum, uint64_t maximum) {
    char *end;
    errno = 0;
    unsigned long long value = text ? strtoull(text, &end, 10) : 0;
    if (!text || text[0] < '0' || text[0] > '9' || errno || *end ||
        value < minimum || value > maximum) {
        fprintf(stderr, "Invalid integer: %s\n", text ? text : "(missing)");
        exit(1);
    }
    return (uint64_t)value;
}

static inline int bench_int(const char *text, int minimum) {
    return (int)bench_u64(text, (uint64_t)minimum, INT_MAX);
}

static inline size_t bench_elements(int rows, int columns) {
    if (rows <= 0 || columns <= 0 ||
        (size_t)rows > SIZE_MAX / sizeof(double) / (size_t)columns) {
        fprintf(stderr, "Matrix dimensions exceed the allocation range\n");
        exit(1);
    }
    return (size_t)rows * (size_t)columns;
}

static inline void *bench_aligned_alloc(size_t bytes) {
    void *pointer = NULL;
    if (posix_memalign(&pointer, 64, bytes) != 0) return NULL;
    return pointer;
}

#endif
