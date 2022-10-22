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

#ifndef TILE_H
#define TILE_H

#define TILE_NEW(x, y, rotation)                                               \
    (1u << 31 | (rotation) << 20 | (y) << 12 | (x))

#define TILE_GET_X(tile)                                                       \
    (((tile) >> 0) & 0x00000FFF)

#define TILE_GET_Y(tile)                                                       \
    (((tile) >> 12) & 0x000000FF)

#define TILE_GET_ROTATION(tile)                                                \
    (((tile) >> 20) & 0x000007FF)

#define TILE_IS_EMPTY(tile)                                                    \
    (!((tile) & 1u << 31))

#define TILE_IS_EQUAL(tile, other)                                             \
    (((tile) & 0x000FFFFF) == ((other) & 0x000FFFFF))

typedef unsigned int tile_t;

#endif // !TILE_H

