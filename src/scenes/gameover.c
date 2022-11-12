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

#include <stdio.h>
#include <stdlib.h>
#include "raylib.h"
#include "game.h"
#include "scene.h"

#define CLOUDS 3

struct scene_data {
    Font alagard;

    Texture grass;
    Texture cloud; 

    Vector2 clouds_pos[CLOUDS];

    float time;
};

scene_data_t *gameover_init(void)
{
    scene_data_t *data = malloc(sizeof(scene_data_t));

    data->alagard = LoadFont("custom_alagard.png");

    data->grass = game_get_texture("gram-img");
    data->cloud = game_get_texture("cloud-img");

    for (int i = 0; i < CLOUDS; i++)
        data->clouds_pos[i] = (Vector2) {
            .x = rand() % game_width(),
            .y = (rand() % 3) * data->cloud.height,
        };

    data->time = GetTime();

    return data;
}

void gameover_deinit(scene_data_t *data)
{
    // Erase the game save
    fclose(game_file("w"));

    UnloadFont(data->alagard);
    free(data);
}


void gameover_update(scene_data_t *data)
{
    if (GetTime() - data->time >= 2.5)
        game_set_scene("menu");

    for (int i = 0; i < CLOUDS; i++)
        data->clouds_pos[i].x = data->clouds_pos[i].x + 1 < game_width() ?
            data->clouds_pos[i].x + 1 : 0;
}

void gameover_draw(scene_data_t *data)
{
    Vector2 text_pos, text_dim;

    Rectangle src_img, dest_img;

    ClearBackground(BLACK);

    DrawRectangleGradientV(0, 0, game_width(), game_height(),
        GetColor(0x038c7fff), GOLD);

    // Clouds
    for (int i = 0; i < CLOUDS; i++) {
        DrawTextureV(data->cloud, data->clouds_pos[i], WHITE);

        if (data->clouds_pos[i].x + data->cloud.width > game_width())
            DrawTexture(data->cloud, data->clouds_pos[i].x - game_width(),
                    data->clouds_pos[i].y, WHITE);
    }

    // Game over text
    text_pos = text_dim = MeasureTextEx(data->alagard, "Game Over!!", 80, 0);
    text_pos.x = game_width() / 2 - text_pos.x / 2;
    text_pos.y = game_height() / 2 - text_pos.y / 2;

    DrawRectangle(
            text_pos.x - (text_dim.x * 0.55) / 2,
            text_pos.y,
            text_dim.x * 1.55,
            text_dim.y * 0.5,
            DARKGREEN);

    DrawRectangle(
            text_pos.x - text_dim.x / 4,
            text_pos.y + text_dim.y / 2,
            text_dim.x * 1.5,
            text_dim.y * 0.5,
            WHITE);

    DrawTextEx(data->alagard, "Game Over!!", text_pos, 80, 0, GREEN);

    // Grass
    src_img.x = (src_img.y = 0) + 10;
    src_img.width = data->grass.width - 10;
    src_img.height = data->grass.height;

    dest_img.x = (dest_img.y = game_height() - data->grass.height) * 0;
    dest_img.width = game_width();
    dest_img.height = data->grass.height;

    DrawTextureTiled(data->grass, src_img, dest_img, (Vector2) { 0, 0 },
        0, 1, WHITE);
}

