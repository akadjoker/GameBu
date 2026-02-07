/*
 *  Exemplo Avançado - Sistema de Sprites Animados
 *  
 *  Este exemplo demonstra:
 *  - Animação de sprites
 *  - Sistema de entidades
 *  - Múltiplos FPGs
 *  - Efeitos visuais
 */

#include "raylib.h"
#include "file_div_raylib.h"
#include <math.h>
#include <stdlib.h>

#define MAX_ENTITIES 100

// Entidade do jogo
typedef struct {
    Vector2 position;
    Vector2 velocity;
    float rotation;
    float scale;
    int graphic_code;
    int anim_frame;
    float anim_timer;
    int active;
    Color tint;
} Entity;

// Sistema de partículas simples
typedef struct {
    Vector2 position;
    Vector2 velocity;
    float life;
    float max_life;
    Color color;
    int active;
} Particle;

#define MAX_PARTICLES 500

Entity entities[MAX_ENTITIES];
Particle particles[MAX_PARTICLES];

void InitEntity(Entity* e, Vector2 pos, int graphic) {
    e->position = pos;
    e->velocity = (Vector2){ 0, 0 };
    e->rotation = 0;
    e->scale = 1.0f;
    e->graphic_code = graphic;
    e->anim_frame = 0;
    e->anim_timer = 0;
    e->active = 1;
    e->tint = WHITE;
}

void SpawnParticle(Vector2 pos, Color color) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!particles[i].active) {
            particles[i].position = pos;
            particles[i].velocity = (Vector2){
                (float)(rand() % 200 - 100),
                (float)(rand() % 200 - 100)
            };
            particles[i].max_life = 1.0f + (rand() % 100) / 100.0f;
            particles[i].life = particles[i].max_life;
            particles[i].color = color;
            particles[i].active = 1;
            break;
        }
    }
}

void UpdateParticles(float deltaTime) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].active) {
            particles[i].position.x += particles[i].velocity.x * deltaTime;
            particles[i].position.y += particles[i].velocity.y * deltaTime;
            particles[i].velocity.y += 200 * deltaTime; // Gravidade
            particles[i].life -= deltaTime;
            
            if (particles[i].life <= 0) {
                particles[i].active = 0;
            }
        }
    }
}

void DrawParticles(void) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].active) {
            float alpha = particles[i].life / particles[i].max_life;
            Color c = particles[i].color;
            c.a = (unsigned char)(alpha * 255);
            DrawCircleV(particles[i].position, 3, c);
        }
    }
}

int main(void) {
    const int screenWidth = 1280;
    const int screenHeight = 720;
    
    InitWindow(screenWidth, screenHeight, "DIV Loader - Advanced Example");
    SetTargetFPS(60);
    
    // Inicializar entidades e partículas
    for (int i = 0; i < MAX_ENTITIES; i++) {
        entities[i].active = 0;
    }
    
    for (int i = 0; i < MAX_PARTICLES; i++) {
        particles[i].active = 0;
    }
    
    // Carregar recursos (descomente quando tiver arquivos reais)
    // DIV_FPG* sprites_fpg = DIV_LoadFPG("sprites.fpg");
    // DIV_FPG* effects_fpg = DIV_LoadFPG("effects.fpg");
    // DIV_GRAPHIC* background = DIV_LoadMAP("background.map");
    // DIV_FONT* font = DIV_LoadFont("font.fnt");
    
    DIV_FPG* sprites_fpg = NULL;
    DIV_FPG* effects_fpg = NULL;
    DIV_GRAPHIC* background = NULL;
    DIV_FONT* font = NULL;
    
    // Criar alguns sprites de teste
    Image testSprite1 = GenImageColor(64, 64, BLUE);
    ImageDrawRectangle(&testSprite1, 16, 16, 32, 32, SKYBLUE);
    Texture2D testTexture1 = LoadTextureFromImage(testSprite1);
    UnloadImage(testSprite1);
    
    Image testSprite2 = GenImageColor(48, 48, RED);
    ImageDrawCircle(&testSprite2, 24, 24, 20, ORANGE);
    Texture2D testTexture2 = LoadTextureFromImage(testSprite2);
    UnloadImage(testSprite2);
    
    // Criar entidades de teste
    for (int i = 0; i < 20; i++) {
        Vector2 pos = {
            (float)(rand() % screenWidth),
            (float)(rand() % screenHeight)
        };
        InitEntity(&entities[i], pos, i % 10);
        entities[i].velocity = (Vector2){
            (float)(rand() % 100 - 50),
            (float)(rand() % 100 - 50)
        };
        entities[i].scale = 0.5f + (rand() % 100) / 100.0f;
    }
    
    float time = 0;
    int particle_spawn_timer = 0;
    
    // Loop principal
    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        time += deltaTime;
        
        // Input
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mousePos = GetMousePosition();
            // Criar explosão de partículas
            for (int i = 0; i < 20; i++) {
                SpawnParticle(mousePos, (Color){
                    (unsigned char)(rand() % 255),
                    (unsigned char)(rand() % 255),
                    (unsigned char)(rand() % 255),
                    255
                });
            }
        }
        
        // Atualizar entidades
        for (int i = 0; i < MAX_ENTITIES; i++) {
            if (entities[i].active) {
                // Movimento
                entities[i].position.x += entities[i].velocity.x * deltaTime;
                entities[i].position.y += entities[i].velocity.y * deltaTime;
                
                // Rotação
                entities[i].rotation += 50 * deltaTime;
                
                // Bounce nas bordas
                if (entities[i].position.x < 0 || entities[i].position.x > screenWidth) {
                    entities[i].velocity.x *= -1;
                    entities[i].position.x = Clamp(entities[i].position.x, 0, screenWidth);
                }
                if (entities[i].position.y < 0 || entities[i].position.y > screenHeight) {
                    entities[i].velocity.y *= -1;
                    entities[i].position.y = Clamp(entities[i].position.y, 0, screenHeight);
                }
                
                // Animação
                entities[i].anim_timer += deltaTime;
                if (entities[i].anim_timer > 0.1f) {
                    entities[i].anim_frame = (entities[i].anim_frame + 1) % 8;
                    entities[i].anim_timer = 0;
                }
                
                // Efeito de cor pulsante
                float pulse = (sinf(time * 3 + i) + 1) / 2;
                entities[i].tint = ColorAlpha(WHITE, 0.5f + pulse * 0.5f);
            }
        }
        
        // Atualizar partículas
        UpdateParticles(deltaTime);
        
        // Spawn partículas automático
        particle_spawn_timer++;
        if (particle_spawn_timer > 5) {
            Vector2 pos = {
                (float)(rand() % screenWidth),
                50
            };
            SpawnParticle(pos, GOLD);
            particle_spawn_timer = 0;
        }
        
        // Desenhar
        BeginDrawing();
        ClearBackground((Color){ 20, 20, 30, 255 });
        
        // Desenhar background
        if (background) {
            DIV_DrawGraphic(background, 0, 0, WHITE);
        } else {
            // Background de teste
            for (int y = 0; y < screenHeight; y += 20) {
                DrawRectangle(0, y, screenWidth, 10, 
                    (Color){ 30, 30, 40, 255 });
            }
        }
        
        // Desenhar entidades
        for (int i = 0; i < MAX_ENTITIES; i++) {
            if (entities[i].active) {
                if (sprites_fpg) {
                    DIV_GRAPHIC* sprite = DIV_GetGraphic(sprites_fpg, 
                        entities[i].graphic_code + entities[i].anim_frame);
                    if (sprite) {
                        DIV_DrawGraphicEx(sprite, entities[i].position,
                            entities[i].rotation, entities[i].scale,
                            entities[i].tint);
                    }
                } else {
                    // Usar texturas de teste
                    Texture2D tex = (i % 2 == 0) ? testTexture1 : testTexture2;
                    Rectangle source = { 0, 0, (float)tex.width, (float)tex.height };
                    Rectangle dest = {
                        entities[i].position.x,
                        entities[i].position.y,
                        tex.width * entities[i].scale,
                        tex.height * entities[i].scale
                    };
                    Vector2 origin = { 
                        tex.width * entities[i].scale / 2,
                        tex.height * entities[i].scale / 2
                    };
                    DrawTexturePro(tex, source, dest, origin,
                        entities[i].rotation, entities[i].tint);
                }
            }
        }
        
        // Desenhar partículas
        DrawParticles();
        
        // Desenhar efeitos visuais
        if (effects_fpg) {
            // Exemplo de efeito no mouse
            Vector2 mousePos = GetMousePosition();
            int effect_frame = (int)(time * 10) % 8;
            DIV_GRAPHIC* effect = DIV_GetGraphic(effects_fpg, effect_frame);
            if (effect) {
                DIV_DrawGraphicEx(effect, mousePos, 0, 1.5f, 
                    ColorAlpha(WHITE, 0.7f));
            }
        }
        
        // UI
        if (font) {
            DIV_DrawText(font, "DIV Advanced Demo", 10, 10, WHITE);
        } else {
            DrawText("DIV Advanced Demo", 10, 10, 30, WHITE);
            DrawText("Using test graphics - Load FPG files for full features", 10, 50, 16, GRAY);
        }
        
        // Stats
        DrawText(TextFormat("FPS: %d", GetFPS()), 10, screenHeight - 100, 20, LIME);
        DrawText(TextFormat("Entities: %d", 20), 10, screenHeight - 70, 20, SKYBLUE);
        
        int active_particles = 0;
        for (int i = 0; i < MAX_PARTICLES; i++) {
            if (particles[i].active) active_particles++;
        }
        DrawText(TextFormat("Particles: %d", active_particles), 10, screenHeight - 40, 20, ORANGE);
        
        // Instruções
        DrawText("Click: Spawn particles", screenWidth - 250, screenHeight - 70, 18, YELLOW);
        DrawText("ESC: Exit", screenWidth - 250, screenHeight - 40, 18, GRAY);
        
        // Borda decorativa
        DrawRectangleLinesEx((Rectangle){ 0, 0, screenWidth, screenHeight }, 2, DARKBLUE);
        
        EndDrawing();
    }
    
    // Limpar
    UnloadTexture(testTexture1);
    UnloadTexture(testTexture2);
    
    if (sprites_fpg) DIV_FreeFPG(sprites_fpg);
    if (effects_fpg) DIV_FreeFPG(effects_fpg);
    if (background) DIV_FreeGraphic(background);
    if (font) DIV_FreeFont(font);
    
    CloseWindow();
    return 0;
}
