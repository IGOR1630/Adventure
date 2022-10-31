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
#include <stdio.h>
#include <string.h>
#include "game.h"
#include "utils/list.h"
#include "world/entity/spawner.h"

static FILE *spawner_goto_section(void);

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
                fread(&spawner, sizeof(spawner_t), 1, file);
                fgetc(file);

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

void spawner_new(spawner_list_t *spawners, Vector2 position)
{
    spawner_t spawner = (spawner_t) {
        .position = position,

        .spawn_entities = rand() % 10,
        .spawn_radius = 15,
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
        fwrite(&list_get(*spawners, i), sizeof(spawner_t), 1, file);
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

