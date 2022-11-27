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

/* SCENE_IMPORT
 *
 * This macro declare the scene callback prototypes with the name given by
 * \scene parameter.
 *
 * The prototype is formed as follow:
 *   scene name + callback suffix
 *
 * Callback suffix | Purpose
 *      _init      | allocate the memory data needed by the scene
 *      _deinit    | free the memory previous allocated by _init
 *      _update    | update the scene data, i.e. move objects, handle user input
 *      _draw      | draw things on screen
 */
#define SCENE_IMPORT(scene)                                                    \
    extern scene_data_t *scene##_init(void);                                   \
    extern void          scene##_deinit(scene_data_t *data);                   \
                                                                               \
    extern void          scene##_update(scene_data_t *data);                   \
    extern void          scene##_draw(scene_data_t *data)

/* SCENE
 *
 * Initialize a scene struct with scene informations i.e. scene callbacks and
 * name all automatic "generated" by only given \scene.
 */
#define SCENE(scene) (scene_t) {                                               \
        .init   = scene##_init,                                                \
        .deinit = scene##_deinit,                                              \
                                                                               \
        .update = scene##_update,                                              \
        .draw   = scene##_draw,                                                \
                                                                               \
        .name = #scene                                                         \
    }

/* Store the scene internal data. When defining the scene data use the struct
 * scene_data, the scene_data_t is a generic way to pass information without
 * know the data needed by all scenes.
 *
 * Example: scene foo.c
 * struct scene_data {
 *   foo data declaration goes here:
 *
 *   int bar;
 *   float baq;
 *
 *   and so on...
 * };
 *
 * NOTE:
 * Because the data is accessed through scene_data_t without know what struct
 * scene_data is scene_data_t must by allocated by _init callback.
 */
typedef struct scene_data scene_data_t;

/* This store the scene functions callbacks and name. Initialize struct this
 * with SCENE macro above.
 */
typedef struct {
    scene_data_t *(* init)(void);
    void          (* deinit)(scene_data_t *data);

    void          (* update)(scene_data_t *data);
    void          (* draw)(scene_data_t *data);

    const char *name;
} scene_t;

#endif // !SCENE_H

