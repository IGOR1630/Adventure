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
#include "raylib.h"
#include "game.h"
#include "world/map/tile.h"
#include "world/entity/entity.h"

typedef struct {
    entity_t base;

    float move_rate;
} slime_t;

static void update(entity_t *entity);
static void draw(entity_t *entity, Rectangle camera);
static void destroy(entity_t *entity);

entity_t *slime_create(Vector2 position)
{
    slime_t *slime = malloc(sizeof(slime_t));

    slime->base.draw = draw;
    slime->base.update = update;
    slime->base.destroy = destroy;

    slime->base.position = position;

    slime->base.frame.current = 0;
    slime->base.frame.delay = GetTime();
    slime->base.frame.max = 0;

    slime->move_rate = 0;

    return (entity_t *) slime;
}

static void update(entity_t *entity)
{
    slime_t *slime = (slime_t *) entity;

    if (entity->is_moving) {
        if (rand() / (double) RAND_MAX >= slime->move_rate) {
            entity->is_moving = false;
            slime->move_rate = 0;
        } else
            slime->move_rate -= 0.01;
    } else {
        if (rand() / (double) RAND_MAX <= slime->move_rate) {
            entity->is_moving = true;
            slime->move_rate = 0;
        } else
            slime->move_rate += 0.01;
    }

    if (entity->is_moving) {
        entity->position.x += 0.1;
    }
}

static void draw(entity_t *entity, Rectangle camera)
{
    slime_t *slime = (slime_t *) entity;

    Rectangle draw_in = {
        .x = (entity->position.x - camera.x) * TILE_DRAW_SIZE,
        .y = (entity->position.y - camera.y) * TILE_DRAW_SIZE,

        .width = 16,
        .height = 16,
    };

    DrawRectangleRec(draw_in, BLUE);
}

static void destroy(entity_t *entity)
{
    free((slime_t *) entity);
}

