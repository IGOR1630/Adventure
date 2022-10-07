/*
The GPLv3 License (GPLv3)

Copyright (c) 2022 Jonatha Gabriel, Igor Nepomuceno and Renne Bispo

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

#include <stddef.h>
#include "game_scene.h"


#define GAME_SCENE(scene, scene_name)                                          \
    extern scene_data_t * scene_name##_init(void);                             \
    extern void           scene_name##_deinit(scene_data_t *data);             \
                                                                               \
    extern void           scene_name##_update(scene_data_t *data);             \
    extern void           scene_name##_draw(scene_data_t *data);               \
                                                                               \
    g_scenes[scene - 1] = (scene_t) {                                          \
        .init   = scene_name##_init,                                           \
        .deinit = scene_name##_deinit,                                         \
                                                                               \
        .update = scene_name##_update,                                         \
        .draw   = scene_name##_draw,                                           \
    }


struct {
    scene_data_t *(* init)(void);
    void          (* deinit)(scene_data_t *data);

    void          (* update)(scene_data_t *data);
    void          (* draw)(scene_data_t *data);
} g_scenes[GAME_SCENE_COUNT - 1];


void game_scene_initialize(void)
{
}


scene_data_t *game_scene_init(game_scene_t scene)
{ return g_scenes[scene - 1].init(); }

void game_scene_deinit(game_scene_t scene, scene_data_t *data)
{ g_scenes[scene - 1].deinit(data); }


void game_scene_update(game_scene_t scene, scene_data_t *data)
{ g_scenes[scene - 1].update(data); }

void game_scene_draw(game_scene_t scene, scene_data_t *data)
{ g_scenes[scene - 1].draw(data); }

