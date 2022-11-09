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

    return (entity_t *) slime;
}

static void update(unsigned entity, entity_list_t *entities, map_t *map)
{
#define MAP_COLLISION_X(x, y, bound, tile)                                     \
    (tile_equal(map->tiles[0][(int) (y)][(int) (x)], tile)                     \
        && tile_equal(map->tiles[0][(int) ((y) + (bound))][(int) (x)], tile))

#define MAP_COLLISION_Y(x, y, bound, tile)                                     \
    (tile_equal(map->tiles[0][(int) (y)][(int) (x)], tile)                     \
        && tile_equal(map->tiles[0][(int) (y)][(int) ((x) + (bound))], tile))

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

    if (end_angle > M_PI * 2)
        end_angle -= M_PI * 2;

    if (start_angle < 0)
        start_angle += M_PI * 2;

    switch (base->state) {
    case ENTITY_STATE_SPAWN:
        base->frame.max = slime->spritesheet.spawn.width / ENTITY_SPRITE_SIZE;

        if (base->frame.current + 1 == base->frame.max)
            base->state = ENTITY_STATE_IDLE;

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
            if (!MAP_COLLISION_X(next_position.x, base->position.y, bounds[3].x,
                        tile_new(11, 2))
                    || !MAP_COLLISION_X(next_position.x + bounds[3].x,
                        base->position.y, bounds[3].x, tile_new(11, 2)))
                next_position.x = base->position.x;

            next_position.y += base->velocity * sin(base->direction)
                * GetFrameTime();
            if (!MAP_COLLISION_Y(base->position.x, next_position.y, bounds[3].y,
                        tile_new(11, 2))
                    || !MAP_COLLISION_Y(base->position.x,
                        next_position.y + bounds[3].y, bounds[3].y, tile_new(11, 2)))
                next_position.y = base->position.y;
        }

        if (base->frame.current + 1 == base->frame.max)
            base->state = ENTITY_STATE_IDLE;

        base->position = next_position;

        break;

    case ENTITY_STATE_IDLE:
        base->frame.max = slime->spritesheet.idle.width / ENTITY_SPRITE_SIZE;

        if ((rand() / (double) RAND_MAX) <= 0.008 || slime->view.target_player) {
            base->state = ENTITY_STATE_MOVING;
            base->frame.current = 0;
        }

        break;
    }

    // Player targeting
    for (int i = 0; i < 4; i++) {
        radius = SQ(player->position.x + bounds[i].x - base->position.x)
            + SQ(player->position.y + bounds[i].y - base->position.y);

        if (radius <= SQ(slime->view.radius)) {
            if (i > 0)
                angle = vec2ang(player->position.x + bounds[i].x - base->position.x,
                    player->position.y + bounds[i].y - base->position.y);

            if (start_angle > end_angle
                    && ((start_angle < angle && angle < M_PI * 2)
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

    switch (base->state) {
    case ENTITY_STATE_SPAWN:
        spritesheet = slime->spritesheet.spawn;
        break;

    case ENTITY_STATE_MOVING:
        spritesheet = slime->spritesheet.moving;
        break;

    case ENTITY_STATE_IDLE:
        spritesheet = slime->spritesheet.idle;
        break;
    }

    if (base->direction > deg2rad(90) && base->direction < deg2rad(270))
        sprite.width = -sprite.width;

    DrawTexturePro(spritesheet, sprite, tile, (Vector2) { 0, 0 }, 0, WHITE);
}

static void destroy(entity_t *entity)
{
    free((slime_t *) entity);
}

