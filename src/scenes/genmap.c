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

#define DEFAULT_MAP_WIDTH  54
#define DEFAULT_MAP_HEIGHT 30

#define MAP_FILL_PERCENTAGE 0.50
#define GENERATION_STAGE_DELAY_TIME (50.0 / 1000.0)

#define MIN(a, b) ((a) < (b) ? (a) : (b))

enum {
    GENMAP_FILL_RANDOMLY = 0,
    GENMAP_POLISHING_MAP = 20,
    GENMAP_SELECT_TILES  = 21,
};

struct scene_data {
    map_t map;

    int map_border_width;

    int    generation_stage;
    double generation_stage_time;
};

static void random_fill_map(scene_data_t *data);
static void polish_map(scene_data_t *data);
static void select_map_tiles(scene_data_t *data);
static int count_neighbors(scene_data_t *data, int x, int y);

scene_data_t *genmap_init(void)
{
    srand(time(NULL));
    scene_data_t *data = malloc(sizeof(scene_data_t));

    if (!map_load(&data->map, "game-island.map"))
        map_create(&data->map, DEFAULT_MAP_WIDTH, DEFAULT_MAP_HEIGHT);

    data->map_border_width = 5;

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
    if (IsMouseButtonPressed(0))
        data->generation_stage = 0;

    if (GetTime() - data->generation_stage_time <= GENERATION_STAGE_DELAY_TIME)
        return;

    if (data->generation_stage <= GENMAP_FILL_RANDOMLY) {
        random_fill_map(data);
    } else if (data->generation_stage <= GENMAP_POLISHING_MAP) {
        polish_map(data);
    } else if (data->generation_stage <= GENMAP_SELECT_TILES) {
        select_map_tiles(data);
    } else {
    }

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

    float sprite_scale = MIN(tile.width / sprite.width, tile.height / sprite.height);


    ClearBackground(BLACK);

    for (int y = 0; y < data->map.height; y++) {
        tile.x = 0;

        for (int x = 0; x < data->map.width; x++) {
            if (data->generation_stage <= GENMAP_POLISHING_MAP) {
                if (data->map.tiles[0][y][x] != 0)
                    DrawRectangleRec(tile, WHITE);
            } else {// if (data->generation_stage <= GENMAP_SELECT_TILES) {
                sprite.x = TILE_X(data->map.tiles[0][y][x]);
                sprite.x = sprite.x * sprite.width + 1 * sprite.x;

                sprite.y = TILE_Y(data->map.tiles[0][y][x]);
                sprite.y = sprite.y * sprite.height + 1 * sprite.y;

                DrawTextureTiled(spritesheet, sprite, tile, (Vector2) { 0, 0 },
                    0, sprite_scale, WHITE);
                //DrawRectangleLinesEx(tile, 0.5, WHITE);
            }

            tile.x += tile.width;
        }

        tile.y += tile.height;
    }
}

static void random_fill_map(scene_data_t *data)
{
    for (int y = 0; y < data->map.height; y++)
        for (int x = 0; x < data->map.width; x++)
            data->map.tiles[0][y][x] =
                ((double) rand() / RAND_MAX) <= MAP_FILL_PERCENTAGE;
}

static void polish_map(scene_data_t *data)
{
    map_t next;
    int neighbors;


    map_create(&next, data->map.width, data->map.height);

    for (int y = 0; y < data->map.height; y++)
        for (int x = 0; x < data->map.width; x++)
            if (x < data->map_border_width || y < data->map_border_width
                    || x > data->map.width - data->map_border_width
                    || y > data->map.height - data->map_border_width)
                next.tiles[0][y][x] = 0;
            else if ((neighbors = count_neighbors(data, x, y)) > 4)
                next.tiles[0][y][x] = 1;
            else if (neighbors < 4)
                next.tiles[0][y][x] = 0;
            else
                next.tiles[0][y][x] = data->map.tiles[0][y][x];

    map_destroy(&data->map);
    data->map = next;
}

static void select_map_tiles(scene_data_t *data)
{
    map_t next;

    map_create(&next, data->map.width, data->map.height);

    for (int y = 0; y < data->map.height; y++) {
        for (int x = 0; x < data->map.width; x++) {
            if (data->map.tiles[0][y][x] == 0) {
                // TODO: function
                unsigned char neighbor_grass = 0;
                int bit = 0;
                int next_x, next_y;

                for (int offset_y = -1; offset_y <= 1; offset_y++) {
                    next_y = y + offset_y;

                    for (int offset_x = -1; offset_x <= 1; offset_x++) {
                        next_x = x + offset_x;

                        if (next_x < 0 || next_y < 0 || next_x >= data->map.width
                                || next_y >= data->map.height
                                || (next_x == x && next_y == y))
                            continue;
                        else if (data->map.tiles[0][next_y][next_x] != 0)
                            neighbor_grass |= 1 << bit;

                        bit++;
                    }
                }

                if (neighbor_grass == 0)
                    next.tiles[0][y][x] = TILE(rand() % 2, 0);
                else if ((neighbor_grass & 0x10) && !(neighbor_grass & 0x6B))
                    next.tiles[0][y][x] = TILE(4, 1);
                else if ((neighbor_grass & 0x08) && !(neighbor_grass & 0xD6))
                    next.tiles[0][y][x] = TILE(2, 1);
                else if ((neighbor_grass & 0x02) && !(neighbor_grass & 0xF8))
                    next.tiles[0][y][x] = TILE(3, 0);
                else if ((neighbor_grass & 0x40) && !(neighbor_grass & 0x1F))
                    next.tiles[0][y][x] = TILE(3, 2);
                else if ((neighbor_grass & 0x0B) == 0x0B && !(neighbor_grass & 0xD0))
                    next.tiles[0][y][x] = TILE(2, 0);
                else if ((neighbor_grass & 0x16) == 0x16 && !(neighbor_grass & 0x68))
                    next.tiles[0][y][x] = TILE(4, 0);
                else if ((neighbor_grass & 0x68) == 0x68 && !(neighbor_grass & 0x16))
                    next.tiles[0][y][x] = TILE(2, 2);
                else if ((neighbor_grass & 0xD0) == 0xD0 && !(neighbor_grass & 0x0B))
                    next.tiles[0][y][x] = TILE(4, 2);
                else if (neighbor_grass == 0x80)
                    next.tiles[0][y][x] = TILE(0, 1);
                else if (neighbor_grass == 0x20)
                    next.tiles[0][y][x] = TILE(1, 1);
                else if (neighbor_grass == 0x04)
                    next.tiles[0][y][x] = TILE(0, 2);
                else if (neighbor_grass == 0x01)
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

static int count_neighbors(scene_data_t *data, int x, int y)
{
    int next_x, next_y;
    int count = 0;

    for (int offset_y = -1; offset_y <= 1; offset_y++) {
        next_y = y + offset_y;

        for (int offset_x = -1; offset_x <= 1; offset_x++) {
            next_x = x + offset_x;

            if (next_x < 0 || next_y < 0 || next_x >= data->map.width
                    || next_y >= data->map.height || (next_x == x && next_y == y))
                continue;
            else if (data->map.tiles[0][next_y][next_x] != 0)
                count++;
        }
    }

    return count;
}

