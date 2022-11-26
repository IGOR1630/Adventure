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

#ifndef GAME_H
#define GAME_H

#include <stdbool.h>
#include <stdio.h>
#include "raylib.h"
#include "core/scene.h"

/* game_init
 *
 * Initialize the game with window dimensions as \width and \height.
 *
 * Return:
 *  true on success initialization
 *  false otherwise
 */
bool game_init(int width, int height);

/* game_deinit
 *
 * De-initialize the game.
 */
void game_deinit(void);

/* game_scene_register
 *
 * Added de \scene to the list of game scenes.
 */
void game_scene_register(scene_t scene);

/* game_scene_make_current
 *
 * Make the scene with name \scene_name the current scene. This operation could
 * be turn pended until some scene flow ends.
 *
 * Return:
 *  true on success change
 *  false otherwise, and on this case the current scene is left unchanged
 */
bool game_scene_make_current(const char *scene_name);

/* game_scene_get_current
 *
 * Return the current running scene.
 *
 * Return
 *  a valid scene if any
 *  otherwise, a null scene
 */
scene_t game_scene_get_current(void);

/* game_start_run
 *
 * Start the game loop.
 */
void game_start_run(void);

/* game_is_running
 *
 * Return if the game loop is started.
 *
 * Return:
 *  true if started
 *  false otherwise
 */
bool game_is_running(void);

/* game_end_run
 *
 * Terminate the game loop.
 */
void game_end_run(void);

/* game_get_width
 *
 * Return the game screen width.
 */
int game_get_width(void);

/* game_get_height
 *
 * Return the game screen height.
 */
int game_get_height(void);

/* game_get_platform
 *
 * Return the game platform OS.
 *
 * Return:
 *  Android, Windows, MacOS, Linux if running on one of this platforms
 *  Unknown otherwise
 */
const char *game_get_platform(void);

/* game_mouse_get_position
 */
Vector2 game_mouse_get_position(void);

/* game_mouse_is_down
 */
bool game_mouse_is_down(int mouse_button);

/* game_mouse_is_up
 */
bool game_mouse_is_up(int mouse_button);

/* game_mouse_is_pressed
 */
bool game_mouse_is_pressed(int mouse_button);

/* game_mouse_is_released
 */
bool game_mouse_is_released(int mouse_button);

/* game_touch_get_position
 */
Vector2 game_touch_get_position(int touch_number);

/* game_touch_get_id
 */
int game_touch_get_id(int touch_number);

/* game_touch_get_points_count
 */
int game_touch_get_points_count(void);

/* game_touch_is_down
 */
bool game_touch_is_down(int touch_id);

/* game_touch_is_up
 */
bool game_touch_is_up(int touch_id);

/* game_touch_is_pressed
 */
bool game_touch_is_pressed(int touch_id);

/* game_touch_is_released
 */
bool game_touch_is_released(int touch_id);

/* game_texture_load
 *
 * Load a texture.
 *
 * Return:
 *  The loaded texture
 */
Texture game_texture_load(const char *filename, const char *name);

/* game_texture_get
 *
 * Fetch for a loaded texture by name \name.
 *
 * Return:
 *  The texture with given name
 */
Texture game_texture_get(const char *name);

/* game_font_load
 *
 * Load a font.
 *
 * Return:
 *  The loaded font
 */
Font game_font_load(const char *filename, const char *name);

/* game_font_get
 *
 * Fetch for a loaded font by name \name.
 *
 * Return:
 *  The font with given name
 */
Font game_font_get(const char *name);

#endif // !GAME_H

