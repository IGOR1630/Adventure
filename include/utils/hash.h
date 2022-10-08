/*
The GPLv3 License (GPLv3)

Copyright (c) 2022 Jonatha Gabriel.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef HASH_H
#define HASH_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define hash(type) struct {                                                    \
        uint64_t *keys;                                                        \
        type     *values;                                                      \
                                                                               \
        uint32_t count;                                                        \
        uint32_t capacity;                                                     \
    }

#define hash_create(hash) do {                                                 \
    (hash).count    = 0;                                                       \
    (hash).capacity = 10;                                                      \
                                                                               \
    (hash).keys   = malloc(sizeof(*(hash).keys) * (hash).capacity);            \
    (hash).values = malloc(sizeof(*(hash).values) * (hash).capacity);          \
} while (0)

#define hash_destroy(hash) do {                                                \
    (hash).count    = 0;                                                       \
    (hash).capacity = 0;                                                       \
                                                                               \
    free((hash).keys);                                                         \
    free((hash).values);                                                       \
} while (0)

#define hash_key(key, out) do {                                                \
    const char *c     = (key);                                                 \
    uint64_t    p_pow = 1;                                                     \
    (out)             = 0;                                                     \
                                                                               \
    for (; *c != '\0'; c++) {                                                  \
        (out) = ((out) + (*c - 'a' + 1) * p_pow) % 18446744073709551557ULL;    \
        p_pow = (p_pow * 53) % 18446744073709551557ULL;                        \
    }                                                                          \
} while (0);

#define hash_add(hash, key, value) do {                                        \
    if ((hash).count + 1 >= (hash).capacity) {                                 \
        (hash).capacity *= (hash).capacity;                                    \
                                                                               \
        (hash).keys = realloc((hash).keys,                                     \
            sizeof(*(hash).keys) * (hash).capacity);                           \
                                                                               \
        (hash).values = realloc((hash).values,                                 \
            sizeof(*(hash).values) * (hash).capacity);                         \
    }                                                                          \
                                                                               \
    hash_key((key), (hash).keys[(hash).count]);                                \
    (hash).values[(hash).count] = (value);                                     \
                                                                               \
    (hash).count++;                                                            \
} while (0)

#define hash_remove(hash, key) do {                                            \
    uint64_t key_hashed;                                                       \
    hash_key((key), key_hashed);                                               \
                                                                               \
    for (uint32_t i = 0; i < (hash).count; i++) {                              \
        if ((hash).keys[i] != key_hashed) continue;                            \
                                                                               \
        memmove((hash).values + i, (hash).values + i + 1,                      \
            ((hash).count - i - 1) * sizeof(*(hash).values));                  \
                                                                               \
        memmove((hash).keys + i, (hash).keys + i + 1,                          \
            ((hash).count - i - 1) * sizeof(*(hash).keys));                    \
                                                                               \
        (hash).count--;                                                        \
        break;                                                                 \
    }                                                                          \
} while (0)

#define hash_size(hash) ((hash).count)

#define hash_get(hash, key, out) do {                                          \
    uint64_t key_hashed;                                                       \
    hash_key((key), key_hashed);                                               \
                                                                               \
    for (uint32_t i = 0; i < (hash).count; i++) {                              \
        if ((hash).keys[i] != key_hashed) continue;                            \
                                                                               \
        (out) = (hash).values[i];                                              \
        break;                                                                 \
    }                                                                          \
} while (0)

#define hash_set(hash, value, key) do {                                        \
    uint64_t key_hashed;                                                       \
    hash_key((key), key_hashed);                                               \
                                                                               \
    for (uint32_t i = 0; i < (hash).count; i++) {                              \
        if ((hash).keys[i] != key_hashed) continue;                            \
                                                                               \
        (hash).values[i] = (value);                                            \
        break;                                                                 \
    }                                                                          \
} while (0)

#endif // !HASH_H

