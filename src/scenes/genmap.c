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
#include <string.h>
#include <time.h>
#include "raylib.h"
#include "game.h"
#include "scene.h"
#include "world/map.h"

#define GENMAP_MAP_BASE_SIZE          350
#define GENMAP_MAP_LAND_SPAWN_RATE    (55.0 / 100.0)
#define GENMAP_TREE_GENERATION_FACTOR (10.0 / 100.0)

#define GENMAP_STEP_CHANGE_DELAY      (100.0 / 1000.0)

#define MIN(a, b) ((a) < (b) ? (a) : (b))

enum {
    GENMAP_STEPS_STAGE0 = 1,
    GENMAP_STEPS_STAGE1 = 10,
    GENMAP_STEPS_STAGE2 = 5,
};

struct scene_data {
    map_t map;

    int map_border_size;

    int    generation_steps;
    int    generation_stage;
    double generation_stage_time;

    Texture spritesheet;
};

static void genmap_stage0(scene_data_t *data);
static void genmap_stage1(scene_data_t *data);
static void genmap_stage2(scene_data_t *data);
static void genmap_stage3(scene_data_t *data);

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

    if (!map_exists()) {
        if (width < height) {
            map_width = GENMAP_MAP_BASE_SIZE;
            map_height = ((float) height / width) * map_width;
        } else {
            map_height = GENMAP_MAP_BASE_SIZE;
            map_width = ((float) width / height) * map_height;
        }

        map_create(&data->map, map_width, map_height);
    } else {
        // Jump to the last stage that change to game scene
        data->generation_stage = 4;
        data->generation_stage_time = 0;

        return data;
    }

    data->map_border_size = 5;

    data->generation_steps = 0;
    data->generation_stage = 0;
    data->generation_stage_time = 0;

    data->spritesheet = game_get_texture("map-sprites");

    return data;
}

void genmap_deinit(scene_data_t *data)
{
    if (!map_exists())
        map_save(&data->map);

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
    case 3:
        genmap_stage3(data);
        break;
    default:
        game_set_scene("gameplay");
        return;
    }

    if (--data->generation_steps == 0)
        data->generation_stage++;

    data->generation_stage_time = GetTime();
}

void genmap_draw(scene_data_t *data)
{
    int draw_layers = 0;

    Rectangle tile;
    Rectangle sprite = {
        .width = 16,
        .height = 16,
    };

    Vector2 tile_rotation_origin;

    // The tile is a square, therefore, have the same size each side. This also
    // fix a bug of little gaps between some tiles when drawing due the previous
    // calculation was producing different sizes for each side.
    //
    // Previous calculation:
    //   width = (float) game_width() / data->map.width
    //   height = (float) game_height() / data->map.height
    tile.width = tile.height = MIN((float) game_width() / data->map.width,
        (float) game_height() / data->map.height);

    // The origin of rotation of the image is the center of the image that has
    // sane size sides.
    tile_rotation_origin.x = tile_rotation_origin.y = tile.width / 2;

    ClearBackground(BLACK);

    tile.y = tile_rotation_origin.y;
    for (int y = 0; y < data->map.height; y++) {
        tile.x = tile_rotation_origin.x;

        for (int x = 0; x < data->map.width; x++) {
            switch (data->generation_stage) {
            case 0: case 1:
                draw_layers = 0;
                if (data->map.tiles[0][y][x] != 0)
                    DrawRectangleRec(tile, WHITE);
                break;
            case 2:
                draw_layers = 1;
                break;
            case 3:
                draw_layers = 2;
                break;
            }

            for (int layer = 0; layer < draw_layers; layer++) {
                if (TILE_IS_EMPTY(data->map.tiles[layer][y][x]))
                    break;

                sprite.x = TILE_X(data->map.tiles[layer][y][x]);
                sprite.x = sprite.x * sprite.width + 1 * sprite.x;

                sprite.y = TILE_Y(data->map.tiles[layer][y][x]);
                sprite.y = sprite.y * sprite.height + 1 * sprite.y;

                DrawTexturePro(data->spritesheet, sprite, tile,
                    tile_rotation_origin,
                    TILE_ROTATION(data->map.tiles[layer][y][x]), WHITE);
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
#define TEST_NEIGHBORS(must_have, can_have, action)                            \
    if ((neighbors & (must_have)) == (must_have) && !(neighbors & ~(can_have))) \
        next.tiles[0][y][x] = (action)

    int neighbors;
    map_t next;

    if (data->generation_steps == 0)
        data->generation_steps = GENMAP_STEPS_STAGE2;
    else
        // Only the first pass on this stage are needed, the others are for see
        // the result.
        return;

    map_create(&next, data->map.width, data->map.height);

    for (int y = 0; y < data->map.height; y++) {
        for (int x = 0; x < data->map.width; x++) {
            if (data->map.tiles[0][y][x] == 0) {
                neighbors = stage2_find_neighbors(&data->map, x, y);
                next.tiles[0][y][x] = TILE(0, 0, (rand() % 4) * 90);

                // Sides
                TEST_NEIGHBORS(0x02, 0x07, TILE(3, 0, 0));
                TEST_NEIGHBORS(0x10, 0x94, TILE(3, 0, 90));
                TEST_NEIGHBORS(0x40, 0xE0, TILE(3, 0, 180));
                TEST_NEIGHBORS(0x08, 0x29, TILE(3, 0, 270));

                // Diagonals
                TEST_NEIGHBORS(0x0B, 0x2F, TILE(2, 0, 0));
                TEST_NEIGHBORS(0x16, 0x97, TILE(2, 0, 90));
                TEST_NEIGHBORS(0xD0, 0xF4, TILE(2, 0, 180));
                TEST_NEIGHBORS(0x68, 0xE9, TILE(2, 0, 270));

                // Corners
                TEST_NEIGHBORS(0x80, 0x80, TILE(0, 1, 0));
                TEST_NEIGHBORS(0x20, 0x20, TILE(0, 1, 90));
                TEST_NEIGHBORS(0x04, 0x04, TILE(0, 1, 270));
                TEST_NEIGHBORS(0x01, 0x01, TILE(0, 1, 180));
            } else {
                next.tiles[0][y][x] = TILE(5, 0, (rand() % 4) * 90);
            }
        }
    }

    map_destroy(&data->map);
    data->map = next;

#undef TEST_NEIGHBORS
}

static void genmap_stage3(scene_data_t *data)
{
    int floors_count = 0;
    int tree_x, tree_y;

    int trees_to_generate;

    bool tree_generated;

    // The total steps is determined by the number of trees that should be
    // generated.
    if (data->generation_steps == 0) {
        for (int y = 0; y < data->map.height; y++)
            for (int x = 0; x < data->map.width; x++)
                if (TILE_IS(data->map.tiles[0][y][x], 5, 0))
                    floors_count++;

        data->generation_steps = floors_count * GENMAP_TREE_GENERATION_FACTOR;
    }

    trees_to_generate = ceil(data->generation_steps * GENMAP_TREE_GENERATION_FACTOR);
    for (int tree = 0; tree < trees_to_generate; tree++) {
        tree_generated = false;

        do {

            tree_x = rand() % data->map.width;
            tree_y = rand() % data->map.height;

            if (TILE_IS(data->map.tiles[0][tree_y][tree_x], 5, 0)
                    && TILE_IS_EMPTY(data->map.tiles[1][tree_y - 0][tree_x])
                    && TILE_IS_EMPTY(data->map.tiles[1][tree_y - 1][tree_x]))
                tree_generated = true;
        } while (!tree_generated);

        data->map.tiles[1][tree_y - 0][tree_x] = TILE(16, 11, 0);
        data->map.tiles[1][tree_y - 1][tree_x] = TILE(16, 10, 0);
    }

    data->generation_steps -= trees_to_generate;
    if (data->generation_steps == 0)
        data->generation_steps++;
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

