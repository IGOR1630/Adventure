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

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "raylib.h"
#include "game.h"
#include "scene.h"
#include "world/map.h"

#define GENMAP_MAP_BASE_SIZE       30
#define GENMAP_MAP_LAND_SPAWN_RATE (50.0 / 100.0)

#define GENMAP_STEP_CHANGE_DELAY   (100.0 / 1000.0)

enum {
    GENMAP_STEPS_STAGE0 = 1,
    GENMAP_STEPS_STAGE1 = 10,
    GENMAP_STEPS_STAGE2 = 1,
};

struct scene_data {
    map_t map;

    int map_border_size;

    int    generation_steps;
    int    generation_stage;
    double generation_stage_time;
};

static void genmap_stage0(scene_data_t *data);
static void genmap_stage1(scene_data_t *data);
static void genmap_stage2(scene_data_t *data);

// Helper functions
static int stage1_count_neighbors(map_t *map, int x, int y);
static int stage2_find_neighbors(map_t *map, int x, int y);

scene_data_t *genmap_init(void)
{
    const int width = game_width();
    const int height = game_height();

    int map_width, map_height;


    srand(time(NULL));
    scene_data_t *data = malloc(sizeof(scene_data_t));

    if (!map_load(&data->map, "game-island.map")) {
        if (width < height) {
            map_width = GENMAP_MAP_BASE_SIZE;
            map_height = ((float) height / width) * map_width;
        } else {
            map_height = GENMAP_MAP_BASE_SIZE;
            map_width = ((float) width / height) * map_height;
        }

        map_create(&data->map, map_width, map_height);
    }

    data->map_border_size = 5;

    data->generation_steps = 0;
    data->generation_stage = 0;
    data->generation_stage_time = 0;

    return data;
}

void genmap_deinit(scene_data_t *data)
{
    map_save(&data->map, "game-island.map");
    map_destroy(&data->map);

    free(data);
}

void genmap_update(scene_data_t *data)
{
    if (GetTime() - data->generation_stage_time <= GENMAP_STEP_CHANGE_DELAY)
        return;

    switch (data->generation_stage) {
    case 0:
        genmap_stage0(data);
        break;
    case 1:
        genmap_stage1(data);
        break;
    case 2:
        genmap_stage2(data);
        break;
    //default:
       // game_set_scene("gameplay");
    }

    if (--data->generation_steps == 0)
        data->generation_stage++;

    data->generation_stage_time = GetTime();
}

void genmap_draw(scene_data_t *data)
{
    Texture spritesheet = game_get_texture("map-sprites");

    Rectangle tile = {
        .x = 0,
        .y = 0,

        .width = (float) game_width() / data->map.width,
        .height = (float) game_height() / data->map.height,
    };

    Rectangle sprite = {
        .x = 0,
        .y = 0,

        .width = 16,
        .height = 16,
    };

    ClearBackground(BLACK);
    for (int y = 0; y < data->map.height; y++) {
        tile.x = 0;

        for (int x = 0; x < data->map.width; x++) {
            switch (data->generation_stage) {
            case 0: case 1:
                if (data->map.tiles[0][y][x] != 0)
                    DrawRectangleRec(tile, WHITE);
                break;
            default:
                sprite.x = TILE_X(data->map.tiles[0][y][x]);
                sprite.x = sprite.x * sprite.width + 1 * sprite.x;

                sprite.y = TILE_Y(data->map.tiles[0][y][x]);
                sprite.y = sprite.y * sprite.height + 1 * sprite.y;

                DrawTexturePro(spritesheet, sprite, tile, (Vector2) { 0, 0 }, 0,
                    WHITE);
                break;
            }

            tile.x += tile.width;
        }

        tile.y += tile.height;
    }
}

static void genmap_stage0(scene_data_t *data)
{
    if (data->generation_steps == 0)
        data->generation_steps = GENMAP_STEPS_STAGE0;

    for (int y = 0; y < data->map.height; y++) {
        for (int x = 0; x < data->map.width; x++) {
            if (((double) rand() / RAND_MAX) <= GENMAP_MAP_LAND_SPAWN_RATE)
                data->map.tiles[0][y][x] = 1;
            else
                data->map.tiles[0][y][x] = 0;

            data->map.tiles[1][y][x] = 0;
        }
    }
}

static void genmap_stage1(scene_data_t *data)
{
    int neighbors;
    map_t next;


    if (data->generation_steps == 0)
        data->generation_steps = GENMAP_STEPS_STAGE1;

    map_create(&next, data->map.width, data->map.height);

    for (int y = 0; y < data->map.height; y++) {
        for (int x = 0; x < data->map.width; x++) {
            neighbors = stage1_count_neighbors(&data->map, x, y);

            if (x < data->map_border_size || y < data->map_border_size
                    || x > data->map.width - data->map_border_size
                    || y > data->map.height - data->map_border_size
                    || neighbors < 4)
                next.tiles[0][y][x] = 0;
            else if (neighbors > 4)
                next.tiles[0][y][x] = 1;
            else
                next.tiles[0][y][x] = data->map.tiles[0][y][x];
        }
    }

    map_destroy(&data->map);
    data->map = next;
}

static void genmap_stage2(scene_data_t *data)
{
    int neighbors;
    map_t next;

    if (data->generation_steps == 0)
        data->generation_steps = GENMAP_STEPS_STAGE2;

    map_create(&next, data->map.width, data->map.height);

    for (int y = 0; y < data->map.height; y++) {
        for (int x = 0; x < data->map.width; x++) {
            if (data->map.tiles[0][y][x] == 0) {
                neighbors = stage2_find_neighbors(&data->map, x, y);

                if (neighbors == 0)
                    next.tiles[0][y][x] = TILE(rand() % 2, 0);
                else if ((neighbors & 0x10) && !(neighbors & 0x6B))
                    next.tiles[0][y][x] = TILE(4, 1);
                else if ((neighbors & 0x08) && !(neighbors & 0xD6))
                    next.tiles[0][y][x] = TILE(2, 1);
                else if ((neighbors & 0x02) && !(neighbors & 0xF8))
                    next.tiles[0][y][x] = TILE(3, 0);
                else if ((neighbors & 0x40) && !(neighbors & 0x1F))
                    next.tiles[0][y][x] = TILE(3, 2);
                else if ((neighbors & 0x0B) == 0x0B && !(neighbors & 0xD0))
                    next.tiles[0][y][x] = TILE(2, 0);
                else if ((neighbors & 0x16) == 0x16 && !(neighbors & 0x68))
                    next.tiles[0][y][x] = TILE(4, 0);
                else if ((neighbors & 0x68) == 0x68 && !(neighbors & 0x16))
                    next.tiles[0][y][x] = TILE(2, 2);
                else if ((neighbors & 0xD0) == 0xD0 && !(neighbors & 0x0B))
                    next.tiles[0][y][x] = TILE(4, 2);
                else if (neighbors == 0x80)
                    next.tiles[0][y][x] = TILE(0, 1);
                else if (neighbors == 0x20)
                    next.tiles[0][y][x] = TILE(1, 1);
                else if (neighbors == 0x04)
                    next.tiles[0][y][x] = TILE(0, 2);
                else if (neighbors == 0x01)
                    next.tiles[0][y][x] = TILE(1, 2);
                else
                    next.tiles[0][y][x] = TILE(3, 1);
            } else {
                next.tiles[0][y][x] = TILE(5, rand() % 2);
            }
        }
    }

    map_destroy(&data->map);
    data->map = next;
}

static int stage1_count_neighbors(map_t *map, int x, int y)
{
    int next_x, next_y;
    int neighbors = 0;


    for (int offset_y = -1; offset_y <= 1; offset_y++) {
        next_y = y + offset_y;

        for (int offset_x = -1; offset_x <= 1; offset_x++) {
            next_x = x + offset_x;

            if (next_x < 0 || next_y < 0 || next_x >= map->width
                    || next_y >= map->height || (next_x == x && next_y == y))
                continue;
            else if (map->tiles[0][next_y][next_x] != 0)
                neighbors++;
        }
    }

    return neighbors;
}

static int stage2_find_neighbors(map_t *map, int x, int y)
{
    int neighbors = 0;
    int bit = 0;
    int next_x, next_y;

    for (int offset_y = -1; offset_y <= 1; offset_y++) {
        next_y = y + offset_y;

        for (int offset_x = -1; offset_x <= 1; offset_x++) {
            next_x = x + offset_x;

            if (next_x < 0 || next_y < 0 || next_x >= map->width
                    || next_y >= map->height || (next_x == x && next_y == y))
                continue;
            else if (map->tiles[0][next_y][next_x] != 0)
                neighbors |= 1 << bit;

            bit++;
        }
    }

    return neighbors;
}

