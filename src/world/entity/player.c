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

    player->attacked = false;
    player->attacking = 0;

    player->spritesheet.moving = game_get_texture("player-moving");
    player->spritesheet.damaging = game_get_texture("player-damaging");
    player->spritesheet.idle = game_get_texture("player-idle");
    player->spritesheet.sword = game_get_texture("player-sword");

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

    Rectangle player_rect;
    Rectangle enemy_rect;

    Vector2 next_position = base->position;

    Vector2 bounds[4] = {
        { 0, 0 },
        { ENTITY_TILE_SIZE / TILE_DRAW_SIZE, 0 },
        { 0, ENTITY_TILE_SIZE / TILE_DRAW_SIZE },
        { ENTITY_TILE_SIZE / TILE_DRAW_SIZE, ENTITY_TILE_SIZE / TILE_DRAW_SIZE },
    };

    static float hitted = 0;

    if (player->attacked)
        player->attacking = 0;

    if (player->attacking > 0 && GetTime() - player->attacking >= 0.05) {
        player->attacked = true;
        player->attacking = 0;
    }

    switch (base->state) {
    case ENTITY_STATE_SPAWN:
        break;

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

        break;

    case ENTITY_STATE_DAMAGING:
        base->frame.max = player->spritesheet.damaging.width / ENTITY_SPRITE_SIZE;

        next_position.x += cos(base->damage_direction) * (base->velocity / 2)
            * GetFrameTime();

        next_position.y += sin(base->damage_direction) * (base->velocity / 2)
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

        if (base->frame.current + 1 == base->frame.max) {
            base->state = ENTITY_STATE_IDLE;
            base->frame.current = 0;
        }

        break;

    case ENTITY_STATE_IDLE:
        base->frame.max = player->spritesheet.idle.width / ENTITY_SPRITE_SIZE;
        break;
    }

    for (unsigned i = 1; i < list_size(*entities); i++) {
        enemy = list_get(*entities, i);

        player_rect = (Rectangle) {
            .x = next_position.x,
            .y = next_position.y,

            .width = bounds[3].x,
            .height = bounds[3].y,
        };

        enemy_rect = (Rectangle) {
            .x = enemy->position.x,
            .y = enemy->position.y,

            .width = bounds[3].x,
            .height = bounds[3].y,
        };

        if (base->state == ENTITY_STATE_MOVING
                && CheckCollisionRecs(player_rect, enemy_rect)) {
            base->state = ENTITY_STATE_DAMAGING;
            base->frame.current = 0;

            base->damage_direction = base->direction + UTILS_PI;
            if (base->damage_direction > UTILS_PI * 2)
                base->damage_direction -= UTILS_PI * 2;

            if (hitted == 0) {
                base->hearts -= max((enemy->attack - base->defense)
                    * (rand() % 2), 5);

                hitted = GetTime();
            }

            next_position = base->position;
        }

        player_rect.x += cos(base->direction) * player_rect.width;
        player_rect.y += sin(base->direction) * player_rect.height;

        if (player->attacking > 0 && CheckCollisionRecs(player_rect,
                    enemy_rect)) {
            enemy->state = ENTITY_STATE_DAMAGING;
            enemy->frame.current = 0;

            enemy->hearts -= max((base->attack - enemy->defense)
                * (rand() % 2), 5);

            enemy->damage_direction = base->direction;

            if ((pointing_left(base->direction)
                    && pointing_left(enemy->direction))
                    || (pointing_right(base->direction)
                        && pointing_right(enemy->direction))) {
                enemy->direction += UTILS_PI;

                if (enemy->direction > UTILS_PI * 2)
                    enemy->direction -= UTILS_PI * 2;
            }

            player->attacked = true;
        }
    }

    if (hitted > 0 && GetTime() - hitted >= 0.3)
        hitted = 0;

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
    case ENTITY_STATE_SPAWN:
        break;

    case ENTITY_STATE_MOVING:
        spritesheet = player->spritesheet.moving;
        break;

    case ENTITY_STATE_DAMAGING:
        spritesheet = player->spritesheet.damaging;
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

    if (player->attacking) {
        // Disable the horizontal flip of the sword
        sprite.width = fabs(sprite.width);

        // Flip vertically the sword sprite
        if (base->direction > deg2rad(90) && base->direction < deg2rad(270))
            sprite.height = -sprite.height;

        tile.x += tile.width / 2 + cos(base->direction) * tile.width;
        tile.y += tile.height / 2 + sin(base->direction) * tile.height;

        DrawTexturePro(player->spritesheet.sword, sprite, tile,
            (Vector2) { tile.width / 2, tile.height / 2 },
            rad2deg(base->direction), WHITE);
    }
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

