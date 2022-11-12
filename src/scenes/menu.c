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

#include <stdlib.h>
#include "raylib.h"
#include "game.h"
#include "scene.h"

int RIGHT, LEFT, MID;

Font alagard;
Rectangle quadrado_girando = (Rectangle){580,368,15,15};

scene_data_t *menu_init(void) {
    alagard = LoadFont("custom_alagard.png");

    RIGHT = rand() % game_width();
    LEFT = rand() % game_width();
    MID = rand() % game_width();

    return NULL;
}

void menu_update(void *data)
{
    Rectangle start_rect = {
        610, 345, MeasureTextEx(alagard, "Start", 50, -1).x, 50
    };

    Rectangle exit_rect = {
        614, 405, MeasureTextEx(alagard, "Exit", 50, -1).x, 50
    };

    (void) data;

    ///L
    if (IsKeyDown(KEY_DOWN)) quadrado_girando.y = 430;
    if (IsKeyDown(KEY_UP)) quadrado_girando.y = 366;
    if (IsKeyDown(KEY_ENTER)) game_end_run();

    if (CheckCollisionPointRec(game_virtual_mouse(), start_rect)) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            game_set_scene("genmap");

        quadrado_girando.y = 366;
    }

    if (CheckCollisionPointRec(game_virtual_mouse(), exit_rect)) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            game_end_run();

        quadrado_girando.y = 430;
    }


    RIGHT = (RIGHT + 1) % 1280;

    LEFT = (LEFT - 1) % 1280;

    MID = (MID + 1) % 1280;
    ////////////////////////////////////////////////////////
}

void menu_draw(void *data)
{
    (void) data;

    Texture joystick = game_get_texture("joystick-img");
    Texture cloud = game_get_texture("cloud-img");
    Texture gram = game_get_texture("gram-img");
    Texture TreeTwo = game_get_texture("TreeTwo-img");
    Texture Tree = game_get_texture("Tree-img");
    Texture plantone = game_get_texture("plantone-img");
    Texture planttwo = game_get_texture("planttwo-img");
    Texture cogu = game_get_texture("cogu-img");
    Texture mushroom = game_get_texture("mushroom-img");

    ClearBackground(GetColor(0x038c7fff));
    DrawRectangleGradientV(0, 0, 1280, 720, GetColor(0x038c7fff), GOLD);
    //AMARELO 0xfeae34fff

    ///CLOUD POSITION
    DrawTexture(cloud,RIGHT, 5, WHITE);
    if (RIGHT + cloud.width > 1280)
        DrawTexture(cloud, (RIGHT + cloud.width) - 1280 - cloud.width, 5, WHITE);

    DrawTexture(cloud,LEFT, 15, WHITE);
    if (LEFT - cloud.width < 1280)
        DrawTexture(cloud, (LEFT - cloud.width) + 1280 + cloud.width, 15, WHITE);

    DrawTexture(cloud,MID, 130, WHITE);
    if (MID + cloud.width > 1280)
        DrawTexture(cloud, (MID + cloud.width) - 1280 - cloud.width, 130, WHITE);

    /////////ARVORE E PLANTAS INFEIROR/////////////
    DrawTexture(Tree,20, 173, WHITE);
    DrawTexture(TreeTwo,800, 243, WHITE);
    DrawTexture(plantone,350,631,WHITE);
    DrawTexture(planttwo,895,629,WHITE);
    DrawTexture(cogu,1200,635,WHITE);
    DrawTexture(mushroom,10,635,WHITE);


    ///RECTANGLE SUPERIOR PART 1
    DrawRectangle(258,197,930,49,DARKGREEN);
    DrawRectangle(258,247,920,45,WHITE);
    DrawRectangleLines(258,197,930,49,DARKGREEN);
    DrawRectangleLines(258,247,920,45,WHITE);

    /// GAME NAME
    DrawTextEx(alagard,"Adventure", (Vector2) { 460, 210 }, 75, -1, GREEN);
    ///RECTANGLE SUPERIOR PART 2
    DrawTextEx(alagard, "Start", (Vector2) { 610, 345 }, 50, -1, GREEN);
    DrawTextEx(alagard, "Exit", (Vector2) { 614, 405 }, 50, -1, GREEN);

    ///RECTANGLE ROTATION
    static float rotation = 0.0f;
    rotation += 5.5f;
    DrawRectanglePro(quadrado_girando,(Vector2){7.7,7.7}, rotation, WHITE);

    DrawTexture(joystick, -50, 0, WHITE);

    ///INFERIOR GRAM
    DrawTextureTiled(gram,
        (Rectangle) { 10, 0, gram.width - 10, gram.height },
        (Rectangle) { 0, game_height() - 40, game_width(), gram.height },
        (Vector2) { 0, 0 }, 0, 1, WHITE);
}

void menu_deinit(void *data)
{
    (void) data;

    // This line breaks the game and I don't now why so memory leak here :)
    // When called a second time after try to go to another scene this closes
    // the game.
    //
    // UnloadFont(data->alagard);
}

