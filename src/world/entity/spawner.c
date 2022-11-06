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
#include <stdio.h>
#include <string.h>
#include "game.h"
#include "utils/list.h"
#include "utils/utils.h"
#include "world/entity/entity.h"
#include "world/entity/spawner.h"
#include "world/entity/player.h"

#define RANDINT(min, max) ((min) + rand() % ((max) - (min) + 1))
#define SPAWMER_SPAWN_RADIUS 5

static FILE *spawner_goto_section(void);
static Vector2 spawner_entity_position(Vector2 center, float radius);

void spawner_create(spawner_list_t *spawners)
{
    list_create(*spawners);
}

bool spawner_load(spawner_list_t *spawners)
{
    int c;

    char token[21];

    bool open_spawner_section = false;
    bool spawner_section_found = false;

    FILE *file;

    spawner_t spawner;

    if ((file = spawner_goto_section()) == NULL)
        return false;

    while ((c = fgetc(file)) != EOF) {
        if (c == '<') {
            fscanf(file, "%20s", token);

            if (strcmp(token, "Spawners") == 0) {
                open_spawner_section = true;
                spawner_section_found = true;
            } else if (open_spawner_section) {
                break;
            }
        } else if (c == '>') {
            fscanf(file, "%20s", token);

            if (strcmp(token, "Spawners") == 0) {
                open_spawner_section = false;
                break;
            } else if (open_spawner_section) {
                break;
            }
        } else if (open_spawner_section && isalpha(c) && isupper(c)) {
            token[0] = c;
            fscanf(file, "%19s", token + 1);

            if (strcmp(token, "Spawner") == 0) {
                fgetc(file);
                fread(&spawner.position, sizeof(Vector2), 1, file);
                fread(&spawner.spawn_min_entities, sizeof(int), 1, file);
                fread(&spawner.spawn_max_entities, sizeof(int), 1, file);
                fread(&spawner.spawn_radius, sizeof(int), 1, file);
                fgetc(file);

                spawner.spawned_entities = 0;
                list_add(*spawners, spawner);
            }
        }
    }

    if (open_spawner_section)
        spawner_destroy(spawners);

    fclose(file);
    return spawner_section_found && !open_spawner_section;
}

void spawner_destroy(spawner_list_t *spawners)
{
    list_destroy(*spawners);
}

// Placeholder
entity_t *slime_create(Vector2 position);
void spawner_update(spawner_list_t *spawners, entity_list_t *entities)
{
    spawner_t *spawner;
    entity_t  *player = list_get(*entities, 0);

    Vector2 spawn_entity_pos;

    for (unsigned i = 0; i < list_size(*spawners); i++) {
        spawner = &list_get(*spawners, i);

        if (!CheckCollisionCircles(player->position, SPAWMER_SPAWN_RADIUS,
                    spawner->position, spawner->spawn_radius))
            continue;

        // If the spawner still has entities spawned don't spawn.
        if (spawner->spawned_entities > 0)
            continue;

        spawner->spawned_entities = RANDINT(spawner->spawn_min_entities,
                spawner->spawn_max_entities);
        for (int j = 0, k = 0; j < spawner->spawned_entities; j++) {
            // This do-while will select a valid position for the new entity
            // generated.
            do {
                spawn_entity_pos = spawner_entity_position(spawner->position,
                    spawner->spawn_radius);

                for (k = list_size(*entities) - 1; k > 0; k--)
                    if (list_get(*entities, k)->spawner_id == i &&
                        CheckCollisionRecs(
                            (Rectangle) {
                                spawn_entity_pos.x,
                                spawn_entity_pos.y,
                                1, 1
                            },
                            (Rectangle) {
                                list_get(*entities, k)->position.x,
                                list_get(*entities, k)->position.y,
                                1, 1
                            })
                        )
                        break;
            } while (k > 0);

            list_add(*entities, slime_create(spawn_entity_pos));
            list_get(*entities, list_size(*entities) - 1)->spawner_id = i;
        }
    }
}

void spawner_new(spawner_list_t *spawners, Vector2 position)
{
    spawner_t spawner = (spawner_t) {
        .position = position,

        .spawn_min_entities = 3,
        .spawn_max_entities = 8,

        .spawn_radius = 3,

        .spawned_entities = 0,
    };

    list_add(*spawners, spawner);
}

bool spawner_save(spawner_list_t *spawners)
{
    FILE *file;

    if ((file = spawner_goto_section()) == NULL)
        return false;

    fprintf(file, "<Spawners\n");

    for (unsigned i = 0; i < list_size(*spawners); i++) {
        fprintf(file, "Spawner ");
        fwrite(&list_get(*spawners, i).position, sizeof(Vector2), 1, file);
        fwrite(&list_get(*spawners, i).spawn_min_entities, sizeof(int), 1, file);
        fwrite(&list_get(*spawners, i).spawn_max_entities, sizeof(int), 1, file);
        fwrite(&list_get(*spawners, i).spawn_radius, sizeof(int), 1, file);
        fprintf(file, "\n");
    }

    fprintf(file, ">Spawners\n");
    fclose(file);

    return true;
}

bool spawner_exists(void)
{
    int c;

    char token[21];

    bool open_spawner_section = false;
    bool spawner_section_found = false;

    FILE *file;

    if ((file = game_file("r")) == NULL)
        return false;

    while ((c = fgetc(file)) != EOF) {
        if (c == '<') {
            fscanf(file, "%20s", token);

            if (strcmp(token, "Spawners") == 0) {
                open_spawner_section = true;
                spawner_section_found = true;
            }
        } else if (c == '>') {
            fscanf(file, "%20s", token);

            if (strcmp(token, "Spawners") == 0) {
                open_spawner_section = false;
                break;
            }
        }
    }

    fclose(file);
    return spawner_section_found && !open_spawner_section;
}

static FILE *spawner_goto_section(void)
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

            if (strcmp(token, "Spawners") == 0) {
                fsetpos(file, &pos);
                return file;
            }
        }

        fgetpos(file, &pos);
    }

    fclose(file);
    return (file = game_file("a"));
}

static Vector2 spawner_entity_position(Vector2 center, float radius)
{
    double x, y;

    do {
        x = (rand() / (double) RAND_MAX) * 2.0 - 1.0;
        y = (rand() / (double) RAND_MAX) * 2.0 - 1.0;
    } while ((x * x) + (y * y) > 1);

    return (Vector2) {
        .x = x * radius + center.x,
        .y = y * radius + center.y,
    };
}

