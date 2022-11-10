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

#ifndef TILE_H
#define TILE_H

#define TILE_DRAW_SIZE 64.0

#define tile_new(x, y) (1u << 31 | (y) << 8 | (x))

#define tile_x(tile) ((tile) & 0x000000FF)
#define tile_y(tile) (((tile) >> 8) & 0x000000FF)

#define tile_flip(tile, axis) ((tile) | 1u << ((axis) + 29))
#define tile_flipped(tile, axis) (((tile) >> (((axis) + 29))) & 0x1)

#define tile_collidable(tile) ((tile) | 1u << 28)
#define tile_collision(tile) (((tile) >> 28) & 0x1)

#define tile_equal(tile, other) (((tile) & 0x0000FFFF) == ((other) & 0x0000FFFF))
#define tile_empty(tile) (!(((tile) >> 31) & 0x1))

typedef unsigned int tile_t;

#endif // !TILE_H

