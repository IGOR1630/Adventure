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

#include <math.h>
#include <stdbool.h>
#include "raylib.h"
#include "game.h"
#include "ui/virtual_joystick.h"

void virtual_joystick_init(virtual_joystick_t *joystick, float radius,
    float center_x, float center_y)
{
    joystick->radius = radius;

    joystick->centers[0] = joystick->centers[1] = (Vector2) {
        .x = center_x,
        .y = center_y,
    };

    joystick->textures[0] = game_get_texture("joy-base");
    joystick->textures[1] = game_get_texture("joy-top");

    joystick->is_move_started = false;
}

void virtual_joystick_update(virtual_joystick_t *joystick, Vector2 *move)
{
    float displacement;

    Vector2 mouse = game_virtual_mouse();

    if (mouse.x < game_width() / 2) {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            move->x = (mouse.x -= joystick->centers[0].x);
            move->y = (mouse.y -= joystick->centers[0].y);

            displacement = sqrt(pow(mouse.x, 2) + pow(mouse.y, 2));
            if (displacement > joystick->radius) {
                if (joystick->is_move_started) {
                    mouse.x *= joystick->radius / displacement;
                    mouse.y *= joystick->radius / displacement;
                }
            } else {
                joystick->is_move_started = true;
            }

            mouse.x += joystick->centers[0].x;
            mouse.y += joystick->centers[0].y;

            if (joystick->is_move_started)
                joystick->centers[1] = mouse;
            else
                move->x = move->y = 0;
        } else if (IsMouseButtonUp(MOUSE_BUTTON_LEFT)) {
            joystick->is_move_started = false;
            joystick->centers[1] = joystick->centers[0];
        }
    } else {
        joystick->is_move_started = false;
        joystick->centers[1] = joystick->centers[0];
    }
}

void virtual_joystick_draw(virtual_joystick_t *joystick)
{
    Rectangle texture_src = (Rectangle) {
        .x = 0,
        .y = 0,
    };

    Rectangle texture_dest;

    for (int i = 0; i < 2; i++) {
        texture_src.width = joystick->textures[i].width;
        texture_src.height = joystick->textures[i].height;

        texture_dest.x = joystick->centers[i].x - joystick->radius / (i + 1);
        texture_dest.y = joystick->centers[i].y - joystick->radius / (i + 1);

        texture_dest.width = joystick->radius * (2 - i);
        texture_dest.height = joystick->radius * (2 - i);

        DrawTexturePro(joystick->textures[i], texture_src, texture_dest,
            (Vector2) { 0, 0 }, 0, WHITE);
    }
}

