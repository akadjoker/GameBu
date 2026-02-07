/*
 *  Template de Projeto Completo
 *  
 *  Este arquivo serve como template/esqueleto para um projeto
 *  completo usando DIV Loader + Raylib
 *  
 *  Características:
 *  - Sistema de estados (menu, jogo, pause, game over)
 *  - Gerenciamento de recursos
 *  - Sistema de entidades básico
 *  - Input handling
 */

#include "raylib.h"
#include "file_div_raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================================
// CONFIGURAÇÕES DO JOGO
// ============================================================================

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define GAME_TITLE "Meu Jogo DIV"
#define TARGET_FPS 60

// ============================================================================
// ESTADOS DO JOGO
// ============================================================================

typedef enum {
    STATE_MENU,
    STATE_PLAYING,
    STATE_PAUSED,
    STATE_GAME_OVER
} GameState;

// ============================================================================
// RECURSOS
// ============================================================================

typedef struct {
    // Gráficos
    DIV_FPG* sprites;
    DIV_FPG* effects;
    DIV_GRAPHIC* background;
    
    // Fontes
    DIV_FONT* font_normal;
    DIV_FONT* font_big;
    
    // Sons (adicione raylib audio)
    // Music music;
    // Sound sfx_jump;
    
    int loaded;
} GameAssets;

// ============================================================================
// ENTIDADE/JOGADOR
// ============================================================================

typedef struct {
    Vector2 position;
    Vector2 velocity;
    float rotation;
    float scale;
    int sprite_code;
    int animation_frame;
    float animation_timer;
    int health;
    int score;
    int active;
} Player;

// ============================================================================
// JOGO
// ============================================================================

typedef struct {
    GameState state;
    GameAssets assets;
    Player player;
    
    // Estado do jogo
    int score;
    int lives;
    float game_time;
    
    // Menu
    int menu_selected;
    
} Game;

// ============================================================================
// FUNÇÕES DE RECURSOS
// ============================================================================

void LoadGameAssets(GameAssets* assets) {
    assets->loaded = 0;
    
    TraceLog(LOG_INFO, "Carregando recursos...");
    
    // Carregar sprites (descomente quando tiver os arquivos)
    // assets->sprites = DIV_LoadFPG("data/sprites.fpg");
    // if (!assets->sprites) {
    //     TraceLog(LOG_ERROR, "Falha ao carregar sprites.fpg");
    //     return;
    // }
    
    // assets->effects = DIV_LoadFPG("data/effects.fpg");
    // assets->background = DIV_LoadMAP("data/background.map");
    // assets->font_normal = DIV_LoadFont("data/font.fnt");
    // assets->font_big = DIV_LoadFont("data/font_big.fnt");
    
    // Para testes sem arquivos DIV
    assets->sprites = NULL;
    assets->effects = NULL;
    assets->background = NULL;
    assets->font_normal = NULL;
    assets->font_big = NULL;
    
    TraceLog(LOG_INFO, "Recursos carregados");
    assets->loaded = 1;
}

void FreeGameAssets(GameAssets* assets) {
    if (assets->sprites) DIV_FreeFPG(assets->sprites);
    if (assets->effects) DIV_FreeFPG(assets->effects);
    if (assets->background) DIV_FreeGraphic(assets->background);
    if (assets->font_normal) DIV_FreeFont(assets->font_normal);
    if (assets->font_big) DIV_FreeFont(assets->font_big);
    
    assets->loaded = 0;
}

// ============================================================================
// FUNÇÕES DO JOGADOR
// ============================================================================

void InitPlayer(Player* player) {
    player->position = (Vector2){ SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
    player->velocity = (Vector2){ 0, 0 };
    player->rotation = 0;
    player->scale = 1.0f;
    player->sprite_code = 1;
    player->animation_frame = 0;
    player->animation_timer = 0;
    player->health = 100;
    player->score = 0;
    player->active = 1;
}

void UpdatePlayer(Player* player, float deltaTime) {
    if (!player->active) return;
    
    // Input
    float speed = 200.0f;
    player->velocity = (Vector2){ 0, 0 };
    
    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
        player->velocity.x = -speed;
    }
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
        player->velocity.x = speed;
    }
    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) {
        player->velocity.y = -speed;
    }
    if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) {
        player->velocity.y = speed;
    }
    
    // Movimento
    player->position.x += player->velocity.x * deltaTime;
    player->position.y += player->velocity.y * deltaTime;
    
    // Limites da tela
    player->position.x = Clamp(player->position.x, 0, SCREEN_WIDTH);
    player->position.y = Clamp(player->position.y, 0, SCREEN_HEIGHT);
    
    // Animação
    if (player->velocity.x != 0 || player->velocity.y != 0) {
        player->animation_timer += deltaTime;
        if (player->animation_timer > 0.1f) {
            player->animation_frame = (player->animation_frame + 1) % 4;
            player->animation_timer = 0;
        }
    }
}

void DrawPlayer(Player* player, GameAssets* assets) {
    if (!player->active) return;
    
    if (assets->sprites) {
        int sprite_code = player->sprite_code + player->animation_frame;
        DIV_GRAPHIC* sprite = DIV_GetGraphic(assets->sprites, sprite_code);
        if (sprite) {
            DIV_DrawGraphicEx(sprite, player->position, 
                player->rotation, player->scale, WHITE);
        }
    } else {
        // Fallback - desenhar retângulo simples
        DrawRectangle(
            (int)player->position.x - 16, 
            (int)player->position.y - 16,
            32, 32, BLUE
        );
    }
}

// ============================================================================
// FUNÇÕES DE ESTADO - MENU
// ============================================================================

void UpdateMenu(Game* game, float deltaTime) {
    (void)deltaTime;  // Não usado
    
    if (IsKeyPressed(KEY_UP)) {
        game->menu_selected = (game->menu_selected - 1 + 3) % 3;
    }
    if (IsKeyPressed(KEY_DOWN)) {
        game->menu_selected = (game->menu_selected + 1) % 3;
    }
    
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        switch (game->menu_selected) {
            case 0:  // Jogar
                game->state = STATE_PLAYING;
                InitPlayer(&game->player);
                game->score = 0;
                game->lives = 3;
                game->game_time = 0;
                break;
            case 1:  // Opções
                // TODO: Implementar tela de opções
                break;
            case 2:  // Sair
                // Sinalizar para fechar
                break;
        }
    }
}

void DrawMenu(Game* game) {
    DrawText(GAME_TITLE, 
        SCREEN_WIDTH / 2 - MeasureText(GAME_TITLE, 40) / 2, 
        100, 40, WHITE);
    
    const char* options[] = { "JOGAR", "OPÇÕES", "SAIR" };
    
    for (int i = 0; i < 3; i++) {
        Color color = (i == game->menu_selected) ? YELLOW : GRAY;
        int y = 250 + i * 50;
        DrawText(options[i], 
            SCREEN_WIDTH / 2 - MeasureText(options[i], 30) / 2,
            y, 30, color);
        
        if (i == game->menu_selected) {
            DrawText(">", 
                SCREEN_WIDTH / 2 - MeasureText(options[i], 30) / 2 - 40,
                y, 30, YELLOW);
        }
    }
    
    DrawText("Use SETAS para navegar, ENTER para selecionar",
        SCREEN_WIDTH / 2 - MeasureText("Use SETAS para navegar", 16) / 2,
        SCREEN_HEIGHT - 50, 16, DARKGRAY);
}

// ============================================================================
// FUNÇÕES DE ESTADO - JOGANDO
// ============================================================================

void UpdatePlaying(Game* game, float deltaTime) {
    // Pause
    if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ESCAPE)) {
        game->state = STATE_PAUSED;
        return;
    }
    
    game->game_time += deltaTime;
    
    // Atualizar jogador
    UpdatePlayer(&game->player, deltaTime);
    
    // TODO: Atualizar inimigos, colisões, etc.
    
    // Exemplo de game over
    if (game->player.health <= 0) {
        game->state = STATE_GAME_OVER;
    }
}

void DrawPlaying(Game* game) {
    // Background
    if (game->assets.background) {
        DIV_DrawGraphic(game->assets.background, 0, 0, WHITE);
    } else {
        ClearBackground((Color){ 20, 20, 30, 255 });
    }
    
    // Jogador
    DrawPlayer(&game->player, &game->assets);
    
    // HUD
    DrawText(TextFormat("SCORE: %d", game->score), 10, 10, 20, WHITE);
    DrawText(TextFormat("VIDAS: %d", game->lives), 10, 35, 20, WHITE);
    DrawText(TextFormat("TEMPO: %.1f", game->game_time), 10, 60, 20, WHITE);
    DrawText(TextFormat("HP: %d", game->player.health), 10, 85, 20, 
        game->player.health > 30 ? GREEN : RED);
    
    // Instruções
    DrawText("WASD/SETAS: Mover | P/ESC: Pause", 
        10, SCREEN_HEIGHT - 25, 16, DARKGRAY);
}

// ============================================================================
// FUNÇÕES DE ESTADO - PAUSE
// ============================================================================

void UpdatePaused(Game* game, float deltaTime) {
    (void)deltaTime;
    
    if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ESCAPE)) {
        game->state = STATE_PLAYING;
    }
    if (IsKeyPressed(KEY_M)) {
        game->state = STATE_MENU;
    }
}

void DrawPaused(Game* game) {
    // Desenhar jogo em background (congelado)
    DrawPlaying(game);
    
    // Overlay escuro
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 
        (Color){ 0, 0, 0, 150 });
    
    // Texto de pause
    const char* text = "PAUSADO";
    DrawText(text, 
        SCREEN_WIDTH / 2 - MeasureText(text, 60) / 2,
        SCREEN_HEIGHT / 2 - 60, 60, YELLOW);
    
    DrawText("P/ESC: Continuar | M: Menu",
        SCREEN_WIDTH / 2 - MeasureText("P/ESC: Continuar", 20) / 2,
        SCREEN_HEIGHT / 2 + 20, 20, WHITE);
}

// ============================================================================
// FUNÇÕES DE ESTADO - GAME OVER
// ============================================================================

void UpdateGameOver(Game* game, float deltaTime) {
    (void)deltaTime;
    
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        game->state = STATE_MENU;
    }
}

void DrawGameOver(Game* game) {
    ClearBackground((Color){ 20, 0, 0, 255 });
    
    const char* text = "GAME OVER";
    DrawText(text,
        SCREEN_WIDTH / 2 - MeasureText(text, 60) / 2,
        SCREEN_HEIGHT / 2 - 80, 60, RED);
    
    DrawText(TextFormat("SCORE FINAL: %d", game->score),
        SCREEN_WIDTH / 2 - MeasureText("SCORE FINAL: 0000", 30) / 2,
        SCREEN_HEIGHT / 2, 30, WHITE);
    
    DrawText("ENTER: Menu Principal",
        SCREEN_WIDTH / 2 - MeasureText("ENTER: Menu Principal", 20) / 2,
        SCREEN_HEIGHT / 2 + 60, 20, GRAY);
}

// ============================================================================
// MAIN GAME LOOP
// ============================================================================

int main(void) {
    // Inicializar janela
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, GAME_TITLE);
    SetTargetFPS(TARGET_FPS);
    
    // Inicializar jogo
    Game game = {0};
    game.state = STATE_MENU;
    game.menu_selected = 0;
    
    // Carregar recursos
    LoadGameAssets(&game.assets);
    
    // Loop principal
    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        
        // Atualizar
        switch (game.state) {
            case STATE_MENU:
                UpdateMenu(&game, deltaTime);
                break;
            case STATE_PLAYING:
                UpdatePlaying(&game, deltaTime);
                break;
            case STATE_PAUSED:
                UpdatePaused(&game, deltaTime);
                break;
            case STATE_GAME_OVER:
                UpdateGameOver(&game, deltaTime);
                break;
        }
        
        // Desenhar
        BeginDrawing();
        
        switch (game.state) {
            case STATE_MENU:
                ClearBackground(BLACK);
                DrawMenu(&game);
                break;
            case STATE_PLAYING:
                DrawPlaying(&game);
                break;
            case STATE_PAUSED:
                DrawPaused(&game);
                break;
            case STATE_GAME_OVER:
                DrawGameOver(&game);
                break;
        }
        
        // FPS
        DrawFPS(SCREEN_WIDTH - 100, 10);
        
        EndDrawing();
        
        // Sair do menu
        if (game.state == STATE_MENU && game.menu_selected == 2) {
            if (IsKeyPressed(KEY_ENTER)) break;
        }
    }
    
    // Limpar
    FreeGameAssets(&game.assets);
    CloseWindow();
    
    return 0;
}
