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

#include "raylib.h"
#include "core/game.h"
#include "core/scene.h"

scene_data_t *logo_init(void)
{
    return NULL;
}

void logo_update(scene_data_t *data)
{
    static float time = 0;

    if (time > 0 && GetTime() - time >= 3.5)
        game_scene_make_current("menu");
    else if (time == 0)
        time = GetTime();
}

void logo_draw(scene_data_t *data)
{
    Texture logo = game_texture_get("tela_logo-img");

    (void) data;

    ClearBackground(GetColor(0x038c7fff));
    DrawRectangleGradientV(0, 0, 1280, 720, GetColor(0x038c7fff), GOLD);

    DrawTexture(logo, 0, 0, WHITE);
}

void logo_deinit(scene_data_t *data)
{
    (void) data;
}

