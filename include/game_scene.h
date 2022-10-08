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

#ifndef GAME_SCENE_H
#define GAME_SCENE_H


typedef enum {
    GAME_SCENE_NONE,
    GAME_SCENE_COUNT,
} game_scene_t;


typedef struct scene_data scene_data_t;


void game_scene_initialize(void);

scene_data_t *game_scene_init(game_scene_t scene);
void          game_scene_deinit(game_scene_t scene, scene_data_t *data);

void          game_scene_update(game_scene_t scene, scene_data_t *data);
void          game_scene_draw(game_scene_t scene, scene_data_t *data);

#endif // !GAME_SCENE_H

