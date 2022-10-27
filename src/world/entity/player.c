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

#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "game.h"
#include "world/map/map.h"
#include "world/map/tile.h"
#include "world/entity/player.h"

static FILE *player_goto_section(void);

void player_create(player_t *player, map_t map)
{
    player->map = map;

    player->position = (Vector2) {
        .x = map.width / 2 - 1,
        .y = map.height / 2 - 1,
    };
}

bool player_load(player_t *player, map_t map)
{
    int c;

    char token[21];

    bool open_player_section = false;
    bool player_section_found = false;

    FILE *file;

    if ((file = player_goto_section()) == NULL)
        return false;

    while ((c = fgetc(file)) != EOF) {
        if (c == '<') {
            fscanf(file, "%20s", token);

            if (strcmp(token, "Player") == 0) {
                open_player_section = true;
                player_section_found = true;
            } else if (open_player_section) {
                break;
            }
        } else if (c == '>') {
            fscanf(file, "%20s", token);

            if (strcmp(token, "Player") == 0) {
                open_player_section = false;
                break;
            } else if (open_player_section) {
                break;
            }
        } else if (open_player_section && isalpha(c) && isupper(c)) {
            token[0] = c;
            fscanf(file, "%19s", token + 1);

            fgetc(file);

            if (strcmp(token, "Position") == 0)
                fread(&player->position, sizeof(Vector2), 1, file);
        }
    }

    fclose(file);
    return player_section_found && !open_player_section;
}

bool player_save(player_t *player)
{
    FILE *file;

    if ((file = player_goto_section()) == NULL)
        return false;

    fprintf(file, "<Player\n");

    fprintf(file, "Position ");
    fwrite(&player->position, sizeof(Vector2), 1, file);
    fprintf(file, "\n");

    fprintf(file, ">Player\n");
    fclose(file);

    return true;
}

bool player_exists(void)
{
    int c;

    char token[21];

    bool open_player_section = false;
    bool player_section_found = false;

    FILE *file;

    if ((file = game_file("r")) == NULL)
        return false;

    while ((c = fgetc(file)) != EOF) {
        if (c == '<') {
            fscanf(file, "%20s", token);

            if (strcmp(token, "Player") == 0) {
                open_player_section = true;
                player_section_found = true;
            }
        } else if (c == '>') {
            fscanf(file, "%20s", token);

            if (strcmp(token, "Player") == 0) {
                open_player_section = false;
                break;
            }
        }
    }

    fclose(file);
    return player_section_found && !open_player_section;
}

static FILE *player_goto_section(void)
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

            if (strcmp(token, "Player") == 0) {
                fsetpos(file, &pos);
                return file;
            }
        }

        fgetpos(file, &pos);
    }

    fclose(file);
    return (file = game_file("a"));
}

void player_update(player_t *player, Vector2 direction)
{
    Vector2 pos = {
        .x = player->position.x + direction.x * 5 * GetFrameTime(),
        .y = player->position.y + direction.y * 5 * GetFrameTime(),
    };

    // Clamp the new position to map bounds
    if (pos.x < 0)
        pos.x = 0;
    else if (pos.x >= player->map.width - 1)
        pos.x = player->map.width - 1;

    if (pos.y < 0)
        pos.y = 0;
    else if (pos.y >= player->map.height - 1)
        pos.y = player->map.height - 1;

    // Check collisions
    direction.x = direction.x > 0 ? direction.x : 0;
    direction.y = direction.y > 0 ? direction.y : 0;
/*
    if (TILE_IS_EQUAL(player->map.tiles[0][(int) player->position.y]
                [(int) (pos.x + direction.x)], TILE_NEW(5, 0, 0))
            && TILE_IS_EQUAL(player->map.tiles[0][(int) player->position.y + 1]
                [(int) (pos.x + direction.x)], TILE_NEW(5, 0, 0)))
        */player->position.x = pos.x;

    /*if (TILE_IS_EQUAL(player->map.tiles[0][(int) (pos.y + direction.y)]
                [(int) player->position.x], TILE_NEW(5, 0, 0))
            && TILE_IS_EQUAL(player->map.tiles[0][(int) (pos.y + direction.y)]
                [(int) player->position.x + 1], TILE_NEW(5, 0, 0)))
        */player->position.y = pos.y;
}

void player_draw(player_t *player, Rectangle *camera)
{
    Rectangle tile = {
        .x = (player->position.x - camera->x) * 32,
        .y = (player->position.y - camera->y) * 32,

        .width = 32,
        .height = 32,
    };

    DrawRectangleRec(tile, RED);
    DrawText(TextFormat("%.2f %.2f", player->position.x, player->position.y),
            0, 0, 20, RED);
}

