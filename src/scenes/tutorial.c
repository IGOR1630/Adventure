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
#include<locale.h>
#include "raylib.h"
#include "game.h"
#include "scene.h"
#define NUM_FRAMES  3
static Font alagard;
scene_data_t *tutorial_init(void) {
    alagard = LoadFont("custom_alagard.png");
 setlocale(LC_ALL,"portuguese");
    return NULL;
}

void tutorial_update(void *data){
                WaitTime(5000);
             game_set_scene("genmap");


}

void tutorial_draw(void *data)
{
    Texture key = game_get_texture("key-img");
    Texture wasd = game_get_texture("wasd-img");
    Texture map_with_player = game_get_texture("map_with_player-img");
    Texture mapt = game_get_texture("mapt-img");
    Texture plantone = game_get_texture("plantone-img");
    Texture planttwo = game_get_texture("planttwo-img");
    Texture cogu = game_get_texture("cogu-img");
    Texture mushroom = game_get_texture("mushroom-img");
    Texture gram = game_get_texture("gram-img");
    ClearBackground(GetColor(0x038c7fff));
    DrawRectangleGradientV(0, 0, 1280, 720, GetColor(0x038c7fff), GOLD);
    //AMARELO 0xfeae34fff
///DRAW INFORMATIONS
     DrawTextEx(alagard,"LOADING... ", (Vector2) { 10, 50 }, 20, -1, BLACK);
      DrawTextEx(alagard,"HOW PLAY?", (Vector2) { 400, 50 }, 75, -1, GOLD);
      DrawTextEx(alagard,"1-ADVENTURE is an  ", (Vector2) { 10, 520 }, 30, -1, WHITE);
      DrawTextEx(alagard,"action and adventure ", (Vector2) { 10, 550 }, 30, -1, WHITE);
      DrawTextEx(alagard,"game and your ", (Vector2) { 10, 580 }, 30, -1, WHITE);
      DrawTextEx(alagard,"main goal is to survive", (Vector2) { 10, 610 }, 30, -1, WHITE);
      DrawTextEx(alagard,"2-To survive you need", (Vector2) { 475, 520 }, 30, -1, WHITE);
      DrawTextEx(alagard,"to move around the map", (Vector2) { 475, 550 }, 30, -1, WHITE);
      DrawTextEx(alagard,"and of course escape and", (Vector2) { 475, 580 }, 30, -1, WHITE);
      DrawTextEx(alagard,"and attack the slimes", (Vector2) { 475, 610 }, 30, -1, WHITE);
      DrawTextEx(alagard,"3-To move use", (Vector2) { 920, 520 }, 30, -1, WHITE);
      DrawTextEx(alagard,"as keys W,A,S,D ", (Vector2) { 920, 550 }, 30, -1, WHITE);
      DrawTextEx(alagard,"and to atack use,", (Vector2) { 920, 580 }, 30, -1, WHITE);
      DrawTextEx(alagard,"key E ", (Vector2) { 920, 610 }, 30, -1, WHITE);


///DRAW ILLUSTRATIVE PICTURES
    DrawTexture(key,1200,400,WHITE);
    DrawTexture(wasd,870,250,WHITE);
    DrawTexture(map_with_player,475,250,WHITE);
    DrawTexture(mapt,10,250,WHITE);

/// DRAW PLANTS
    DrawTexture(plantone,350,631,WHITE);
    DrawTexture(planttwo,895,629,WHITE);
    DrawTexture(cogu,1200,635,WHITE);
    DrawTexture(mushroom,10,635,WHITE);

///INFERIOR GRAM
    DrawTextureTiled(gram,
        (Rectangle) { 10, 0, gram.width - 10, gram.height },
        (Rectangle) { 0, game_height() - 40, game_width(), gram.height },
        (Vector2) { 0, 0 }, 0, 1, WHITE);

}
void tutorial_deinit(void *data)
{
    UnloadFont(alagard);
}

