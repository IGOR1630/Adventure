/*
The GPLv3 License (GPLv3)

Copyright (c) 2022 Jonatha Gabriel, Igor Nepomuceno and Renne Bispo

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
#include "game_scene.h"


static struct {
    struct {
        int current;
        void *data;
    } scene;

    bool running;
} g_game;


bool game_init(void)
{
    memset(&g_game, 0, sizeof g_game);

    g_game.scene.current = GAME_SCENE_NONE;
    g_game.scene.data = NULL;

    g_game.running = false;

    InitWindow(0, 0, "Game");
    SetTargetFPS(60);

    game_scene_initialize();

    return true;
}

void game_deinit(void)
{
    CloseWindow();
}


void game_set_scene(game_scene_t scene)
{
    if (g_game.scene.current != GAME_SCENE_NONE)
        game_scene_deinit(g_game.scene.current, g_game.scene.data);

    g_game.scene.current = scene;

    if (g_game.scene.current != GAME_SCENE_NONE)
        g_game.scene.data = game_scene_init(g_game.scene.current);
}

game_scene_t game_get_scene(void)
{ return g_game.scene.current; }


bool game_is_running(void)
{ return g_game.running; }

void game_end_run(void)
{ g_game.running = false; }


void game_run(void)
{
    if (g_game.scene.current != GAME_SCENE_NONE)
        g_game.running = true;

    while (g_game.running) {
        game_scene_update(g_game.scene.current, g_game.scene.data);

        BeginDrawing();
        game_scene_render(g_game.scene.current, g_game.scene.data);
        EndDrawing();
    }
}

