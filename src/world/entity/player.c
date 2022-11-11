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
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "game.h"
#include "utils/list.h"
#include "utils/utils.h"
#include "world/map/tile.h"
#include "world/map/map.h"
#include "world/entity/entity.h"
#include "world/entity/player.h"

#define PLAYER_DEFAULT_VELOCITY 5

static void update(unsigned entity, entity_list_t *entities, map_t *map);
static void draw(entity_t *player, Rectangle camera);
static void destroy(entity_t *player);

static FILE *player_goto_section(void);

player_t *player_create(Vector2 position)
{
    player_t *player = malloc(sizeof(player_t));

    player->base.position = position;
    player->base.velocity = PLAYER_DEFAULT_VELOCITY;

    player->base.attack = 20;
    player->base.defense = 20;

    player->base.draw = draw;
    player->base.update = update;
    player->base.destroy = destroy;

    player->base.hearts = 100;
    player->base.max_hearts = 100;

    player->base.state = ENTITY_STATE_IDLE;

    player->spritesheet.moving = game_get_texture("player-moving");
    player->spritesheet.idle = game_get_texture("player-idle");

    return player;
}

bool player_load(player_t *player)
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
                fread(&player->base.position, sizeof(Vector2), 1, file);
            else if (strcmp(token, "Velocity") == 0)
                fread(&player->base.velocity, sizeof(float), 1, file);
            else if (strcmp(token, "Direction") == 0)
                fread(&player->base.direction, sizeof(float), 1, file);
            else if (strcmp(token, "Hearts") == 0)
                fread(&player->base.hearts, sizeof(float), 1, file);
            else if (strcmp(token, "MaxHearts") == 0)
                fread(&player->base.max_hearts, sizeof(float), 1, file);
            else if (strcmp(token, "Attack") == 0)
                fread(&player->base.attack, sizeof(float), 1, file);
            else if (strcmp(token, "Defense") == 0)
                fread(&player->base.defense, sizeof(float), 1, file);
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
    fwrite(&player->base.position, sizeof(Vector2), 1, file);
    fprintf(file, "\n");

    fprintf(file, "Velocity ");
    fwrite(&player->base.velocity, sizeof(float), 1, file);
    fprintf(file, "\n");

    fprintf(file, "Direction ");
    fwrite(&player->base.direction, sizeof(float), 1, file);
    fprintf(file, "\n");

    fprintf(file, "Hearts ");
    fwrite(&player->base.hearts, sizeof(float), 1, file);
    fprintf(file, "\n");

    fprintf(file, "MaxHearts ");
    fwrite(&player->base.max_hearts, sizeof(float), 1, file);
    fprintf(file, "\n");

    fprintf(file, "Attack ");
    fwrite(&player->base.attack, sizeof(float), 1, file);
    fprintf(file, "\n");

    fprintf(file, "Defense ");
    fwrite(&player->base.defense, sizeof(float), 1, file);
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

static void update(unsigned entity, entity_list_t *entities, map_t *map)
{
    entity_t *base = list_get(*entities, entity);
    entity_t *enemy;
    player_t *player = (player_t *) base;

    Vector2 next_position = base->position;

    Vector2 bounds[4] = {
        { 0, 0 },
        { ENTITY_TILE_SIZE / TILE_DRAW_SIZE, 0 },
        { 0, ENTITY_TILE_SIZE / TILE_DRAW_SIZE },
        { ENTITY_TILE_SIZE / TILE_DRAW_SIZE, ENTITY_TILE_SIZE / TILE_DRAW_SIZE },
    };

    switch (base->state) {
    case ENTITY_STATE_MOVING:
        base->frame.max = player->spritesheet.moving.width / ENTITY_SPRITE_SIZE;

        next_position.x += cos(base->direction) * base->velocity
            * GetFrameTime();

        next_position.y += sin(base->direction) * base->velocity
            * GetFrameTime();

        for (int i = 0; i < 4; i++) {
            for (int layer = 0; layer < MAP_MAX_LAYERS; layer++) {
                if (tile_collision(map_tile(map, layer,
                                next_position.x + bounds[i].x,
                                base->position.y + bounds[i].y)))
                    next_position.x = base->position.x;

                if (tile_collision(map_tile(map, layer,
                                base->position.x + bounds[i].x,
                                next_position.y + bounds[i].y)))
                    next_position.y = base->position.y;
            }
        }

        for (unsigned i = 1; i < list_size(*entities); i++) {
            enemy = list_get(*entities, i);

            if (CheckCollisionRecs((Rectangle) {
                    next_position.x, next_position.y,
                    bounds[3].x, bounds[3].y }, (Rectangle) {
                        enemy->position.x, enemy->position.y,
                        bounds[3].x, bounds[3].y })) {
                next_position = base->position;
            }
        }

        break;

    case ENTITY_STATE_IDLE:
        base->frame.max = player->spritesheet.idle.width / ENTITY_SPRITE_SIZE;
        break;
    }

    if (next_position.x < 0)
        next_position.x = 0;

    if (next_position.y < 0)
        next_position.y = 0;

    base->position = next_position;
}

static void draw(entity_t *base, Rectangle camera)
{
    player_t *player = (player_t *) base;

    Texture spritesheet;

    Rectangle sprite = {
        .x = base->frame.current * ENTITY_SPRITE_SIZE,
        .y = 0,

        .width = ENTITY_SPRITE_SIZE,
        .height = ENTITY_SPRITE_SIZE,
    };

    Rectangle tile = {
        .x = (base->position.x - camera.x) * TILE_DRAW_SIZE,
        .y = (base->position.y - camera.y) * TILE_DRAW_SIZE,

        .width = ENTITY_TILE_SIZE,
        .height = ENTITY_TILE_SIZE,
    };

    Rectangle heart_bar_rect = {
        .x = (tile.x + tile.width / 2) - ENTITY_HEART_BAR_WIDTH / 2,
        .y = tile.y - ENTITY_HEART_BAR_HEIGHT * 1.2,

        .height = ENTITY_HEART_BAR_HEIGHT,
    };

    switch (base->state) {
    case ENTITY_STATE_MOVING:
        spritesheet = player->spritesheet.moving;
        break;

    case ENTITY_STATE_IDLE:
        spritesheet = player->spritesheet.idle;
        break;
    }

    if (base->hearts < base->max_hearts) {
        heart_bar_rect.width = (base->hearts / base->max_hearts)
            * ENTITY_HEART_BAR_WIDTH;

        DrawRectangleRec(heart_bar_rect, RED);

        heart_bar_rect.width = ENTITY_HEART_BAR_WIDTH;
        DrawRectangleLinesEx(heart_bar_rect, 1, BLACK);
    }

    if (base->direction > deg2rad(90) && base->direction < deg2rad(270))
        sprite.width = -sprite.width;

    DrawTexturePro(spritesheet, sprite, tile, (Vector2) { 0, 0 }, 0, WHITE);
}

static void destroy(entity_t *player)
{
    free((player_t *) player);
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

