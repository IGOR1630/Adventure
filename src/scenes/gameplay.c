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
#include "world/map.h"

#define GAMEPLAY_TILE_SIZE 32.0

struct scene_data {
    map_t map;

    Rectangle camera;
    Texture spritesheet;
};

scene_data_t *gameplay_init(void)
{
    scene_data_t *data = malloc(sizeof(scene_data_t));

    map_load(&data->map);

    data->camera = (Rectangle) {
        .x = 0,
        .y = 0,

        .width = ceil((float) game_width() / GAMEPLAY_TILE_SIZE),
        .height = ceil((float) game_height() / GAMEPLAY_TILE_SIZE),
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
}

void gameplay_draw(scene_data_t *data)
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
        .x = GAMEPLAY_TILE_SIZE / 2,
        .y = GAMEPLAY_TILE_SIZE / 2,
    };

    ClearBackground(BLACK);

    for (int layer = 0; layer < MAP_MAX_LAYERS; layer++) {
        tile.y = tile_rotation_origin.y;

        for (int y = 0; y < data->camera.height; y++) {
            tile.x = tile_rotation_origin.x;
            camera_y = y + data->camera.y;

            for (int x = 0; x < data->camera.width; x++) {
                camera_x = x + data->camera.x;

                if (TILE_IS_EMPTY(data->map.tiles[layer][camera_y][camera_x])) {
                    tile.x += tile.width;
                    continue;
                }

                sprite.x = TILE_X(data->map.tiles[layer][y][x]);
                sprite.x = sprite.x * sprite.width + 1 * sprite.x;

                sprite.y = TILE_Y(data->map.tiles[layer][y][x]);
                sprite.y = sprite.y * sprite.height + 1 * sprite.y;

                DrawTexturePro(data->spritesheet, sprite, tile,
                    tile_rotation_origin,
                    TILE_ROTATION(data->map.tiles[layer][y][x]), WHITE);

                tile.x += tile.width;
            }

            tile.y += tile.height;
        }
    }
}

