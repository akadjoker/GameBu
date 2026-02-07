
 
#include "raylib.h"
#include "engine.hpp"
#include <cmath>
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 800;
const int GRID_SIZE = 40;
extern GraphLib gGraphLib;


 int main()
{
    InitWindow(800, 600, "Particles");
    SetTargetFPS(60);

    InitScene(); 

    gGraphLib.loadDIV("assets/div/tutor1.fpg");
    

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();

        
        BeginDrawing();

        gGraphLib.DrawGraph(62, 100, 100, WHITE);
        
        EndDrawing();
    }

     
    DestroyScene();

    CloseWindow();
}