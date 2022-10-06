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

#ifndef GAME_H
#define GAME_H

#include <stdbool.h>
#include "game_scene.h"

bool         game_init(void);
void         game_deinit(void);

void         game_set_scene(game_scene_t scene);
game_scene_t game_get_scene(void);

bool         game_is_running(void);
void         game_end_run(void);
void         game_run(void);

#endif // !GAME_H

