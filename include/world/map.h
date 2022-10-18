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

#ifndef MAP_H
#define MAP_H

#include <stdbool.h>

#define MAP_MAX_LAYERS 2

#define TILE(x, y, rotation) (1u << 31 | (rotation) << 20 | (y) << 12 | (x))

#define TILE_X(tile)         (((tile) >> 0)  & 0x00000FFF)
#define TILE_Y(tile)         (((tile) >> 12) & 0x000000FF)
#define TILE_ROTATION(tile)  (((tile) >> 20) & 0x000007FF)

#define TILE_IS_EMPTY(tile)  (!((tile) & 1u << 31))

#define TILE_IS(tile, x, y)  (TILE_X(tile) == (x) && TILE_Y(tile) == (y))

typedef unsigned int tile_t;

typedef struct map {
    tile_t **tiles[MAP_MAX_LAYERS];

    int width;
    int height;
} map_t;

void map_create(map_t *map, int width, int height);
bool map_load(map_t *map, const char *filename);
void map_destroy(map_t *map);

void map_save(map_t *map, const char *filename);

#endif // !MAP_H

