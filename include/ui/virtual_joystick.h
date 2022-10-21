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

#ifndef VIRTUAL_JOYSTICK_H
#define VIRTUAL_JOYSTICK_H

#include <stdbool.h>
#include "raylib.h"

typedef struct {
    float radius;

    bool is_move_started;

    // Base and Top of the joystick
    Vector2 centers[2];
    Texture textures[2];
} virtual_joystick_t;

void virtual_joystick_init(virtual_joystick_t *joystick, float radius,
    float center_x, float center_y);

Vector2 virtual_joystick_update(virtual_joystick_t *joystick);
void virtual_joystick_draw(virtual_joystick_t *joystick);

#endif // !VIRTUAL_JOYSTICK_H

