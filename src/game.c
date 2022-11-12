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
#include <string.h>
#include "raylib.h"
#include "game.h"
#include "scene.h"
#include "utils/hash.h"
#include "utils/list.h"
#include "utils/utils.h"

static struct {
    struct {
        hash(scene_t) list;

        scene_t       current;
        scene_data_t *data;
    } scene;

    struct {
        int   width;
        int   height;
        float scale;

        Vector2 mouse_factor;

        RenderTexture target;
        Rectangle     target_source;
        Rectangle     target_destination;
    } rendering;

    struct {
        list(int) current;
        list(int) previous;
    } touches;

    hash(Texture) textures;

    bool running;
} g_game;

bool game_init(int width, int height)
{
    memset(&g_game, 0, sizeof g_game);

    hash_create(g_game.scene.list);
    g_game.scene.current = (scene_t) { NULL, NULL, NULL, NULL, NULL };
    g_game.scene.data = NULL;

    g_game.running = false;

    hash_create(g_game.textures);

    list_create(g_game.touches.current);
    list_create(g_game.touches.previous);

    InitWindow(0, 0, "Game");
    SetTargetFPS(60);
    ToggleFullscreen();
    ChangeDirectory("assets");

    g_game.rendering.width = width;
    g_game.rendering.height = height;
    g_game.rendering.scale = min((float) GetScreenWidth() / width,
        (float) GetScreenHeight() / height);

    g_game.rendering.mouse_factor = (Vector2) {
        (GetScreenWidth() - g_game.rendering.width * g_game.rendering.scale) / 2.,
        (GetScreenHeight() - g_game.rendering.height * g_game.rendering.scale) / 2.,
    };

    g_game.rendering.target = LoadRenderTexture(width, height);
    SetTextureFilter(g_game.rendering.target.texture, TEXTURE_FILTER_BILINEAR);

    g_game.rendering.target_source = (Rectangle) { 0, 0, width, -height };
    g_game.rendering.target_destination = (Rectangle) {
        (GetScreenWidth() - (width * g_game.rendering.scale)) / 2.0,
        (GetScreenHeight() - (height * g_game.rendering.scale)) / 2.0,

        width * g_game.rendering.scale,
        height * g_game.rendering.scale,
    };

    return true;
}

void game_deinit(void)
{
    for (unsigned int i = 0; i < hash_size(g_game.textures); i++)
        UnloadTexture(g_game.textures.values[i]);

    hash_destroy(g_game.textures);
    hash_destroy(g_game.scene.list);

    list_destroy(g_game.touches.current);
    list_destroy(g_game.touches.previous);

    UnloadRenderTexture(g_game.rendering.target);
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
    static int touch_count = -1;

    if (g_game.scene.current.name != NULL)
        g_game.running = true;

    while (g_game.running && g_game.scene.current.name != NULL) {
        if (touch_count != GetTouchPointCount()) {
            touch_count = GetTouchPointCount();

            list_copy(g_game.touches.current, g_game.touches.previous);
            list_empty(g_game.touches.current);

            for (int i = 0; i < touch_count; i++)
                list_add(g_game.touches.current, GetTouchPointId(i));
        }

        if (g_game.scene.current.name != NULL)
            g_game.scene.current.update(g_game.scene.data);

        BeginTextureMode(g_game.rendering.target);

        if (g_game.scene.current.name != NULL)
            g_game.scene.current.draw(g_game.scene.data);

        EndTextureMode();

        BeginDrawing();
        ClearBackground(BLACK);
        DrawTexturePro(g_game.rendering.target.texture,
            g_game.rendering.target_source, g_game.rendering.target_destination,
            (Vector2) { 0, 0 }, 0, WHITE);
        EndDrawing();
    }

    g_game.running = false;
}

int game_width(void)
{ return g_game.rendering.width; }

int game_height(void)
{ return g_game.rendering.height; }

FILE *game_file(const char *mode)
{
    char filename[200] = "../game.sav";
    FILE *file;

    if ((file = fopen(filename, mode)) == NULL) {
        sprintf(filename, "/storage/emulated/0/game.sav");
        file = fopen(filename, mode);
    }

    return file;
}

Vector2 game_virtual_mouse(void)
{
    Vector2 mouse = GetMousePosition();

    mouse.x = (mouse.x - g_game.rendering.mouse_factor.x) / g_game.rendering.scale;
    mouse.y = (mouse.y - g_game.rendering.mouse_factor.y) / g_game.rendering.scale;

    return mouse;
}

Vector2 game_virtual_touch(int touch_number)
{
    Vector2 touch = GetTouchPosition(touch_number);

    touch.x = (touch.x - g_game.rendering.mouse_factor.x) / g_game.rendering.scale;
    touch.y = (touch.y - g_game.rendering.mouse_factor.y) / g_game.rendering.scale;

    return touch;
}

int game_touch_id(int touch_number)
{ return GetTouchPointId(touch_number); }

bool game_touch_down(int touch_id)
{
    for (unsigned i = 0; i < list_size(g_game.touches.current); i++)
        if (touch_id == list_get(g_game.touches.current, i))
            return true;

    return false;
}

bool game_touch_up(int touch_id)
{
    for (unsigned i = 0; i < list_size(g_game.touches.current); i++)
        if (touch_id == list_get(g_game.touches.current, i))
            return false;

    return true;
}

bool game_touch_pressed(int touch_id)
{
    for (unsigned i = 0; i < list_size(g_game.touches.previous); i++)
        if (touch_id == list_get(g_game.touches.previous, i))
            return false;

    for (unsigned i = 0; i < list_size(g_game.touches.current); i++)
        if (touch_id == list_get(g_game.touches.current, i))
            return true;

    return false;
}

bool game_touch_released(int touch_id)
{
    for (unsigned i = 0; i < list_size(g_game.touches.current); i++)
        if (touch_id == list_get(g_game.touches.current, i))
            return false;

    for (unsigned i = 0; i < list_size(g_game.touches.previous); i++)
        if (touch_id == list_get(g_game.touches.previous, i))
            return true;

    return false;
}

Texture game_load_texture(const char *filename, const char *name)
{
    Texture texture = LoadTexture(filename);
    hash_add(g_game.textures, name, texture);

    return texture;
}

Texture game_get_texture(const char *name)
{
    Texture texture = (Texture) { 0 };
    hash_get(g_game.textures, name, texture);

    return texture;
}

