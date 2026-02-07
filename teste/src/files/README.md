# DIV/Bennu File Loader para Raylib

Biblioteca para carregar arquivos de gráficos e fontes do formato DIV Games Studio / BennuGD usando Raylib.

## 📋 Sobre

Este projeto adapta o sistema de carregamento de arquivos do BennuGD para funcionar com a biblioteca Raylib, permitindo usar assets clássicos do DIV Games Studio em projetos modernos.

### Formatos Suportados

- **FPG** (f32, f16, fpg, f01) - Bibliotecas de gráficos (coleções de sprites)
- **MAP** (m32, m16, map, m01) - Imagens individuais
- **FNT** (fnt, fnx) - Fontes bitmap

### Profundidades de Cor Suportadas

- 32-bit RGBA
- 16-bit RGB565
- 8-bit indexado (com paleta)
- 1-bit monocromático

## 🚀 Instalação

### Dependências

- **Raylib** 4.0 ou superior
- **GCC** ou **Clang**

#### Instalar Raylib

**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install libraylib-dev
```

**Fedora:**
```bash
sudo dnf install raylib-devel
```

**macOS:**
```bash
brew install raylib
```

**Windows:**
Baixe de [raylib.com](https://www.raylib.com/) e siga as instruções de instalação.

### Compilação

```bash
# Compilar o exemplo
make

# Executar
make run

# Limpar arquivos compilados
make clean

# Compilar em modo debug
make debug
```

## 📖 Uso

### Exemplo Básico - Carregar FPG

```c
#include "raylib.h"
#include "file_div_raylib.h"

int main(void) {
    InitWindow(800, 600, "DIV Loader Example");
    
    // Carregar biblioteca de gráficos
    DIV_FPG* fpg = DIV_LoadFPG("sprites.fpg");
    
    if (fpg) {
        printf("Carregados %d gráficos\n", fpg->num_graphics);
        
        while (!WindowShouldClose()) {
            BeginDrawing();
            ClearBackground(RAYWHITE);
            
            // Desenhar gráfico código 1
            DIV_GRAPHIC* sprite = DIV_GetGraphic(fpg, 1);
            if (sprite) {
                DIV_DrawGraphic(sprite, 100, 100, WHITE);
            }
            
            EndDrawing();
        }
        
        DIV_FreeFPG(fpg);
    }
    
    CloseWindow();
    return 0;
}
```

### Carregar MAP (Imagem Individual)

```c
// Carregar uma imagem
DIV_GRAPHIC* map = DIV_LoadMAP("background.map");

if (map) {
    printf("MAP: %s (%dx%d)\n", map->name, map->width, map->height);
    
    // Desenhar
    DIV_DrawGraphic(map, 0, 0, WHITE);
    
    // Salvar como PNG
    DIV_SaveImage(map, "output.png");
    
    // Liberar
    DIV_FreeGraphic(map);
}
```

### Carregar e Usar Fontes

```c
// Carregar fonte
DIV_FONT* font = DIV_LoadFont("game.fnt");

if (font) {
    // Desenhar texto
    DIV_DrawText(font, "Hello World!", 50, 50, WHITE);
    DIV_DrawText(font, "DIV Font Support!", 50, 100, BLUE);
    
    // Liberar
    DIV_FreeFont(font);
}
```

### Desenho Avançado com Transformações

```c
DIV_GRAPHIC* sprite = DIV_GetGraphic(fpg, 5);

if (sprite) {
    Vector2 position = { 400, 300 };
    float rotation = 45.0f;  // graus
    float scale = 2.0f;      // 200%
    
    // Desenhar com rotação e escala
    DIV_DrawGraphicEx(sprite, position, rotation, scale, WHITE);
}
```

### Trabalhar com Control Points

Os gráficos DIV podem ter pontos de controle que definem pivôs ou hotspots:

```c
DIV_GRAPHIC* graphic = DIV_GetGraphic(fpg, 10);

if (graphic && graphic->ncpoints > 0) {
    printf("Gráfico tem %d control points\n", graphic->ncpoints);
    
    for (int i = 0; i < graphic->ncpoints; i++) {
        if (graphic->cpoints[i].x != CPOINT_UNDEFINED) {
            printf("  Point %d: (%d, %d)\n", 
                   i, graphic->cpoints[i].x, graphic->cpoints[i].y);
        }
    }
}
```

## 🔧 API Reference

### Estruturas

#### `DIV_FPG`
Biblioteca de gráficos (arquivo FPG).

```c
typedef struct {
    int num_graphics;           // Número de gráficos carregados
    int capacity;               // Capacidade (normalmente 1000)
    DIV_GRAPHIC** graphics;     // Array de gráficos
    Color palette[256];         // Paleta para gráficos 8-bit
    int has_palette;            // 1 se tem paleta
} DIV_FPG;
```

#### `DIV_GRAPHIC`
Um único gráfico/sprite.

```c
typedef struct {
    uint32_t code;              // Código identificador
    char name[32];              // Nome do gráfico
    uint16_t width;             // Largura
    uint16_t height;            // Altura
    Image image;                // Imagem Raylib
    Texture2D texture;          // Textura Raylib (GPU)
    int ncpoints;               // Número de control points
    CPOINT* cpoints;            // Array de control points
} DIV_GRAPHIC;
```

#### `DIV_FONT`
Fonte bitmap.

```c
typedef struct {
    int charset;                // Conjunto de caracteres
    int bpp;                    // Bits por pixel
    DIV_CHARDATA chardata[256]; // Dados de cada caractere
    Image glyphs[256];          // Imagens dos glifos
    Texture2D textures[256];    // Texturas dos glifos
    Color palette[256];         // Paleta (se 8-bit)
    int has_palette;            // 1 se tem paleta
} DIV_FONT;
```

### Funções - FPG

```c
// Carregar arquivo FPG
DIV_FPG* DIV_LoadFPG(const char* filename);

// Liberar FPG
void DIV_FreeFPG(DIV_FPG* fpg);

// Obter gráfico por código
DIV_GRAPHIC* DIV_GetGraphic(DIV_FPG* fpg, int code);

// Obter textura por código
Texture2D DIV_GetTexture(DIV_FPG* fpg, int code);
```

### Funções - MAP

```c
// Carregar arquivo MAP
DIV_GRAPHIC* DIV_LoadMAP(const char* filename);

// Liberar gráfico
void DIV_FreeGraphic(DIV_GRAPHIC* graphic);
```

### Funções - Fontes

```c
// Carregar fonte
DIV_FONT* DIV_LoadFont(const char* filename);

// Liberar fonte
void DIV_FreeFont(DIV_FONT* font);

// Desenhar texto
void DIV_DrawText(DIV_FONT* font, const char* text, int x, int y, Color tint);
```

### Funções - Desenho

```c
// Desenhar gráfico simples
void DIV_DrawGraphic(DIV_GRAPHIC* graphic, int x, int y, Color tint);

// Desenhar com transformações
void DIV_DrawGraphicEx(DIV_GRAPHIC* graphic, Vector2 position, 
                       float rotation, float scale, Color tint);

// Salvar gráfico como imagem
int DIV_SaveImage(DIV_GRAPHIC* graphic, const char* filename);
```

## 🎮 Compatibilidade

### Arquivos Testados
- ✅ FPG 32-bit (f32)
- ✅ FPG 16-bit (f16)
- ✅ FPG 8-bit com paleta (fpg)
- ✅ FPG 1-bit (f01)
- ✅ MAP 32-bit (m32)
- ✅ MAP 16-bit (m16)
- ✅ MAP 8-bit (map)
- ✅ Fontes 8-bit (fnt)

### Plataformas
- ✅ Linux
- ✅ Windows
- ✅ macOS
- ✅ Web (com Emscripten)

## 🔄 Conversão de Byte Order

A biblioteca lida automaticamente com diferenças de endianness entre plataformas, permitindo que arquivos criados em sistemas big-endian funcionem em little-endian e vice-versa.

## 💡 Dicas

### Performance

1. **Pré-carregar tudo**: Carregue FPGs no início do programa
2. **Usar texturas**: As texturas já são enviadas para GPU automaticamente
3. **Evitar recarregar**: Reutilize os recursos carregados

### Paletas

Para gráficos 8-bit, a cor 0 da paleta é automaticamente definida como transparente. Se precisar mudar:

```c
if (fpg->has_palette) {
    fpg->palette[0].a = 255;  // Tornar opaco
}
```

### Control Points

O primeiro control point (índice 0) é usado como origem em `DIV_DrawGraphicEx`. Você pode modificar manualmente:

```c
graphic->cpoints[0].x = graphic->width / 2;   // Centro
graphic->cpoints[0].y = graphic->height / 2;
```

## 🐛 Troubleshooting

### Erro ao carregar arquivo
```
DIV_LoadFPG: Cannot open file sprites.fpg
```
- Verifique se o arquivo existe
- Verifique o caminho relativo
- Tente caminho absoluto para teste

### Cores erradas
- Arquivo pode usar formato não suportado
- Verifique se é realmente um arquivo DIV/Bennu válido

### Crash ao desenhar
- Verifique se a textura foi carregada: `if (sprite->texture.id > 0)`
- Certifique-se de não liberar recursos ainda em uso

## 📚 Recursos Adicionais

- [Raylib Documentation](https://www.raylib.com/cheatsheet/cheatsheet.html)
- [BennuGD Wiki](http://wiki.bennugd.org/)
- [DIV Games Studio](https://www.divgames.com/)

## 📄 Licença

Este código é uma adaptação do código original do BennuGD e mantém as mesmas permissões de licença.

Original Copyright:
- (C) SplinterGU (Fenix/BennuGD) (Since 2006)
- (C) 2002-2006 Fenix Team (Fenix)
- (C) 1999-2002 José Luis Cebrián Pagüe (Fenix)

Este software é fornecido "como está", sem garantias de qualquer tipo.

## 🤝 Contribuindo

Contribuições são bem-vindas! Sinta-se à vontade para:

- Reportar bugs
- Sugerir melhorias
- Enviar pull requests
- Adicionar exemplos

## ✨ Changelog

### v1.0.0 (2026-02-06)
- Primeira versão
- Suporte a FPG, MAP e FNT
- Conversão automática de paletas
- Suporte a control points
- Compatibilidade cross-platform

---

Criado com ❤️ para a comunidade DIV/BennuGD
