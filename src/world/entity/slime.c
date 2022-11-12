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
#include <stdbool.h>
#include <stdlib.h>
#include "raylib.h"
#include "game.h"
#include "utils/list.h"
#include "utils/utils.h"
#include "world/map/tile.h"
#include "world/map/map.h"
#include "world/entity/entity.h"

#define SLIME_PLAYER_UNTARGET_RADIUS 12
#define SQ(x) ((x) * (x))

typedef struct {
    entity_t base;

    struct {
        float field;
        float radius;
        bool target_player;
    } view;

    struct {
        Texture spawn;
        Texture moving;
        Texture damaging;
        Texture idle;
    } spritesheet;
} slime_t;

static void update(unsigned entity, entity_list_t *entities, map_t *map);
static void draw(entity_t *entity, Rectangle camera);
static void destroy(entity_t *entity);

entity_t *slime_create(Vector2 position)
{
    slime_t *slime = malloc(sizeof(slime_t));

    slime->base.draw = draw;
    slime->base.update = update;
    slime->base.destroy = destroy;

    slime->base.position = position;
    slime->base.velocity = 4;
    slime->base.direction = deg2rad(rand() % 360);

    slime->base.hearts = 30;
    slime->base.max_hearts = 30;

    slime->base.attack = 10;
    slime->base.defense = 5;

    slime->base.frame.current = 0;
    slime->base.frame.delay = GetTime();
    slime->base.frame.max = 0;

    slime->base.state = ENTITY_STATE_SPAWN;

    slime->view.field = deg2rad(60);
    slime->view.radius = 4;
    slime->view.target_player = false;

    slime->spritesheet.spawn = game_get_texture("slime-spawn");
    slime->spritesheet.moving = game_get_texture("slime-moving");
    slime->spritesheet.idle = game_get_texture("slime-idle");
    slime->spritesheet.damaging = game_get_texture("slime-damaging");

    return (entity_t *) slime;
}

static void update(unsigned entity, entity_list_t *entities, map_t *map)
{
    entity_t *base = list_get(*entities, entity);
    slime_t *slime = (slime_t *) base;

    entity_t *player = list_get(*entities, 0);

    Vector2 next_position = base->position;

    Vector2 bounds[4] = {
        { 0, 0 },
        { ENTITY_TILE_SIZE / TILE_DRAW_SIZE, 0 },
        { 0, ENTITY_TILE_SIZE / TILE_DRAW_SIZE },
        { ENTITY_TILE_SIZE / TILE_DRAW_SIZE, ENTITY_TILE_SIZE / TILE_DRAW_SIZE },
    };

    float radius;
    float angle = vec2ang(player->position.x - base->position.x,
        player->position.y - base->position.y);

    float start_angle = base->direction - slime->view.field / 2.0;
    float end_angle = base->direction + slime->view.field / 2.0;

    static float hit_player = 0;

    if (end_angle > UTILS_PI * 2)
        end_angle -= UTILS_PI * 2;

    if (start_angle < 0)
        start_angle += UTILS_PI * 2;

    switch (base->state) {
    case ENTITY_STATE_SPAWN:
        base->frame.max = slime->spritesheet.spawn.width / ENTITY_SPRITE_SIZE;

        if (base->frame.current + 1 == base->frame.max) {
            base->state = ENTITY_STATE_IDLE;
            base->frame.current = 0;
        }

        break;

    case ENTITY_STATE_MOVING:
        base->frame.max = slime->spritesheet.moving.width / ENTITY_SPRITE_SIZE;

        if (base->frame.current == 0) {
            if (slime->view.target_player)
                base->direction = angle;
            else if (rand() / (double) RAND_MAX <= 0.5)
                base->direction = deg2rad(rand() % 360);
        } else if (base->frame.current > 3) {
            next_position.x += base->velocity * cos(base->direction)
                * GetFrameTime();

            next_position.y += base->velocity * sin(base->direction)
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

            // Attack the player
            if (CheckCollisionRecs((Rectangle) {
                        next_position.x, next_position.y,
                        bounds[3].x, bounds[3].y }, (Rectangle) {
                            player->position.x, player->position.y,
                            bounds[3].x, bounds[3].y })) {
                player->state = ENTITY_STATE_DAMAGING;
                player->frame.current = 0;

                player->damage_direction = base->direction;

                if (hit_player == 0) {
                    player->hearts -= max((base->attack - player->defense)
                        * (rand() % 2), 5);

                    base->state = ENTITY_STATE_IDLE;
                    base->frame.current = 0;

                    hit_player = GetTime();
                }

                next_position = base->position;
            }
        }

        if (base->frame.current + 1 == base->frame.max) {
            base->state = ENTITY_STATE_IDLE;
            base->frame.current = 0;
        }

        base->position = next_position;

        break;

    case ENTITY_STATE_DAMAGING:
        base->frame.max = slime->spritesheet.damaging.width / ENTITY_SPRITE_SIZE;

        next_position.x += (base->velocity / 3) * cos(base->damage_direction)
            * GetFrameTime();
        next_position.y += (base->velocity / 3) * sin(base->damage_direction)
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

        base->position = next_position;

        if (base->frame.current + 1 == base->frame.max) {
            base->state = ENTITY_STATE_IDLE;
            base->frame.current = 0;
        }

        break;

    case ENTITY_STATE_IDLE:
        base->frame.max = slime->spritesheet.idle.width / ENTITY_SPRITE_SIZE;

        if (((rand() / (double) RAND_MAX) <= 0.008 || slime->view.target_player)
                && hit_player == 0) {
            base->state = ENTITY_STATE_MOVING;
            base->frame.current = 0;
        }

        break;
    }

    if (hit_player > 0 && GetTime() - hit_player >= 0.3)
        hit_player = 0;

    // Player targeting
    for (int i = 0; i < 4; i++) {
        radius = SQ(player->position.x + bounds[i].x - base->position.x)
            + SQ(player->position.y + bounds[i].y - base->position.y);

        if (radius <= SQ(slime->view.radius)) {
            if (i > 0)
                angle = vec2ang(player->position.x + bounds[i].x - base->position.x,
                    player->position.y + bounds[i].y - base->position.y);

            if (start_angle > end_angle
                    && ((start_angle < angle && angle < UTILS_PI * 2)
                        || (angle < end_angle))) {
                slime->view.target_player = true;
                break;
            } else if (start_angle < angle && angle < end_angle) {
                slime->view.target_player = true;
                break;
            }
        } else if (radius >= SQ(SLIME_PLAYER_UNTARGET_RADIUS)) {
            slime->view.target_player = false;
        }
    }
}

static void draw(entity_t *base, Rectangle camera)
{
    slime_t *slime = (slime_t *) base;

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
        spritesheet = slime->spritesheet.spawn;
        break;

    case ENTITY_STATE_MOVING:
        spritesheet = slime->spritesheet.moving;
        break;

    case ENTITY_STATE_DAMAGING:
        spritesheet = slime->spritesheet.damaging;
        break;

    case ENTITY_STATE_IDLE:
        spritesheet = slime->spritesheet.idle;
        break;
    }

    if (base->direction > deg2rad(90) && base->direction < deg2rad(270))
        sprite.width = -sprite.width;

    if (base->hearts < base->max_hearts) {
        heart_bar_rect.width = (base->hearts / base->max_hearts)
            * ENTITY_HEART_BAR_WIDTH;

        DrawRectangleRec(heart_bar_rect, RED);

        heart_bar_rect.width = ENTITY_HEART_BAR_WIDTH;
        DrawRectangleLinesEx(heart_bar_rect, 1, BLACK);
    }

    DrawTexturePro(spritesheet, sprite, tile, (Vector2) { 0, 0 }, 0, WHITE);
}

static void destroy(entity_t *entity)
{
    free((slime_t *) entity);
}

