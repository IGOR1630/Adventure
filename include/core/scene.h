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

#ifndef SCENE_H
#define SCENE_H

#include <stdint.h>

#define SCENE_IMPORT(scene)                                                    \
    extern scene_data_t *scene##_init(void);                                   \
    extern void          scene##_deinit(scene_data_t *data);                   \
                                                                               \
    extern void          scene##_update(scene_data_t *data);                   \
    extern void          scene##_draw(scene_data_t *data)

#define SCENE(scene) (scene_t) {                                               \
        .init   = scene##_init,                                                \
        .deinit = scene##_deinit,                                              \
                                                                               \
        .update = scene##_update,                                              \
        .draw   = scene##_draw,                                                \
                                                                               \
        .name = #scene                                                         \
    }

typedef struct scene_data scene_data_t;

typedef struct {
    scene_data_t *(* init)(void);
    void          (* deinit)(scene_data_t *data);

    void          (* update)(scene_data_t *data);
    void          (* draw)(scene_data_t *data);

    const char *name;
} scene_t;

#endif // !SCENE_H

