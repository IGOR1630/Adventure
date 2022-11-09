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

#ifndef UTILS_H
#define UTILS_H

#include <math.h>

#define UTILS_PI 3.14159265358979323846

#define vec2ang(x, y) (                                                        \
    (x) == 0 ?                                                                 \
        ((y) == 0 ? 0                                                          \
            : (UTILS_PI / 2) * ((y) > 0 ? 1 : 3))                              \
        : atan((y) / (x)) + ((x) < 0 ? UTILS_PI : UTILS_PI * ((y) < 0) * 2)    \
)

#define deg2rad(degrees) ((degrees) * (UTILS_PI / 180.0))
#define rad2deg(radians) ((radians) * (180.0 / UTILS_PI))

#define dbg_point()                                                            \
    printf("FILE: %s\nFUNC: %s\nLINE: %u\n\n", __FILE__, __func__, __LINE__);

#endif // !UTILS_H

