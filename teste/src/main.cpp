
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include "interpreter.hpp"
 
#include "platform.hpp"

struct FileLoaderContext
{
    const char *searchPaths[8];
    int pathCount;
    char fullPath[512];
    char buffer[1024 * 1024];
};

const char *multiPathFileLoader(const char *filename, size_t *outSize, void *userdata)
{
    FileLoaderContext *ctx = (FileLoaderContext *)userdata;

    for (int i = 0; i < ctx->pathCount; i++)
    {
        snprintf(ctx->fullPath, sizeof(ctx->fullPath), "%s/%s", ctx->searchPaths[i], filename);

        FILE *f = fopen(ctx->fullPath, "rb");
        if (!f)
            continue;

        fseek(f, 0, SEEK_END);
        long size = ftell(f);

        if (size <= 0)
        {
            fclose(f);
            continue;
        }

        if (size >= (long)sizeof(ctx->buffer))
        {
            fprintf(stderr, "File too large: %s (%ld bytes)\n", ctx->fullPath, size);
            fclose(f);
            *outSize = 0;
            return nullptr;
        }

        fseek(f, 0, SEEK_SET);
        size_t bytesRead = fread(ctx->buffer, 1, size, f);
        fclose(f);

        if (bytesRead != (size_t)size)
            continue;

        ctx->buffer[bytesRead] = '\0';
        *outSize = bytesRead;
        return ctx->buffer;
    }

    *outSize = 0;
    return nullptr;
}

// Helper: load file contents into a string
static std::string loadFile(const char *path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        fprintf(stderr, "Could not open file: %s\n", path);
        return "";
    }
    return std::string((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
}

// int main(int argc, char **argv)
// {
//     // Interpreter vm;

//     // vm.registerAll();

//     // FileLoaderContext ctx;
//     // ctx.searchPaths[0] = "./bin";
//     // ctx.searchPaths[1] = "./scripts";
//     // ctx.searchPaths[2] = "./bin/scripts";
//     // ctx.searchPaths[3] = ".";
//     // ctx.pathCount = 4;
//     // vm.setFileLoader(multiPathFileLoader, &ctx);

//     // const char* scriptFile = nullptr;

//     // if (argc>1)
//     // {
//     //     if (OsFileExists(argv[1]))
//     //     {
//     //         scriptFile = argv[1];
//     //     }
//     //     else
//     //     {
//     //         fprintf(stderr, "Specified script file does not exist: %s\n", argv[1]);
//     //         return 1;
//     //     }
//     // }

//     // if (!scriptFile)
//     // {
//     //     if (OsFileExists("scripts/main.bu"))
//     //     {
//     //         scriptFile = "scripts/main.bu";
//     //     }
//     //     else if (OsFileExists("main.bu"))
//     //     {
//     //         scriptFile = "main.bu";
//     //     }
//     //     else
//     //     {
//     //         fprintf(stderr, "No script file specified and no default found.\n");
//     //         return 1;
//     //     }
//     // }

//     // if (!engine.load(scriptFile))
//     // {
//     //     fprintf(stderr, "Failed to load main script: %s\n", scriptFile);
//     //     return 1;

//     // }

//     // engine.startAll();
//     // float dt = 1.0f / 60.0f;
//     // for (int frame = 0; frame < 10; frame++)
//     // {
//     //     engine.updateAll(dt);
//     //     engine.renderAll();
//     // }

//     // vm.unloadAllPlugins();
//     return 0;
// }

/*
 *  Exemplo de uso do DIV/Bennu Loader com Raylib
 *
 *  Este exemplo demonstra como carregar e exibir:
 *  - Arquivos FPG (bibliotecas de gráficos)
 *  - Arquivos MAP (imagens individuais)
 *  - Arquivos FNT (fontes)
 */

#include "div.h"
#include <raylib.h>
#include <stdio.h>

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "DIV/Bennu Loader - Raylib Example");
    SetTargetFPS(60);

    // -------------------------------------------------------------------------
    // Exemplo 1: Carregar FPG (Biblioteca de Gráficos)
    // -------------------------------------------------------------------------

    // Descomente para carregar um FPG real:
    DIV_FPG* fpg = DIV_LoadFPG("assets/div/tutor3.fpg");
    

    if (fpg)
    {
        TraceLog(LOG_INFO, "FPG loaded with %d graphics", fpg->num_graphics);

        // Listar todos os gráficos
        for (int i = 0; i < fpg->capacity; i++)
        {
            DIV_GRAPHIC *g = fpg->graphics[i];
            if (g)
            {
                TraceLog(LOG_INFO, "  Graphic %d: '%s' (%dx%d)",g->code, g->name, g->width, g->height);
            }
        }
    }

    // -------------------------------------------------------------------------
    // Exemplo 2: Carregar MAP (Imagem Individual)
    // -------------------------------------------------------------------------

    // Descomente para carregar um MAP real:
    DIV_GRAPHIC* map = DIV_LoadMAP("assets/div/mapa1.map");
 

    if (map)
    {
        TraceLog(LOG_INFO, "MAP loaded: '%s' (%dx%d)",
                 map->name, map->width, map->height);

        // Salvar como PNG
        DIV_SaveImage(map, "output.png");
    }

  
    // -------------------------------------------------------------------------
    // Criar gráficos de exemplo para demonstração
    // -------------------------------------------------------------------------

    Image testImage = GenImageColor(100, 100, BLUE);
    Texture2D testTexture = LoadTextureFromImage(testImage);
    UnloadImage(testImage);

    // Variáveis para animação
    float rotation = 0.0f;
    float scale = 1.0f;
    Vector2 position = {400, 300};

    // Loop principal
    while (!WindowShouldClose())
    {
        // Atualizar
        rotation += 1.0f;
        scale = 1.0f + 0.5f * sinf(GetTime() * 2.0f);

        // Desenhar
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // -------------------------------------------------------------------------
        // Desenhar gráficos carregados
        // -------------------------------------------------------------------------

        if (map)
        {
            // Desenhar MAP no canto superior esquerdo
            DIV_DrawGraphic(map, 10, 10, WHITE);
        }

        if (fpg)
        {
            // Exemplo: Desenhar gráfico 1 do FPG
            DIV_GRAPHIC *sprite = DIV_GetGraphic(fpg, 1);
            if (sprite)
            {
                DIV_DrawGraphicEx(sprite, position, rotation, scale, WHITE);
            }

            // Desenhar vários sprites em diferentes posições
            for (int i = 0; i < 10; i++)
            {
                DIV_GRAPHIC *g = DIV_GetGraphic(fpg, i);
                if (g)
                {
                    Vector2 pos = {50.0f + i * 70.0f, 450.0f};
                    DIV_DrawGraphic(g, (int)pos.x, (int)pos.y, WHITE);
                }
            }
        }

     
        // -------------------------------------------------------------------------
        // Interface de demonstração
        // -------------------------------------------------------------------------

        // Gráfico de teste rotacionando
        Rectangle source = {0, 0, 100, 100};
        Rectangle dest = {position.x, position.y, 100 * scale, 100 * scale};
        Vector2 origin = {50 * scale, 50 * scale};
        DrawTexturePro(testTexture, source, dest, origin, rotation, WHITE);

        // Informações na tela
        DrawText("DIV/Bennu File Loader - Raylib", 10, 10, 20, DARKGRAY);
        DrawText("Este exemplo demonstra o carregamento de arquivos DIV", 10, 40, 14, GRAY);

     

        // Status
        DrawText(TextFormat("FPS: %d", GetFPS()), 10, screenHeight - 30, 20, DARKGREEN);
        DrawText(TextFormat("Rotation: %.1f", rotation), 200, screenHeight - 30, 20, DARKBLUE);

        // Instruções
        DrawText("ESC - Sair", screenWidth - 150, screenHeight - 30, 20, DARKGRAY);

        EndDrawing();
    }

    // Limpar recursos
    UnloadTexture(testTexture);

    if (fpg)
        DIV_FreeFPG(fpg);
    if (map)
        DIV_FreeGraphic(map);
 

    CloseWindow();

    return 0;
}
