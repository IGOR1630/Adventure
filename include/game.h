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

#ifndef GAME_H
#define GAME_H

#include <stdbool.h>
#include "raylib.h"
#include "scene.h"

bool    game_init(int width, int height);
void    game_deinit(void);

void    game_register_scene(scene_t scene);
void    game_set_scene(const char *scene_name);
scene_t game_current_scene(void);

bool    game_is_running(void);
void    game_end_run(void);
void    game_run(void);

int     game_width(void);
int     game_height(void);

Vector2 game_virtual_mouse(void);

Texture game_load_texture(const char *filename, const char *name);
Texture game_get_texture(const char *name);

#endif // !GAME_H

