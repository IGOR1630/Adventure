/*
The GPLv3 License (GPLv3)

Copyright (c) 2022 Jonatha Gabriel <jonathagabrielns@gmail.com>

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

#ifndef LIST_H
#define LIST_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define list(type) struct {                                                    \
        type     *values;                                                      \
                                                                               \
        uint32_t count;                                                        \
        uint32_t capacity;                                                     \
    }

#define list_create(list) do {                                                 \
    (list).count    = 0;                                                       \
    (list).capacity = 10;                                                      \
                                                                               \
    (list).values = malloc(sizeof(*(list).values) * (list).capacity);          \
} while (0)

#define list_destroy(list) do {                                                \
    (list).count    = 0;                                                       \
    (list).capacity = 0;                                                       \
                                                                               \
    free((list).values);                                                       \
} while (0)

#define list_add(list, value) do {                                             \
    if ((list).count + 1 >= (list).capacity) {                                 \
        (list).capacity *= (list).capacity;                                    \
                                                                               \
        (list).values = realloc((list).values,                                 \
            sizeof(*(list).values) * (list).capacity);                         \
    }                                                                          \
                                                                               \
    (list).values[(list).count] = (value);                                     \
                                                                               \
    (list).count++;                                                            \
} while (0)

#define list_insert(list, index, value) do {                                   \
    if ((list).count + 1 >= (list).capacity) {                                 \
        (list).capacity *= (list).capacity;                                    \
                                                                               \
        (list).values = realloc((list).values,                                 \
            sizeof(*(list).values) * (list).capacity);                         \
    }                                                                          \
                                                                               \
    memmove((list).values + (index) + 1, (list).values + (index),              \
        ((list).count - (index)) * sizeof(*(list).values));                    \
                                                                               \
    (list).values[(index)] = (value);                                          \
                                                                               \
    (list).count++;                                                            \
} while (0)

#define list_remove(list, index) do {                                          \
    memmove((list).values + (index), (list).values + (index) + 1,              \
        ((list).count - (index) - 1) * sizeof(*(list).values));                \
                                                                               \
    (list).count--;                                                            \
} while (0)

#define list_size(list) ((list).count)

#define list_get(list, index) ((list).values[(index)])
#define list_set(list, index, value) (list).values[(index)] = (value)

#endif // !LIST_H

