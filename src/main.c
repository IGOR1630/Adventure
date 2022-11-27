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

#include <stdlib.h>
#include "core/game.h"
#include "core/scene.h"

int main (int argc, char *argv[])
{
    (void) argc; (void) argv;

    if (!game_init(1280, 720))
        return EXIT_FAILURE;

    { // Scenes
        SCENE_IMPORT(logo);
        game_scene_register(SCENE(logo));
    }

    { // Resources
        game_texture_load("logo.png", "logo-logo");
    }

    game_scene_make_current("logo");
    game_start_run();

    game_deinit();
    return EXIT_SUCCESS;
}

