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

#ifndef SPAWNER_H
#define SPAWNER_H

#include <stdbool.h>
#include "raylib.h"
#include "utils/list.h"
#include "world/entity/entity.h"

#define SPAWNER_DISTANCE_RADIUS 20

typedef struct {
    Vector2 position;

    int spawn_min_entities;
    int spawn_max_entities;

    int spawn_radius;

    int spawned_entities;
} spawner_t;

typedef list(spawner_t) spawner_list_t;

void spawner_create(spawner_list_t *spawners);
bool spawner_load(spawner_list_t *spawners);
void spawner_destroy(spawner_list_t *spawners);

void spawner_update(spawner_list_t *spawners, entity_list_t *entities);

void spawner_new(spawner_list_t *spawners, Vector2 point);

bool spawner_save(spawner_list_t *spawners);

bool spawner_exists(void);

#endif // !SPAWNER_H

