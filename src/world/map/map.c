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

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "game.h"
#include "world/map/map.h"

static FILE *map_goto_section(void);

void map_create(map_t *map, int width, int height)
{
    for (int layer = 0; layer < MAP_MAX_LAYERS; layer++) {
        map->tiles[layer] = malloc(sizeof(tile_t *) * height);

        for (int y = 0; y < height; y++)
            map->tiles[layer][y] = malloc(sizeof(tile_t) * width);
    }

    map->width  = width;
    map->height = height;
}

bool map_load(map_t *map)
{
    int c;

    int layer;

    char token[21];

    bool open_map_section = false;
    bool map_section_found = false;

    FILE *file;

    if ((file = map_goto_section()) == NULL)
        return false;

    while ((c = fgetc(file)) != EOF) {
        if (c == '<') {
            fscanf(file, "%20s", token);

            if (strcmp(token, "Map") == 0) {
                open_map_section = true;
                map_section_found = true;

                fscanf(file, "%u %u", &map->width, &map->height);
            } else if (open_map_section && strcmp(token, "Layer") == 0) {
                fscanf(file, "%u", &layer);

                // Discard the newline character preceded by layer declaration
                fgetc(file);

                map->tiles[layer] = malloc(sizeof(tile_t *) * map->height);
                for (int y = 0; y < map->height; y++) {
                    map->tiles[layer][y] = malloc(sizeof(tile_t) * map->width);
                    fread(map->tiles[layer][y], sizeof(tile_t), map->width, file);
                }
            } else if (open_map_section) {
                break;
            }
        } else if (c == '>') {
            fscanf(file, "%20s", token);

            if (strcmp(token, "Map") == 0) {
                open_map_section = false;
                break;
            } else if (open_map_section && strcmp(token, "Layer") == 0) {
                continue;
            } else if (open_map_section) {
                break;
            }
        }
    }

    if (open_map_section)
        map_destroy(map);

    fclose(file);
    return map_section_found && !open_map_section;
}

void map_destroy(map_t *map)
{
    // Free the map data
    for (int i = 0; i < MAP_MAX_LAYERS; i++) {
        for (int j = 0; j < map->height; j++)
            free(map->tiles[i][j]);

        free(map->tiles[i]);
    }

    // Reset map state
    map->width  = 0;
    map->height = 0;
}

bool map_save(map_t *map)
{
    FILE *file;

    if ((file = map_goto_section()) == NULL)
        return false;

    fprintf(file, "<Map %u %u\n", map->width, map->height);
    for (int layer = 0; layer < MAP_MAX_LAYERS; layer++) {
        fprintf(file, "<Layer %u\n", layer);

        for (int y = 0; y < map->height; y++)
            fwrite(map->tiles[layer][y], sizeof(tile_t), map->width, file);

        fprintf(file, "\n>Layer\n");
    }

    fprintf(file, ">Map\n");
    fclose(file);

    return true;
}

bool map_exists(void)
{
    int c;

    char token[21];

    bool open_map_section = false;
    bool map_section_found = false;

    FILE *file;

    if ((file = game_file("r")) == NULL)
        return false;

    while ((c = fgetc(file)) != EOF) {
        if (c == '<') {
            fscanf(file, "%20s", token);

            if (strcmp(token, "Map") == 0) {
                open_map_section = true;
                map_section_found = true;
            }
        } else if (c == '>') {
            fscanf(file, "%20s", token);

            if (strcmp(token, "Map") == 0) {
                open_map_section = false;
                break;
            }
        }
    }

    fclose(file);
    return map_section_found && !open_map_section;
}

static FILE *map_goto_section(void)
{
    int c;

    char token[21];

    fpos_t pos;

    FILE *file;

    if ((file = game_file("r+")) == NULL)
        return (file = game_file("w"));

    fgetpos(file, &pos);
    while ((c = fgetc(file)) != EOF) {
        if (c == '<') {
            fscanf(file, "%20s", token);

            if (strcmp(token, "Map") == 0) {
                fsetpos(file, &pos);
                return file;
            }
        }

        fgetpos(file, &pos);
    }

    fclose(file);
    return (file = game_file("a"));
}

