/*
The GPLv3 License (GPLv3)

Copyright (c) 2022 Jonatha Gabriel.

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
#include <string.h>
#include "raylib.h"
#include "game.h"
#include "scene.h"
#include "utils/hash.h"

static struct {
    struct {
        hash(scene_t) list;

        scene_t       current;
        scene_data_t *data;
    } scene;

    bool running;
} g_game;

bool game_init(void)
{
    memset(&g_game, 0, sizeof g_game);

    hash_create(g_game.scene.list);
    g_game.scene.current = (scene_t) { NULL, NULL, NULL, NULL, NULL };
    g_game.scene.data = NULL;

    g_game.running = false;

    InitWindow(0, 0, "Game");
    SetTargetFPS(60);
    ToggleFullscreen();

    return true;
}

void game_deinit(void)
{
    hash_destroy(g_game.scene.list);

    CloseWindow();
}


void game_register_scene(scene_t scene)
{ hash_add(g_game.scene.list, scene.name, scene); }

void game_set_scene(const char *scene_name)
{
    scene_t scene = (scene_t) { NULL, NULL, NULL, NULL, NULL };

    if (g_game.scene.current.name != NULL) {
        if (g_game.scene.current.name == scene_name)
            return;

        g_game.scene.current.deinit(g_game.scene.data);
    }

    if (scene_name != NULL)
        hash_get(g_game.scene.list, scene_name, scene);

    g_game.scene.current = scene;
    if (g_game.scene.current.name != NULL)
        g_game.scene.data = g_game.scene.current.init();
}

scene_t game_current_scene(void)
{ return g_game.scene.current; }


bool game_is_running(void)
{ return g_game.running; }

void game_end_run(void)
{ g_game.running = false; }


void game_run(void)
{
    if (g_game.scene.current.name != NULL)
        g_game.running = true;

    while (g_game.running) {
        g_game.scene.current.update(g_game.scene.data);

        BeginDrawing();
        g_game.scene.current.draw(g_game.scene.data);
        EndDrawing();
    }
}

