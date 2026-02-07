# Guia de Início Rápido - DIV Loader para Raylib

## 🚀 Instalação em 5 minutos

### 1. Instalar Raylib

**Ubuntu/Debian:**
```bash
sudo apt update && sudo apt install libraylib-dev
```

**macOS:**
```bash
brew install raylib
```

**Windows:**
Baixe de https://www.raylib.com/ e siga as instruções.

### 2. Compilar

```bash
# Compilar tudo
make

# Ou compilar apenas um exemplo específico
make div_loader_demo
make div_advanced_demo
make div_converter
```

### 3. Executar

```bash
# Demo básico
./div_loader_demo

# Demo avançado com animações
./div_advanced_demo

# Conversor de arquivos
./div_converter sprites.fpg output/
```

## 📦 Estrutura do Projeto

```
.
├── file_div_raylib.h       # Header principal
├── file_div_raylib.c       # Implementação
├── example.c               # Exemplo básico
├── advanced_example.c      # Exemplo avançado
├── div_converter.c         # Utilitário conversor
├── Makefile                # Build system
└── README.md               # Documentação completa
```

## 💻 Exemplo Mínimo

```c
#include "raylib.h"
#include "file_div_raylib.h"

int main(void) {
    InitWindow(800, 600, "My DIV Game");
    
    // Carregar sprites
    DIV_FPG* fpg = DIV_LoadFPG("sprites.fpg");
    
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);
        
        // Desenhar sprite código 1
        DIV_GRAPHIC* sprite = DIV_GetGraphic(fpg, 1);
        DIV_DrawGraphic(sprite, 100, 100, WHITE);
        
        EndDrawing();
    }
    
    DIV_FreeFPG(fpg);
    CloseWindow();
    return 0;
}
```

Compile com:
```bash
gcc -o mygame mygame.c file_div_raylib.c -lraylib -lm
```

## 🎮 Usando Seus Arquivos DIV

### 1. Converter FPG para PNG

```bash
./div_converter sprites.fpg output_sprites/
```

Isso cria:
- `output_sprites/001_hero.png`
- `output_sprites/002_enemy.png`
- `output_sprites/palette.txt`
- ...

### 2. Converter MAP para PNG

```bash
./div_converter background.map background.png
```

### 3. Extrair Fonte

```bash
./div_converter game.fnt output_font/
```

Isso cria:
- `output_font/font_sheet.png` - Todos os caracteres
- `output_font/char_065.png` - Letra 'A'
- `output_font/font_info.txt` - Informações
- ...

## 🔥 Recursos Principais

### Carregar FPG
```c
DIV_FPG* fpg = DIV_LoadFPG("sprites.fpg");
DIV_GRAPHIC* sprite = DIV_GetGraphic(fpg, 1);
DIV_DrawGraphic(sprite, x, y, WHITE);
```

### Carregar MAP
```c
DIV_GRAPHIC* bg = DIV_LoadMAP("background.map");
DIV_DrawGraphic(bg, 0, 0, WHITE);
```

### Carregar Fonte
```c
DIV_FONT* font = DIV_LoadFont("game.fnt");
DIV_DrawText(font, "Hello!", 50, 50, WHITE);
```

### Rotação e Escala
```c
Vector2 pos = {400, 300};
float rotation = 45.0f;
float scale = 2.0f;
DIV_DrawGraphicEx(sprite, pos, rotation, scale, WHITE);
```

### Control Points (Pivô)
```c
if (sprite->ncpoints > 0) {
    // Primeiro control point é usado como pivô
    printf("Pivot: (%d, %d)\n", 
        sprite->cpoints[0].x, 
        sprite->cpoints[0].y);
}
```

## 🐛 Problemas Comuns

### "Cannot open file"
- Verifique o caminho do arquivo
- Use caminho absoluto para teste: `/home/user/sprites.fpg`

### "Invalid magic number"
- Arquivo não é um FPG/MAP/FNT válido
- Verifique se não está corrompido

### Cores erradas
- Arquivo pode ser de versão não suportada
- Tente converter primeiro com o `div_converter`

### Crash ao desenhar
```c
// SEMPRE verifique se carregou:
if (sprite && sprite->texture.id > 0) {
    DIV_DrawGraphic(sprite, x, y, WHITE);
}
```

## 📚 Próximos Passos

1. **Leia o README.md** para documentação completa
2. **Execute os exemplos** para ver tudo funcionando
3. **Experimente converter** seus arquivos DIV antigos
4. **Crie seu jogo** combinando Raylib + DIV assets!

## 🆘 Precisa de Ajuda?

- Documentação completa: `README.md`
- Exemplos: `example.c` e `advanced_example.c`
- Raylib docs: https://www.raylib.com/cheatsheet/
- BennuGD wiki: http://wiki.bennugd.org/

## 🎯 Dicas

### Performance
```c
// Carregar uma vez, usar muitas vezes
DIV_FPG* sprites = DIV_LoadFPG("sprites.fpg");

// Em um loop
while (!WindowShouldClose()) {
    // Isso é rápido - texture já está na GPU
    DIV_DrawGraphic(DIV_GetGraphic(sprites, 1), x, y, WHITE);
}
```

### Organização
```c
// Estrutura sugerida
typedef struct {
    DIV_FPG* sprites;
    DIV_FPG* effects;
    DIV_GRAPHIC* background;
    DIV_FONT* font;
} GameAssets;

void LoadAssets(GameAssets* assets) {
    assets->sprites = DIV_LoadFPG("sprites.fpg");
    assets->effects = DIV_LoadFPG("effects.fpg");
    // ...
}

void FreeAssets(GameAssets* assets) {
    DIV_FreeFPG(assets->sprites);
    DIV_FreeFPG(assets->effects);
    // ...
}
```

### Animação
```c
// Sistema simples de animação
typedef struct {
    int first_frame;
    int num_frames;
    float fps;
    float timer;
    int current;
} Animation;

void UpdateAnim(Animation* anim, float dt) {
    anim->timer += dt;
    if (anim->timer >= 1.0f / anim->fps) {
        anim->current = (anim->current + 1) % anim->num_frames;
        anim->timer = 0;
    }
}

void DrawAnim(DIV_FPG* fpg, Animation* anim, int x, int y) {
    int code = anim->first_frame + anim->current;
    DIV_DrawGraphic(DIV_GetGraphic(fpg, code), x, y, WHITE);
}
```

---

**Pronto para começar? Execute `make` e divirta-se!** 🎮
