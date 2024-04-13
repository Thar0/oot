#ifndef UTIL_H_
#define UTIL_H_

#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <dirent.h>

// Attribute macros

#define ALWAYS_INLINE inline __attribute__((always_inline))

#define NORETURN __attribute__((noreturn))
#define UNUSED   __attribute__((unused))

// Helper macros

#define strequ(s1, s2) ((__builtin_constant_p(s2) ? strncmp(s1, s2, sizeof(s2) - 1) : strcmp(s1, s2)) == 0)

#define LL_FOREACH(type, x, base) for (type(x) = (base); (x) != NULL; (x) = (x)->next)

#define ARRAY_COUNT(arr) (sizeof(arr) / sizeof((arr)[0]))

#define ALIGN16(x) (((x) + 0xF) & ~0xF)

#define BSWAP16(x)                  \
    do {                            \
        (x) = __builtin_bswap16(x); \
    } while (0)
#define BSWAP32(x)                  \
    do {                            \
        (x) = __builtin_bswap32(x); \
    } while (0)

#define BOOL_STR(b) ((b) ? "true" : "false")

// util.c functions

__attribute__((format(printf, 1, 2))) NORETURN void
error(const char *fmt, ...);
__attribute__((format(printf, 1, 2))) void
warning(const char *fmt, ...);

void *
util_read_whole_file(const char *filename, size_t *size_out);
void
util_write_whole_file(const char *filename, const void *data, size_t size);

bool
isdir(struct dirent *dp);

char *
path_join(const char *root, const char *f);

void
dir_walk_rec(const char *root, void (*callback)(const char *, void *), void *udata);

bool
str_is_c_identifier(const char *str);

#endif
