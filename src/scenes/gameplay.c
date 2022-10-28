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

#include <math.h>
#include <stdlib.h>
#include "game.h"
#include "scene.h"
#include "utils/utils.h"
#include "utils/list.h"
#include "world/map/map.h"
#include "world/map/tile.h"
#include "world/entity/player.h"

#ifdef PLATFORM_ANDROID
#include "ui/virtual_joystick.h"
#endif // PLATFORM_ANDROID

#define GAMEPLAY_LOAD_STAGES 4

struct scene_data {
    map_t map;
    player_t player;

    list(entity_t *) entities;

#ifdef PLATFORM_ANDROID
    virtual_joystick_t virtual_joystick;
#endif // PLATFORM_ANDROID

    Rectangle camera;
    Texture spritesheet;

    int loading_stage;
};

static void update_loading(scene_data_t *data);
static void update_game(scene_data_t *data);

static void draw_loading(scene_data_t *data);
static void draw_game(scene_data_t *data);

scene_data_t *gameplay_init(void)
{
    scene_data_t *data = malloc(sizeof(scene_data_t));

    data->loading_stage = 0;

    list_create(data->entities);

    data->camera = (Rectangle) {
        .x = 0,
        .y = 0,

        // +1 because it's needed to draw one line/column more to fill gaps when
        // drawing a camera that hasn't a integer value.
        .width = ceil((float) game_width() / TILE_DRAW_SIZE) + 1,
        .height = ceil((float) game_height() / TILE_DRAW_SIZE) + 1,
    };

    data->spritesheet = game_get_texture("tiles");

#ifdef PLATFORM_ANDROID
    virtual_joystick_init(&data->virtual_joystick, 125, 175, game_height() - 175);
#endif // PLATFORM_ANDROID

    return data;
}

void gameplay_deinit(scene_data_t *data)
{
    // Save the map state and free the memory
    map_save(&data->map);
    map_destroy(&data->map);

    // Save the player state and free the memory
    player_save(&data->player);
    entity_destroy((entity_t *) &data->player);

    for (unsigned i = 0; i < list_size(data->entities); i++)
        entity_destroy(list_get(data->entities, i));

    list_destroy(data->entities);

    free(data);
}


void gameplay_update(scene_data_t *data)
{
    if (data->loading_stage < GAMEPLAY_LOAD_STAGES)
        update_loading(data);
    else
        update_game(data);
}

void gameplay_draw(scene_data_t *data)
{
    if (data->loading_stage < GAMEPLAY_LOAD_STAGES)
        draw_loading(data);
    else
        draw_game(data);
}

static void update_loading(scene_data_t *data)
{
    switch (data->loading_stage) {
    // Load map dimensions
    case 0:
        map_load(&data->map, MAP_LOAD_DIMENSIONS);
        break;

    // Load map layer 0
    case 1:
        map_load(&data->map, MAP_LOAD_LAYER_0);
        break;

    // Load map layer 1
    case 2:
        map_load(&data->map, MAP_LOAD_LAYER_1);
        break;

    // Load player state
    case 3:
        player_create(&data->player, &data->map);
        player_load(&data->player);
        break;
    }

    data->loading_stage++;
}

static void update_game(scene_data_t *data)
{
    Vector2 direction = { 0, 0 };

#ifdef PLATFORM_ANDROID
    direction = virtual_joystick_update(&data->virtual_joystick);
#else
    if (IsKeyDown(KEY_W))
        direction.y = -1;
    else if (IsKeyDown(KEY_S))
        direction.y = 1;

    if (IsKeyDown(KEY_D))
        direction.x = 1;
    else if (IsKeyDown(KEY_A))
        direction.x = -1;
#endif // PLATFORM_ANDROID

    // Update the player state
    data->player.base.direction = vec2ang(direction.x, direction.y);
    data->player.base.is_moving = direction.x != 0 || direction.y != 0;
    entity_update((entity_t *) &data->player);

    // Update the game camera
    data->camera.x = data->player.base.position.x + 1 - data->camera.width / 2;
    data->camera.y = data->player.base.position.y + 1 - data->camera.height / 2;

    if (data->camera.x < 0)
        data->camera.x = 0;
    else if (data->camera.x >= data->map.width - data->camera.width)
        data->camera.x = data->map.width - data->camera.width;

    if (data->camera.y < 0)
        data->camera.y = 0;
    else if (data->camera.y >= data->map.height - data->camera.height)
        data->camera.y = data->map.height - data->camera.height;

    for (unsigned i = 0; i < list_size(data->entities); i++)
        entity_update(list_get(data->entities, i));
}

static void draw_loading(scene_data_t *data)
{
    switch (data->loading_stage) {
    // Draw loading map dimensions
    case 0:
        break;

    // Draw loading map layer 0
    case 1:
        break;

    // Draw loading map layer 1
    case 2:
        break;

    // Draw loading player state
    case 3:
        break;
    }
}

static void draw_game(scene_data_t *data)
{
    int camera_x, camera_y;
    int player_x, player_y;

    Rectangle tile = {
        .width = TILE_DRAW_SIZE,
        .height = TILE_DRAW_SIZE,
    };

    Rectangle sprite = {
        .width = 16,
        .height = 16,
    };

    Vector2 tile_rotation_origin = {
        .x = TILE_DRAW_SIZE / 2.0,
        .y = TILE_DRAW_SIZE / 2.0,
    };

    ClearBackground(BLACK);

    player_x = data->player.base.position.x;
    player_y = data->player.base.position.y;

    for (int layer = 0; layer < MAP_MAX_LAYERS; layer++) {
        for (int y = 0; y < data->camera.height; y++) {
            camera_y = y + data->camera.y;

            for (int x = 0; x < data->camera.width; x++) {
                camera_x = x + data->camera.x;

                if (layer == 1) {
                    // Draw the player
                    if (player_x == camera_x && player_y == camera_y)
                        entity_draw((entity_t *) &data->player, data->camera);
                }

                if (tile_empty(data->map.tiles[layer][camera_y][camera_x]))
                    continue;

                tile.x = tile_rotation_origin.x
                    + (camera_x - data->camera.x) * tile.width;

                tile.y = tile_rotation_origin.y
                    + (camera_y - data->camera.y) * tile.height;

                sprite.x = tile_x(data->map.tiles[layer][camera_y][camera_x])
                    * fabs(sprite.width);

                sprite.y = tile_y(data->map.tiles[layer][camera_y][camera_x])
                    * fabs(sprite.height);

                if (tile_flipped(data->map.tiles[layer][camera_y][camera_x], 0))
                    sprite.width = -fabs(sprite.width);
                else
                    sprite.width = fabs(sprite.width);

                if (tile_flipped(data->map.tiles[layer][camera_y][camera_x], 1))
                    sprite.height = -fabs(sprite.height);
                else
                    sprite.height = fabs(sprite.height);

                // Fix little gaps that appear when move the camera, some thin
                // black lines around all tiles.
                tile.width++;
                tile.height++;

                DrawTexturePro(data->spritesheet, sprite, tile,
                    tile_rotation_origin,
                    tile_rotation(data->map.tiles[layer][camera_y][camera_x]),
                    WHITE);

                tile.width--;
                tile.height--;
            }
        }
    }

    for (unsigned i = 0; i < list_size(data->entities); i++)
        entity_draw(list_get(data->entities, i), data->camera);

#ifdef PLATFORM_ANDROID
    virtual_joystick_draw(&data->virtual_joystick);
#endif // PLATFORM_ANDROID
}

