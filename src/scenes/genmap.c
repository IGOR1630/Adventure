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
#include <string.h>
#include <time.h>
#include "raylib.h"
#include "game.h"
#include "scene.h"
#include "utils/list.h"
#include "world/map/map.h"
#include "world/map/tile.h"
#include "world/entity/spawner.h"
#include "world/entity/player.h"

#define GENMAP_MAP_BASE_SIZE          300
#define GENMAP_MAP_LAND_SPAWN_RATE    (55.0 / 100.0)
#define GENMAP_TREE_GENERATION_FACTOR (5.0 / 100.0)

#define GENMAP_FLOWER_SPAWN_RATE      (8.0 / 100.0)
#define GENMAP_SINGLE_ROCK_SPAWN_RATE (0.5 / 100.0)
#define GENMAP_ROCKS_SPAWN_RATE       (40.0 / 100.0)
#define GENMAP_GRAVESTONE_SPAWN_RATE  (20.0 / 100.0)

#define GENMAP_STEP_CHANGE_DELAY      (100.0 / 1000.0)

#define MIN(a, b) ((a) < (b) ? (a) : (b))

enum {
    GENMAP_STEPS_STAGE0 = 1,
    GENMAP_STEPS_STAGE1 = 10,
    GENMAP_STEPS_STAGE2 = 5,
    GENMAP_STEPS_STAGE4 = 5,
};

struct scene_data {
    map_t map;
    player_t player;

    list(Vector2) spawn_points;
    spawner_list_t spawners;

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
static void genmap_stage4(scene_data_t *data);
static void genmap_stage5(scene_data_t *data);

// Helper functions
static int stage1_count_neighbors(map_t *map, int x, int y);
static int stage2_find_neighbors(map_t *map, int x, int y);
static int stage4_count_neighbors(map_t *map, int x, int y, tile_t tile);
static void stage5_generate_spawners(scene_data_t *data, int spawners);

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

        if (!player_exists())
            player_create(&data->player, &data->map);

        if (!spawner_exists()) {
            list_create(data->spawn_points);
            spawner_create(&data->spawners);
        }
    } else {
        // Jump to the last stage that change to game scene
        data->generation_stage = 5;
        data->generation_stage_time = 0;

        return data;
    }

    data->map_border_size = 5;

    data->generation_steps = 0;
    data->generation_stage = 0;
    data->generation_stage_time = 0;

    data->spritesheet = game_get_texture("tiles");

    return data;
}

void genmap_deinit(scene_data_t *data)
{
    if (!map_exists()) {
        map_save(&data->map);
        map_destroy(&data->map);
    }

    if (!player_exists())
        player_save(&data->player);

    if (!spawner_exists()) {
        list_destroy(data->spawn_points);

        spawner_save(&data->spawners);
        spawner_destroy(&data->spawners);
    }

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
    case 4:
        genmap_stage4(data);
        break;
    case 5:
        genmap_stage5(data);
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
            case 3: case 4: case 5:
                draw_layers = 2;
                break;
            }

            for (int layer = 0; layer < draw_layers; layer++) {
                if (tile_empty(data->map.tiles[layer][y][x]))
                    break;

                sprite.x = tile_x(data->map.tiles[layer][y][x])
                    * fabs(sprite.width);

                sprite.y = tile_y(data->map.tiles[layer][y][x])
                    * fabs(sprite.height);

                if (tile_flipped(data->map.tiles[layer][y][x], 0))
                    sprite.width = -fabs(sprite.width);
                else
                    sprite.width = fabs(sprite.width);

                if (tile_flipped(data->map.tiles[layer][y][x], 1))
                    sprite.height = -fabs(sprite.height);
                else
                    sprite.height = fabs(sprite.height);

                DrawTexturePro(data->spritesheet, sprite, tile,
                    tile_rotation_origin,
                    tile_rotation(data->map.tiles[layer][y][x]), WHITE);
            }

            if (data->generation_stage == 5)
                for (unsigned i = 0; i < list_size(data->spawners); i++)
                    if (list_get(data->spawners, i).position.x == x
                            && list_get(data->spawners, i).position.y == y)
                        DrawRectangleRec(tile, BLUE);

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
                next.tiles[0][y][x] = tile_new(9, 1);

                // Sides
                TEST_NEIGHBORS(0x02, 0x07, tile_new(9, 0));
                TEST_NEIGHBORS(0x10, 0x94, tile_new(10, 1));
                TEST_NEIGHBORS(0x40, 0xE0, tile_new(9, 2));
                TEST_NEIGHBORS(0x08, 0x29, tile_new(8, 1));

                // Sides double
                TEST_NEIGHBORS(0x42, 0xE7, tile_new(4, 4));
                TEST_NEIGHBORS(0x18, 0xBD, tile_new(5, 4));

                // Sides all
                TEST_NEIGHBORS(0x59, 0xFF, tile_new(6, 4));

                // Sides triple
                TEST_NEIGHBORS(0x1A, 0xBF, tile_new(6, 0));
                TEST_NEIGHBORS(0x58, 0xFD, tile_new(7, 0));
                TEST_NEIGHBORS(0x4A, 0xEF, tile_new(6, 1));
                TEST_NEIGHBORS(0x52, 0xF7, tile_new(7, 1));

                // Sides corner
                TEST_NEIGHBORS(0x22, 0x27, tile_new(0, 1));
                TEST_NEIGHBORS(0x82, 0x87, tile_new(1, 1));
                TEST_NEIGHBORS(0x41, 0xE1, tile_new(0, 0));
                TEST_NEIGHBORS(0x44, 0xE4, tile_new(1, 0));

                TEST_NEIGHBORS(0x0C, 0x2D, tile_new(3, 1));
                TEST_NEIGHBORS(0x88, 0xA9, tile_new(3, 0));
                TEST_NEIGHBORS(0x11, 0x95, tile_new(2, 1));
                TEST_NEIGHBORS(0x30, 0xB4, tile_new(2, 0));

                // Sides double corner
                TEST_NEIGHBORS(0xA2, 0xA7, tile_new(5, 3));
                TEST_NEIGHBORS(0x31, 0xB5, tile_new(4, 2));
                TEST_NEIGHBORS(0x45, 0xE5, tile_new(4, 3));
                TEST_NEIGHBORS(0x8C, 0xAD, tile_new(5, 2));

                // Diagonals
                TEST_NEIGHBORS(0x0A, 0x2F, tile_new(8, 0));
                TEST_NEIGHBORS(0x12, 0x97, tile_new(10, 0));
                TEST_NEIGHBORS(0x50, 0xF4, tile_new(10, 2));
                TEST_NEIGHBORS(0x48, 0xE9, tile_new(8, 2));

                // Diagonals corner
                TEST_NEIGHBORS(0x8A, 0xAF, tile_new(6, 2));
                TEST_NEIGHBORS(0x32, 0xB7, tile_new(7, 2));
                TEST_NEIGHBORS(0x51, 0xF5, tile_new(7, 3));
                TEST_NEIGHBORS(0x4C, 0xED, tile_new(6, 3));

                // Corners
                TEST_NEIGHBORS(0x80, 0x80, tile_new(8, 3));
                TEST_NEIGHBORS(0x20, 0x20, tile_new(9, 3));
                TEST_NEIGHBORS(0x04, 0x04, tile_new(8, 4));
                TEST_NEIGHBORS(0x01, 0x01, tile_new(9, 4));

                // Corners double
                TEST_NEIGHBORS(0x84, 0x84, tile_new(3, 3));
                TEST_NEIGHBORS(0x21, 0x21, tile_new(2, 3));
                TEST_NEIGHBORS(0x05, 0x05, tile_new(3, 4));
                TEST_NEIGHBORS(0xA0, 0xA0, tile_new(2, 4));

                // Corners opposite double
                TEST_NEIGHBORS(0x81, 0x81, tile_new(3, 2));
                TEST_NEIGHBORS(0x24, 0x24, tile_new(2, 2));

                // Corners triple
                TEST_NEIGHBORS(0x85, 0x85, tile_new(5, 0));
                TEST_NEIGHBORS(0x25, 0x25, tile_new(4, 0));
                TEST_NEIGHBORS(0xA4, 0xA4, tile_new(5, 1));
                TEST_NEIGHBORS(0xA1, 0xA1, tile_new(4, 1));

                // Corners all
                TEST_NEIGHBORS(0xA5, 0xA5, tile_new(7, 4));
            } else {
                next.tiles[0][y][x] = tile_new(11, 2);
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
    int tree_type;

    bool tree_generated;

    // The total steps is determined by the number of trees that should be
    // generated.
    if (data->generation_steps == 0) {
        for (int y = 0; y < data->map.height; y++)
            for (int x = 0; x < data->map.width; x++)
                if (tile_equal(data->map.tiles[0][y][x], tile_new(11, 2)))
                    floors_count++;

        data->generation_steps = floors_count * GENMAP_TREE_GENERATION_FACTOR;
    }

    trees_to_generate = ceil(data->generation_steps * GENMAP_TREE_GENERATION_FACTOR);
    for (int tree = 0; tree < trees_to_generate; tree++) {
        tree_generated = false;

        do {

            tree_x = rand() % data->map.width;
            tree_y = rand() % data->map.height;

            if (tile_equal(data->map.tiles[0][tree_y][tree_x], tile_new(11, 2))
                    && tile_empty(data->map.tiles[1][tree_y - 0][tree_x])
                    && tile_empty(data->map.tiles[1][tree_y - 1][tree_x]))
                tree_generated = true;
        } while (!tree_generated);

        tree_type = rand() % 2;
        data->map.tiles[1][tree_y - 0][tree_x] = tile_new(tree_type, 4);
        data->map.tiles[1][tree_y - 1][tree_x] = tile_new(tree_type, 3);
    }

    data->generation_steps -= trees_to_generate;
    if (data->generation_steps == 0)
        data->generation_steps++;
}

static void genmap_stage4(scene_data_t *data)
{
    int type;
    tile_t tile;

    if (data->generation_steps == 0)
        data->generation_steps = GENMAP_STEPS_STAGE4;

    for (int y = 0; y < data->map.height; y++) {
        for (int x = 0; x < data->map.width; x++) {
            tile = 0;

            if (!tile_empty(data->map.tiles[1][y][x]))
                continue;

            if (tile_equal(data->map.tiles[0][y][x], tile_new(11, 2))) {
                type = rand() % 2;

                if (((double) rand() / RAND_MAX) <= GENMAP_FLOWER_SPAWN_RATE)
                    tile = tile_new(11, type);
                else if (((double) rand() / RAND_MAX) <= GENMAP_GRAVESTONE_SPAWN_RATE
                        && (stage4_count_neighbors(&data->map, x, y,
                                tile_new(11, 0)) > 2
                            || stage4_count_neighbors(&data->map, x, y,
                                tile_new(11, 1)) > 3))
                    tile = !stage4_count_neighbors(&data->map, x, y,
                        tile_new(1 - type, 2)) ? tile_new(type, 2) : 0;
            } else if (tile_equal(data->map.tiles[0][y][x], tile_new(9, 1))) {
                type = rand() % 2;

                if (((double) rand() / RAND_MAX) <= GENMAP_SINGLE_ROCK_SPAWN_RATE)
                    tile = tile_new(type + 10, type + 3);
                else if (((double) rand() / RAND_MAX) <= GENMAP_ROCKS_SPAWN_RATE
                        && (stage4_count_neighbors(&data->map, x, y,
                                tile_new(10, 3))
                            || stage4_count_neighbors(&data->map, x, y,
                                tile_new(11, 4))))
                    tile = tile_new(11 - type, 3 + type);
            }

            if (!tile_empty(tile) && rand() % 2)
                tile = tile_flip(tile, 0);

            data->map.tiles[1][y][x] = tile;
        }
    }
}

/* The fifth generation stage:
 *
 *   On this stage are generated the spawn points of enemies the number of steps
 * aren't pre determined because its how many as possible spawn points to place
 * on the map, therefore, is only stop when isn't possible to place other spawn.
 */
static void genmap_stage5(scene_data_t *data)
{
    if (data->generation_steps == 0) {
        for (int y = 0; y < data->map.height; y++)
            for (int x = 0; x < data->map.width; x++)
                list_add(data->spawn_points, ((Vector2) { x, y }));

        data->generation_steps = 2;
    }

    stage5_generate_spawners(data, 40);
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

static int stage4_count_neighbors(map_t *map, int x, int y, tile_t tile)
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
            else if (tile_equal(map->tiles[1][next_y][next_x], tile))
                neighbors++;
        }
    }

    return neighbors;
}

static void stage5_generate_spawners(scene_data_t *data, int spawners)
{
    Vector2 point;
    Vector2 spawner_position;

    bool is_valid_point = false;
    int point_number;

    if (spawners == 0 || list_size(data->spawn_points) == 0)
        return;

    while (!is_valid_point && list_size(data->spawn_points) > 0) {
        point_number = rand() % list_size(data->spawn_points);
        point = list_get(data->spawn_points, point_number);

        is_valid_point = true;
        for (unsigned i = 0; i < list_size(data->spawners); i++) {
            spawner_position = list_get(data->spawners, i).position;

            if (pow(point.x - spawner_position.x, 2)
                    + pow(point.y - spawner_position.y, 2)
                    < pow(SPAWNER_DISTANCE_RADIUS, 2)) {
                is_valid_point = false;
                break;
            }
        }

        list_remove(data->spawn_points, point_number);

        if (!is_valid_point)
            continue;

        spawner_new(&data->spawners, point);
        data->generation_steps = 2;
    }

    stage5_generate_spawners(data, spawners - 1);
}

