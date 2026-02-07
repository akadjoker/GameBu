# 🎮 DIV/Bennu Loader para Raylib - Pacote Completo

## 📦 Conteúdo do Pacote

Este pacote contém tudo o que você precisa para carregar arquivos DIV/BennuGD em projetos Raylib:

### Arquivos Principais
- **file_div_raylib.h** - Biblioteca header
- **file_div_raylib.c** - Implementação da biblioteca
- **Makefile** - Sistema de build

### Exemplos
- **example.c** - Exemplo básico de uso
- **advanced_example.c** - Exemplo avançado com animações e partículas
- **div_converter.c** - Utilitário para converter arquivos DIV para PNG

### Documentação
- **README.md** - Documentação completa
- **QUICKSTART.md** - Guia de início rápido
- **test_build.sh** - Script de teste de compilação

## 🚀 Instalação Rápida

### 1. Instale o Raylib

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
Baixe de https://www.raylib.com/

### 2. Teste a Instalação

```bash
chmod +x test_build.sh
./test_build.sh
```

Este script verifica:
- ✅ Se o GCC está instalado
- ✅ Se o Raylib está instalado  
- ✅ Se todos os arquivos estão presentes
- ✅ Se o projeto compila

### 3. Compile os Exemplos

```bash
make all
```

Isso criará:
- `div_loader_demo` - Demo básico
- `div_advanced_demo` - Demo avançado com efeitos
- `div_converter` - Conversor de arquivos

### 4. Execute

```bash
# Demo básico
./div_loader_demo

# Demo avançado
./div_advanced_demo

# Conversor (precisa de arquivos DIV)
./div_converter sprites.fpg output/
```

## 📖 Como Usar na Sua Aplicação

### Opção 1: Incluir Diretamente no Projeto

```bash
# Copie os arquivos para seu projeto
cp file_div_raylib.h seu_projeto/
cp file_div_raylib.c seu_projeto/

# No seu código:
#include "file_div_raylib.h"

# Compile:
gcc seu_jogo.c file_div_raylib.c -o seu_jogo -lraylib -lm
```

### Opção 2: Usar como Biblioteca

```bash
# Compile a biblioteca
gcc -c file_div_raylib.c -o file_div_raylib.o

# Use no seu projeto
gcc seu_jogo.c file_div_raylib.o -o seu_jogo -lraylib -lm
```

## 🎯 Exemplo Mínimo

Crie um arquivo `meu_jogo.c`:

```c
#include "raylib.h"
#include "file_div_raylib.h"

int main(void) {
    InitWindow(800, 600, "Meu Jogo DIV");
    
    // Carregar sprites
    DIV_FPG* sprites = DIV_LoadFPG("sprites.fpg");
    
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);
        
        // Desenhar sprite
        DIV_GRAPHIC* player = DIV_GetGraphic(sprites, 1);
        if (player) {
            DIV_DrawGraphic(player, 100, 100, WHITE);
        }
        
        EndDrawing();
    }
    
    DIV_FreeFPG(sprites);
    CloseWindow();
    return 0;
}
```

Compile:
```bash
gcc meu_jogo.c file_div_raylib.c -o meu_jogo -lraylib -lm
./meu_jogo
```

## 🔧 Formatos Suportados

### Bibliotecas de Gráficos (FPG)
- `.fpg` - 8-bit com paleta
- `.f32` - 32-bit RGBA
- `.f16` - 16-bit RGB565
- `.f01` - 1-bit monocromático

### Imagens Individuais (MAP)
- `.map` - 8-bit com paleta
- `.m32` - 32-bit RGBA
- `.m16` - 16-bit RGB565
- `.m01` - 1-bit monocromático

### Fontes
- `.fnt` - Fonte bitmap 8-bit
- `.fnx` - Fonte bitmap estendida

## 📚 Funções Principais

### Carregar Arquivos
```c
DIV_FPG* fpg = DIV_LoadFPG("sprites.fpg");
DIV_GRAPHIC* map = DIV_LoadMAP("background.map");
DIV_FONT* font = DIV_LoadFont("game.fnt");
```

### Desenhar
```c
// Simples
DIV_DrawGraphic(graphic, x, y, WHITE);

// Com transformações
Vector2 pos = {400, 300};
DIV_DrawGraphicEx(graphic, pos, rotation, scale, WHITE);

// Texto
DIV_DrawText(font, "Hello!", x, y, WHITE);
```

### Liberar Memória
```c
DIV_FreeFPG(fpg);
DIV_FreeGraphic(map);
DIV_FreeFont(font);
```

## 🛠️ Utilitário Conversor

Converta seus arquivos DIV para formatos modernos:

```bash
# FPG → PNGs
./div_converter sprites.fpg output_dir/

# MAP → PNG
./div_converter background.map output.png

# Font → PNGs + info
./div_converter game.fnt font_output/
```

## ⚡ Comandos Make Úteis

```bash
make                    # Compilar tudo
make clean              # Limpar arquivos compilados
make rebuild            # Limpar e recompilar
make run                # Executar demo básico
make run-advanced       # Executar demo avançado
make div_loader_demo    # Compilar apenas demo básico
make div_converter      # Compilar apenas conversor
```

## 🐛 Resolução de Problemas

### "raylib.h: No such file or directory"
Raylib não está instalado. Veja seção de instalação acima.

### "undefined reference to..."
Faltando `-lraylib -lm` na compilação.

### "Cannot open file sprites.fpg"
Verifique se o arquivo existe e o caminho está correto.

### Cores estranhas
Arquivo pode estar corrompido ou em formato não suportado.

## 📖 Mais Informações

- **README.md** - Documentação completa da API
- **QUICKSTART.md** - Tutorial passo a passo
- **example.c** - Código de exemplo comentado
- **advanced_example.c** - Exemplos avançados

## 🌟 Recursos

- Carregamento de FPG, MAP e FNT
- Suporte a 1-bit, 8-bit, 16-bit e 32-bit
- Conversão automática de paletas
- Control points (pivôs)
- Cross-platform (Linux, Windows, macOS)
- Utilitário de conversão incluso

## 💡 Dicas

1. **Sempre verifique se os recursos carregaram:**
   ```c
   if (fpg && fpg->graphics[1]) {
       DIV_DrawGraphic(fpg->graphics[1], x, y, WHITE);
   }
   ```

2. **Libere recursos quando não precisar mais:**
   ```c
   DIV_FreeFPG(fpg);  // Libera TODAS as texturas
   ```

3. **Use o conversor para debug:**
   ```bash
   ./div_converter sprites.fpg debug/
   # Verifique as PNGs geradas
   ```

## 📞 Suporte

Para problemas ou dúvidas:
1. Verifique README.md
2. Execute test_build.sh
3. Veja os exemplos

## 📄 Licença

Adaptado do código original BennuGD, mantendo as mesmas permissões de licença.

---

**Desenvolvido com ❤️ para a comunidade DIV/BennuGD**

Bom desenvolvimento! 🎮
