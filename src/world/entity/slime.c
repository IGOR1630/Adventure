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

typedef struct {
    entity_t base;

    struct {
        float field;
        float radius;
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
    slime->base.velocity = 3;
    slime->base.direction = 0;

    slime->base.frame.current = 0;
    slime->base.frame.delay = GetTime();
    slime->base.frame.max = 0;

    slime->base.state = ENTITY_STATE_SPAWN;

    slime->view.field = deg2rad(90);
    slime->view.radius = 4;

    slime->spritesheet.spawn = game_get_texture("slime-spawn");
    slime->spritesheet.moving = game_get_texture("slime-moving");
    slime->spritesheet.idle = game_get_texture("slime-idle");

    return (entity_t *) slime;
}

static void update(unsigned entity, entity_list_t *entities, map_t *map)
{
    entity_t *base = list_get(*entities, entity);
    slime_t *slime = (slime_t *) base;

    switch (base->state) {
    case ENTITY_STATE_SPAWN:
        base->frame.max = slime->spritesheet.spawn.width / ENTITY_SPRITE_SIZE;

        if (base->frame.current + 1 == base->frame.max)
            base->state = ENTITY_STATE_IDLE;

        break;

    case ENTITY_STATE_MOVING:
        base->frame.max = slime->spritesheet.moving.width / ENTITY_SPRITE_SIZE;

        if (base->frame.current == 0) {
            base->direction = deg2rad(rand() % 360);
        } else if (base->frame.current > 3) {
            base->position.x += base->velocity * cos(base->direction)
                * GetFrameTime();
            base->position.y += base->velocity * sin(base->direction)
                * GetFrameTime();
        }

        if (base->frame.current + 1 == base->frame.max)
            base->state = ENTITY_STATE_IDLE;

        break;

    case ENTITY_STATE_IDLE:
        base->frame.max = slime->spritesheet.idle.width / ENTITY_SPRITE_SIZE;

        if ((rand() / (double) RAND_MAX) <= 0.008) {
            base->state = ENTITY_STATE_MOVING;
            base->frame.current = 0;
        }

        break;
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

