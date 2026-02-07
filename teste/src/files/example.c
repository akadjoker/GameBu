/*
 *  Exemplo de uso do DIV/Bennu Loader com Raylib
 *  
 *  Este exemplo demonstra como carregar e exibir:
 *  - Arquivos FPG (bibliotecas de gráficos)
 *  - Arquivos MAP (imagens individuais)
 *  - Arquivos FNT (fontes)
 */

#include "raylib.h"
#include "file_div_raylib.h"
#include <stdio.h>

int main(void) {
    // Inicializar janela
    const int screenWidth = 800;
    const int screenHeight = 600;
    
    InitWindow(screenWidth, screenHeight, "DIV/Bennu Loader - Raylib Example");
    SetTargetFPS(60);
    
    // -------------------------------------------------------------------------
    // Exemplo 1: Carregar FPG (Biblioteca de Gráficos)
    // -------------------------------------------------------------------------
    
    // Descomente para carregar um FPG real:
    // DIV_FPG* fpg = DIV_LoadFPG("sprites.fpg");
    DIV_FPG* fpg = NULL;
    
    if (fpg) {
        TraceLog(LOG_INFO, "FPG loaded with %d graphics", fpg->num_graphics);
        
        // Listar todos os gráficos
        for (int i = 0; i < fpg->capacity; i++) {
            DIV_GRAPHIC* g = fpg->graphics[i];
            if (g) {
                TraceLog(LOG_INFO, "  Graphic %d: '%s' (%dx%d)", 
                         g->code, g->name, g->width, g->height);
            }
        }
    }
    
    // -------------------------------------------------------------------------
    // Exemplo 2: Carregar MAP (Imagem Individual)
    // -------------------------------------------------------------------------
    
    // Descomente para carregar um MAP real:
    // DIV_GRAPHIC* map = DIV_LoadMAP("background.map");
    DIV_GRAPHIC* map = NULL;
    
    if (map) {
        TraceLog(LOG_INFO, "MAP loaded: '%s' (%dx%d)", 
                 map->name, map->width, map->height);
        
        // Salvar como PNG
        DIV_SaveImage(map, "output.png");
    }
    
    // -------------------------------------------------------------------------
    // Exemplo 3: Carregar Fonte
    // -------------------------------------------------------------------------
    
    // Descomente para carregar uma fonte real:
    // DIV_FONT* font = DIV_LoadFont("game.fnt");
    DIV_FONT* font = NULL;
    
    if (font) {
        TraceLog(LOG_INFO, "Font loaded with charset %d", font->charset);
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
    Vector2 position = { 400, 300 };
    
    // Loop principal
    while (!WindowShouldClose()) {
        // Atualizar
        rotation += 1.0f;
        scale = 1.0f + 0.5f * sinf(GetTime() * 2.0f);
        
        // Desenhar
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        // -------------------------------------------------------------------------
        // Desenhar gráficos carregados
        // -------------------------------------------------------------------------
        
        if (map) {
            // Desenhar MAP no canto superior esquerdo
            DIV_DrawGraphic(map, 10, 10, WHITE);
        }
        
        if (fpg) {
            // Exemplo: Desenhar gráfico 1 do FPG
            DIV_GRAPHIC* sprite = DIV_GetGraphic(fpg, 1);
            if (sprite) {
                DIV_DrawGraphicEx(sprite, position, rotation, scale, WHITE);
            }
            
            // Desenhar vários sprites em diferentes posições
            for (int i = 0; i < 10; i++) {
                DIV_GRAPHIC* g = DIV_GetGraphic(fpg, i);
                if (g) {
                    Vector2 pos = { 50.0f + i * 70.0f, 450.0f };
                    DIV_DrawGraphic(g, (int)pos.x, (int)pos.y, WHITE);
                }
            }
        }
        
        if (font) {
            // Desenhar texto com a fonte DIV
            DIV_DrawText(font, "Hello from DIV Font!", 50, 50, WHITE);
            DIV_DrawText(font, "Raylib + DIV/Bennu!", 50, 100, BLUE);
        }
        
        // -------------------------------------------------------------------------
        // Interface de demonstração
        // -------------------------------------------------------------------------
        
        // Gráfico de teste rotacionando
        Rectangle source = { 0, 0, 100, 100 };
        Rectangle dest = { position.x, position.y, 100 * scale, 100 * scale };
        Vector2 origin = { 50 * scale, 50 * scale };
        DrawTexturePro(testTexture, source, dest, origin, rotation, WHITE);
        
        // Informações na tela
        DrawText("DIV/Bennu File Loader - Raylib", 10, 10, 20, DARKGRAY);
        DrawText("Este exemplo demonstra o carregamento de arquivos DIV", 10, 40, 14, GRAY);
        
        if (!fpg && !map && !font) {
            DrawText("Nenhum arquivo carregado - usando gráficos de teste", 10, 70, 14, RED);
            DrawText("Descomente as linhas de carregamento no código", 10, 90, 14, RED);
        }
        
        // Status
        DrawText(TextFormat("FPS: %d", GetFPS()), 10, screenHeight - 30, 20, DARKGREEN);
        DrawText(TextFormat("Rotation: %.1f", rotation), 200, screenHeight - 30, 20, DARKBLUE);
        
        // Instruções
        DrawText("ESC - Sair", screenWidth - 150, screenHeight - 30, 20, DARKGRAY);
        
        EndDrawing();
    }
    
    // Limpar recursos
    UnloadTexture(testTexture);
    
    if (fpg) DIV_FreeFPG(fpg);
    if (map) DIV_FreeGraphic(map);
    if (font) DIV_FreeFont(font);
    
    CloseWindow();
    
    return 0;
}
