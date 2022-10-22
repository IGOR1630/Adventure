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
#include <stdlib.h>
#include "game.h"
#include "scene.h"
#include "world/map/map.h"

#ifdef PLATFORM_ANDROID
#include "ui/virtual_joystick.h"
#endif // PLATFORM_ANDROID

#define GAMEPLAY_TILE_SIZE 64.0

struct scene_data {
    map_t map;

#ifdef PLATFORM_ANDROID
    virtual_joystick_t virtual_joystick;
#endif // PLATFORM_ANDROID

    Rectangle camera;
    Texture spritesheet;
};

static void gameplay_draw_map(scene_data_t *data);

scene_data_t *gameplay_init(void)
{
    scene_data_t *data = malloc(sizeof(scene_data_t));

    map_load(&data->map);

#ifdef PLATFORM_ANDROID
    virtual_joystick_init(&data->virtual_joystick, 125, 175, game_height() - 175);
#endif // PLATFORM_ANDROID

    data->camera = (Rectangle) {
        .x = 0,
        .y = 0,

        // +1 because it's needed to draw one line/column more to fill gaps when
        // drawing a camera that hasn't a integer value.
        .width = ceil((float) game_width() / GAMEPLAY_TILE_SIZE) + 1,
        .height = ceil((float) game_height() / GAMEPLAY_TILE_SIZE) + 1,
    };

    data->spritesheet = game_get_texture("map-sprites");

    return data;
}

void gameplay_deinit(scene_data_t *data)
{
    map_save(&data->map);
    map_destroy(&data->map);

    free(data);
}


void gameplay_update(scene_data_t *data)
{
    Vector2 direction = { 0, 0 };

#ifdef PLATFORM_ANDROID
    direction = virtual_joystick_update(&data->virtual_joystick);
#else
    if (IsKeyDown(KEY_W))
        direction.y = 1;
    else if (IsKeyDown(KEY_S))
        direction.y = -1;

    if (IsKeyDown(KEY_D))
        direction.x = 1;
    else if (IsKeyDown(KEY_A))
        direction.x = -1;
#endif // PLATFORM_ANDROID
}

void gameplay_draw(scene_data_t *data)
{
    ClearBackground(BLACK);

    gameplay_draw_map(data);

#ifdef PLATFORM_ANDROID
    virtual_joystick_draw(&data->virtual_joystick);
#endif // PLATFORM_ANDROID
}

static void gameplay_draw_map(scene_data_t *data)
{
    int camera_x, camera_y;

    Rectangle tile = {
        .width = GAMEPLAY_TILE_SIZE,
        .height = GAMEPLAY_TILE_SIZE,
    };

    Rectangle sprite = {
        .width = 16,
        .height = 16,
    };

    Vector2 tile_rotation_origin = {
        .x = GAMEPLAY_TILE_SIZE / 2.0,
        .y = GAMEPLAY_TILE_SIZE / 2.0,
    };

    for (int layer = 0; layer < MAP_MAX_LAYERS; layer++) {
        for (int y = 0; y < data->camera.height; y++) {
            camera_y = y + data->camera.y;

            for (int x = 0; x < data->camera.width; x++) {
                camera_x = x + data->camera.x;

                if (TILE_IS_EMPTY(data->map.tiles[layer][camera_y][camera_x]))
                    continue;

                // TODO: Maybe fix a little that appear when move the camera,
                // some thin black lines around all tiles.
                tile.x = tile_rotation_origin.x
                    + (camera_x - data->camera.x) * tile.width;

                tile.y = tile_rotation_origin.y
                    + (camera_y - data->camera.y) * tile.height;

                sprite.x = TILE_GET_X(data->map.tiles[layer][camera_y][camera_x]);
                sprite.x = sprite.x * sprite.width + 1 * sprite.x;

                sprite.y = TILE_GET_Y(data->map.tiles[layer][camera_y][camera_x]);
                sprite.y = sprite.y * sprite.height + 1 * sprite.y;

                DrawTexturePro(data->spritesheet, sprite, tile,
                    tile_rotation_origin,
                    TILE_GET_ROTATION(data->map.tiles[layer][camera_y][camera_x]),
                    WHITE);
            }
        }
    }
}

