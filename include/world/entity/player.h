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

#ifndef PLAYER_H
#define PLAYER_H

#include <stdbool.h>
#include "raylib.h"
#include "world/map/map.h"

typedef struct player {
    map_t map;

    Vector2 position;
} player_t;

void player_create(player_t *player, map_t map);
bool player_load(player_t *player, map_t map);

bool player_save(player_t *player);

bool player_exists(void);

void player_update(player_t *player, Vector2 direction);
void player_draw(player_t *player, Rectangle *camera);

#endif // !PLAYER_H

