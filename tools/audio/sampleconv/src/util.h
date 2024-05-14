/**
 * SPDX-FileCopyrightText: Copyright (C) 2024 ZeldaRET
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>

#define ARRAY_COUNT(arr) (sizeof(arr) / sizeof((arr)[0]))

#define NORETURN __attribute__((noreturn))
#define UNUSED   __attribute__((unused))

#define strequ(s1, s2) ((__builtin_constant_p(s2) ? strncmp(s1, s2, sizeof(s2) - 1) : strcmp(s1, s2)) == 0)

#define MALLOC_CHECKED(length)                                                                         \
    ({                                                                                                 \
        size_t malloc_len_ = (size_t)(length);                                                         \
        void *result_ = malloc(malloc_len_);                                                           \
        if (result_ == NULL)                                                                           \
            error("[malloc] Failed to allocate %lu bytes @ [%s:%u]", malloc_len_, __FILE__, __LINE__); \
        result_;                                                                                       \
    })

#define MALLOC_CHECKED_INFO(length, fmt, ...)                                                                   \
    ({                                                                                                          \
        size_t malloc_len_ = (size_t)(length);                                                                  \
        void *result_ = malloc(malloc_len_);                                                                    \
        if (result_ == NULL)                                                                                    \
            error("[malloc] Failed to allocate %lu bytes @ [%s:%u] (" fmt ")", malloc_len_, __FILE__, __LINE__, \
                  ##__VA_ARGS__);                                                                               \
        result_;                                                                                                \
    })

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

#define ABS(x) (((x) < 0) ? (-(x)) : (x))

#define LITTLE_ENDIAN_STRUCT __attribute__((scalar_storage_order("little-endian"))) struct
#define BIG_ENDIAN_STRUCT    __attribute__((scalar_storage_order("big-endian"))) struct

typedef LITTLE_ENDIAN_STRUCT
{
    uint32_t value;
}
uint32_t_LE;

typedef LITTLE_ENDIAN_STRUCT
{
    uint16_t value;
}
uint16_t_LE;

typedef LITTLE_ENDIAN_STRUCT
{
    int16_t value;
}
int16_t_LE;

typedef BIG_ENDIAN_STRUCT
{
    uint32_t value;
}
uint32_t_BE;

typedef BIG_ENDIAN_STRUCT
{
    uint16_t value;
}
uint16_t_BE;

typedef BIG_ENDIAN_STRUCT
{
    int16_t value;
}
int16_t_BE;

#define FWRITE(file, data, size)                                                                    \
    do {                                                                                            \
        if (fwrite((data), (size), 1, (file)) != 1) {                                               \
            error("[%s:%d] Could not write %lu bytes to file", __FILE__, __LINE__, (size_t)(size)); \
        }                                                                                           \
    } while (0)

#define FREAD(file, data, size)                                                                      \
    do {                                                                                             \
        if (fread((data), (size), 1, (file)) != 1) {                                                 \
            error("[%s:%d] Could not read %lu bytes from file", __FILE__, __LINE__, (size_t)(size)); \
        }                                                                                            \
    } while (0)

#define CC4_CHECK(buf, str) \
    ((buf)[0] == (str)[0] && (buf)[1] == (str)[1] && (buf)[2] == (str)[2] && (buf)[3] == (str)[3])

#define CC4(c1, c2, c3, c4) (((c1) << 24) | ((c2) << 16) | ((c3) << 8) | (c4))

#define CHUNK_BEGIN(file, name, start)    \
    do {                                  \
        *(start) = ftell(out);            \
        FWRITE(file, name "\0\0\0\0", 8); \
    } while (0)

#define CHUNK_WRITE(file, structure)                   \
    do {                                               \
        FWRITE(file, structure, sizeof(*(structure))); \
    } while (0)

#define CHUNK_WRITE_RAW(file, data, length) FWRITE(file, data, length)

#define CHUNK_END(file, start, endian_type)     \
    do {                                        \
        long end = ftell(out);                  \
        endian_type size = { end - (start)-8 }; \
        fseek(out, (start) + 4, SEEK_SET);      \
        FWRITE(out, &size, 4);                  \
        fseek(out, end, SEEK_SET);              \
    } while (0)

__attribute__((format(printf, 1, 2))) NORETURN void
error(const char *fmt, ...);

__attribute__((format(printf, 1, 2))) void
warning(const char *fmt, ...);

#endif
