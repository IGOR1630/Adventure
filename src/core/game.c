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
#include "core/game.h"
#include "core/scene.h"
#include "utils/hash.h"
#include "utils/list.h"
#include "utils/utils.h"

static struct {
    struct {
        hash(scene_t) list;

        scene_t current;
        scene_t pending;

        scene_data_t *data;
    } scene;

    struct {
        int width;
        int height;
        float scale;

        Vector2 mouse_factor;

        RenderTexture target;
        Rectangle target_source;
        Rectangle target_destination;
    } rendering;

    struct {
        list(int) current;
        list(int) previous;
    } touches;

    hash(Texture) textures;
    hash(Font) fonts;

    bool running;
} g_game;

bool game_init(int width, int height)
{
    // Clear the game state
    memset(&g_game, 0, sizeof g_game);

    // Initialize the scenes state
    hash_create(g_game.scene.list);

    g_game.scene.current = (scene_t) { NULL, NULL, NULL, NULL, NULL };
    g_game.scene.pending = (scene_t) { NULL, NULL, NULL, NULL, NULL };

    g_game.scene.data = NULL;

    // Initialize the resources containers
    hash_create(g_game.textures);
    hash_create(g_game.fonts);

    // Initialize the touch event container
    list_create(g_game.touches.current);
    list_create(g_game.touches.previous);

    // Initialize the raylib
    InitWindow(0, 0, "Game");
    SetTargetFPS(60);
    ToggleFullscreen();

    // Setup the rendering area dimensions and scale
    g_game.rendering.width = width;
    g_game.rendering.height = height;
    g_game.rendering.scale = min((float) GetScreenWidth() / width,
        (float) GetScreenHeight() / height);

    // Setup the mouse to be relative to rendering area
    g_game.rendering.mouse_factor = (Vector2) {
        (GetScreenWidth() - g_game.rendering.width * g_game.rendering.scale) / 2.,
        (GetScreenHeight() - g_game.rendering.height * g_game.rendering.scale) / 2.,
    };

    // Create the rendering area buffer
    g_game.rendering.target = LoadRenderTexture(width, height);
    SetTextureFilter(g_game.rendering.target.texture, TEXTURE_FILTER_BILINEAR);

    // Setup the rendering area scaled region
    g_game.rendering.target_source = (Rectangle) { 0, 0, width, -height };
    g_game.rendering.target_destination = (Rectangle) {
        (GetScreenWidth() - (width * g_game.rendering.scale)) / 2.0,
        (GetScreenHeight() - (height * g_game.rendering.scale)) / 2.0,

        width * g_game.rendering.scale,
        height * g_game.rendering.scale,
    };

    // Finish the initialization process
    g_game.running = false;
    return true;
}

void game_deinit(void)
{
    // Unload game resources
    /// Textures
    for (unsigned int i = 0; i < hash_size(g_game.textures); i++)
        UnloadTexture(g_game.textures.values[i]);

    /// Fonts
    for (unsigned int i = 0; i < hash_size(g_game.fonts); i++)
        UnloadFont(g_game.fonts.values[i]);

    // Delete resources containers
    hash_destroy(g_game.textures);
    hash_destroy(g_game.scene.list);

    // Delete touch event containers
    list_destroy(g_game.touches.current);
    list_destroy(g_game.touches.previous);

    // Delete rendering area buffer
    UnloadRenderTexture(g_game.rendering.target);

    // De-initialize raylib
    CloseWindow();
}

void game_scene_register(scene_t scene)
{
    hash_add(g_game.scene.list, scene.name, scene);
}

bool game_scene_make_current(const char *scene_name)
{
    scene_t scene = (scene_t) { NULL, NULL, NULL, NULL, NULL };

    if (scene_name == NULL)
        return false;
    else if (strcmp(scene_name, g_game.scene.current.name) == 0)
        return false;

    // Try fetch a scene by your name
    hash_get(g_game.scene.list, scene_name, scene);
    if (scene.name == NULL)
        return false;

    // If successfully found the scene make the game pending the scene change if
    // a scene was already has a current scene
    if (g_game.scene.current.name == NULL)
        g_game.scene.current = scene;
    else
        g_game.scene.pending = scene;

    return true;
}

scene_t game_scene_get_current(void)
{
    return g_game.scene.current;
}

void game_start_run(void)
{
    int touch_count = -1;

    if (g_game.scene.current.name != NULL)
        g_game.running = true;

    while (g_game.running && g_game.scene.current.name != NULL) {
        // If the number of touches change because this means that a touch event
        // was generated
        if (touch_count != GetTouchPointCount()) {
            touch_count = GetTouchPointCount();

            // Update the previous touches list with the current list and clear
            // the current list
            list_copy(g_game.touches.current, g_game.touches.previous);
            list_empty(g_game.touches.current);

            // Update the current touches list
            for (int i = 0; i < touch_count; i++)
                list_add(g_game.touches.current, GetTouchPointId(i));
        }

        // Check if a scene change is pending, if does, change the scene
        if (g_game.scene.pending.name != NULL) {
            g_game.scene.current.deinit(g_game.scene.data);
            g_game.scene.data = g_game.scene.pending.init();

            g_game.scene.current = g_game.scene.pending;

            // Reset the scene pending state
            g_game.scene.pending = (scene_t) { NULL, NULL, NULL, NULL, NULL };
        }

        // Update the game scene logic
        g_game.scene.current.update(g_game.scene.data);

        // Draw the game scene on rendering area buffer
        BeginTextureMode(g_game.rendering.target);
        g_game.scene.current.draw(g_game.scene.data);
        EndTextureMode();

        // Draw the rendering area buffer on screen buffer
        BeginDrawing();
        ClearBackground(BLACK);
        DrawTexturePro(g_game.rendering.target.texture,
            g_game.rendering.target_source, g_game.rendering.target_destination,
            (Vector2) { 0, 0 }, 0, WHITE);
        EndDrawing();
    }

    g_game.running = false;
}

bool game_is_running(void)
{
    return g_game.running;
}

void game_end_run(void)
{
    g_game.running = false;
}

int game_get_width(void)
{
    return g_game.rendering.width;
}

int game_get_height(void)
{
    return g_game.rendering.height;
}

const char *game_get_platform(void)
{
#if defined(__ANDROID__)
    return "Android";
#elif defined(_WIN32)
    return "Windows";
#elif defined(__APPLE__)
    return "MacOS";
#elif defined(__linux__)
    return "Linux";
#else
    return "Unknown";
#endif
}

/*
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
*/

Vector2 game_mouse_get_position(void)
{
    Vector2 mouse = GetMousePosition();

    // Scale the mouse that is relative to the screen to be relative to the
    // rendering area
    mouse.x = (mouse.x - g_game.rendering.mouse_factor.x) / g_game.rendering.scale;
    mouse.y = (mouse.y - g_game.rendering.mouse_factor.y) / g_game.rendering.scale;

    return mouse;
}

bool game_mouse_is_down(int mouse_button)
{
    return IsMouseButtonDown(mouse_button);
}

bool game_mouse_is_up(int mouse_button)
{
    return IsMouseButtonUp(mouse_button);
}

bool game_mouse_is_pressed(int mouse_button)
{
    return IsMouseButtonPressed(mouse_button);
}

bool game_mouse_is_released(int mouse_button)
{
    return IsMouseButtonReleased(mouse_button);
}

Vector2 game_touch_get_position(int touch_number)
{
    Vector2 touch = GetTouchPosition(touch_number);

    // Scale the touch that is relative to the screen to be relative to the
    // rendering area
    touch.x = (touch.x - g_game.rendering.mouse_factor.x) / g_game.rendering.scale;
    touch.y = (touch.y - g_game.rendering.mouse_factor.y) / g_game.rendering.scale;

    return touch;
}

int game_touch_get_id(int touch_number)
{
    return GetTouchPointId(touch_number);
}

int game_touch_get_points_count(void)
{
    return GetTouchPointCount();
}

bool game_touch_is_down(int touch_id)
{
    for (unsigned i = 0; i < list_size(g_game.touches.current); i++)
        if (touch_id == list_get(g_game.touches.current, i))
            return true;

    return false;
}

bool game_touch_is_up(int touch_id)
{
    for (unsigned i = 0; i < list_size(g_game.touches.current); i++)
        if (touch_id == list_get(g_game.touches.current, i))
            return false;

    return true;
}

bool game_touch_is_pressed(int touch_id)
{
    for (unsigned i = 0; i < list_size(g_game.touches.previous); i++)
        if (touch_id == list_get(g_game.touches.previous, i))
            return false;

    for (unsigned i = 0; i < list_size(g_game.touches.current); i++)
        if (touch_id == list_get(g_game.touches.current, i))
            return true;

    return false;
}

bool game_touch_is_released(int touch_id)
{
    for (unsigned i = 0; i < list_size(g_game.touches.current); i++)
        if (touch_id == list_get(g_game.touches.current, i))
            return false;

    for (unsigned i = 0; i < list_size(g_game.touches.previous); i++)
        if (touch_id == list_get(g_game.touches.previous, i))
            return true;

    return false;
}

Texture game_texture_load(const char *filename, const char *name)
{
    Texture texture = LoadTexture(filename);
    hash_add(g_game.textures, name, texture);

    return texture;
}

Texture game_texture_get(const char *name)
{
    Texture texture = (Texture) { 0 };
    hash_get(g_game.textures, name, texture);

    return texture;
}

Font game_font_load(const char *filename, const char *name)
{
    Font font = LoadFont(filename);
    hash_add(g_game.fonts, name, font);

    return font;
}

Font game_font_get(const char *name)
{
    Font font = (Font) { 0 };
    hash_get(g_game.fonts, name, font);

    return font;
}

