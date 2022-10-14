/*
The GPLv3 License (GPLv3)

Copyright (c) 2022 Jonatha Gabriel

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

#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "world/map.h"

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

bool map_load(map_t *map, const char *filename)
{
    int c;

    int layer;
    int width;
    int height;

    tile_t **tiles;

    FILE *map_file;


    if ((map_file = fopen(filename, "r")) == NULL)
        return false;

    map->width  = 0;
    map->height = 0;

    while ((c = fgetc(map_file)) != EOF) {
        // Ignore commented lines
        if (c == '#')
            while ((c = fgetc(map_file)) != '\n');

        if (c == '!') {
            // Read the layer level and ignore any other character
            layer = 0;
            while ((c = fgetc(map_file)) != '\n')
                layer += c == '!' ? 1 : 0;

            // Read the layer data until find the end section mark
            height = 0;
            tiles  = NULL;
            while ((c = fgetc(map_file)) != '~') {
                tiles = realloc(tiles, sizeof(tile_t *) * (++height));
                tiles[height - 1] = malloc(sizeof(tile_t) * (width = 1));

                while ((c = fgetc(map_file)) != '\n') {
                    if (isdigit(c))
                        tiles[height - 1][width - 1] = (c - '0') +
                            tiles[height - 1][width - 1] * 10;
                    else
                        tiles[height - 1] = realloc(tiles[height - 1],
                            sizeof(tile_t) * (++width));
                }
            }

            map->tiles[layer] = tiles;

            // Store the map dimensions
            map->width  = map->width  < width  ? width  : map->width;
            map->height = map->height < height ? height : map->height;
        }
    }

    fclose(map_file);
    return true;
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

void map_save(map_t *map, const char *filename)
{
    FILE *map_file;


    if ((map_file = fopen(filename, "w")) == NULL)
        return;

    fprintf(map_file, "# Game map, don't modify!\n");
    fprintf(map_file, "# Auto generated game map file.\n\n");

    for (int layer = 0; layer < MAP_MAX_LAYERS; layer++) {
        fprintf(map_file, "%*s Map layer: %d\n", layer, "!!!!!", layer + 1);

        for (int y = 0; y < map->height; y++) {
            for (int x = 0; x < map->width; x++) {
                fprintf(map_file, "%d", map->tiles[layer][y][x]);

                if (x + 1 < map->width)
                    fprintf(map_file, " ");
            }

            fprintf(map_file, "\n");
        }

        fprintf(map_file, "~\n\n");
    }

    fclose(map_file);
}

