/*
 *  DIV File Converter - Converte arquivos DIV para formatos modernos
 *  
 *  Uso:
 *    ./div_converter input.fpg output_dir/
 *    ./div_converter input.map output.png
 *    ./div_converter input.fnt output_dir/
 */

#include "raylib.h"
#include "file_div_raylib.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32
    #include <direct.h>
    #define mkdir(path, mode) _mkdir(path)
#endif

void PrintUsage(const char* program) {
    printf("DIV File Converter v1.0\n");
    printf("Usage:\n");
    printf("  %s <input> <output>\n\n", program);
    printf("Examples:\n");
    printf("  %s sprites.fpg output/     - Extract FPG to PNG files\n", program);
    printf("  %s image.map output.png    - Convert MAP to PNG\n", program);
    printf("  %s font.fnt output/        - Extract font glyphs\n\n", program);
    printf("Supported formats:\n");
    printf("  Input:  .fpg, .f32, .f16, .f01, .map, .m32, .m16, .m01, .fnt, .fnx\n");
    printf("  Output: .png, .bmp, .tga, .jpg\n");
}

int IsDirectory(const char* path) {
    struct stat statbuf;
    if (stat(path, &statbuf) != 0) {
        return 0;
    }
    return S_ISDIR(statbuf.st_mode);
}

void EnsureDirectory(const char* path) {
    struct stat st = {0};
    if (stat(path, &st) == -1) {
        mkdir(path, 0755);
    }
}

const char* GetFileExtension(const char* filename) {
    const char* dot = strrchr(filename, '.');
    if (!dot || dot == filename) return "";
    return dot + 1;
}

int ConvertFPG(const char* input, const char* output_dir) {
    printf("Loading FPG: %s\n", input);
    
    // Inicializar Raylib sem janela (headless)
    SetTraceLogLevel(LOG_WARNING);
    InitWindow(1, 1, "");
    
    DIV_FPG* fpg = DIV_LoadFPG(input);
    if (!fpg) {
        CloseWindow();
        printf("Error: Could not load FPG file\n");
        return 1;
    }
    
    printf("Loaded %d graphics\n", fpg->num_graphics);
    
    // Criar diretório de saída
    EnsureDirectory(output_dir);
    
    // Exportar cada gráfico
    int exported = 0;
    for (int i = 0; i < fpg->capacity; i++) {
        if (fpg->graphics[i]) {
            DIV_GRAPHIC* g = fpg->graphics[i];
            char output_file[512];
            
            // Usar nome se disponível, senão usar código
            if (g->name[0] != '\0') {
                snprintf(output_file, sizeof(output_file), 
                    "%s/%03d_%s.png", output_dir, g->code, g->name);
            } else {
                snprintf(output_file, sizeof(output_file), 
                    "%s/%03d.png", output_dir, g->code);
            }
            
            if (ExportImage(g->image, output_file)) {
                printf("  Exported: %s (%dx%d)\n", 
                    output_file, g->width, g->height);
                exported++;
                
                // Exportar informações de control points se existirem
                if (g->ncpoints > 0) {
                    char info_file[512];
                    snprintf(info_file, sizeof(info_file), "%s.txt", output_file);
                    FILE* f = fopen(info_file, "w");
                    if (f) {
                        fprintf(f, "Graphic: %s\n", g->name);
                        fprintf(f, "Code: %d\n", g->code);
                        fprintf(f, "Size: %dx%d\n", g->width, g->height);
                        fprintf(f, "Control Points: %d\n", g->ncpoints);
                        for (int cp = 0; cp < g->ncpoints; cp++) {
                            if (g->cpoints[cp].x != CPOINT_UNDEFINED) {
                                fprintf(f, "  Point %d: (%d, %d)\n",
                                    cp, g->cpoints[cp].x, g->cpoints[cp].y);
                            }
                        }
                        fclose(f);
                    }
                }
            } else {
                printf("  Failed to export: %s\n", output_file);
            }
        }
    }
    
    printf("\nTotal exported: %d graphics\n", exported);
    
    // Exportar paleta se existir
    if (fpg->has_palette) {
        char palette_file[512];
        snprintf(palette_file, sizeof(palette_file), "%s/palette.txt", output_dir);
        FILE* f = fopen(palette_file, "w");
        if (f) {
            fprintf(f, "DIV Palette (256 colors)\n\n");
            for (int i = 0; i < 256; i++) {
                fprintf(f, "%3d: RGB(%3d, %3d, %3d) A=%3d\n",
                    i, fpg->palette[i].r, fpg->palette[i].g, 
                    fpg->palette[i].b, fpg->palette[i].a);
            }
            fclose(f);
            printf("Palette exported to: %s\n", palette_file);
        }
    }
    
    DIV_FreeFPG(fpg);
    CloseWindow();
    return 0;
}

int ConvertMAP(const char* input, const char* output) {
    printf("Loading MAP: %s\n", input);
    
    SetTraceLogLevel(LOG_WARNING);
    InitWindow(1, 1, "");
    
    DIV_GRAPHIC* map = DIV_LoadMAP(input);
    if (!map) {
        CloseWindow();
        printf("Error: Could not load MAP file\n");
        return 1;
    }
    
    printf("Map: %s (%dx%d)\n", map->name, map->width, map->height);
    
    if (ExportImage(map->image, output)) {
        printf("Exported to: %s\n", output);
    } else {
        printf("Error: Could not export image\n");
        DIV_FreeGraphic(map);
        CloseWindow();
        return 1;
    }
    
    DIV_FreeGraphic(map);
    CloseWindow();
    return 0;
}

int ConvertFont(const char* input, const char* output_dir) {
    printf("Loading Font: %s\n", input);
    
    SetTraceLogLevel(LOG_WARNING);
    InitWindow(1, 1, "");
    
    DIV_FONT* font = DIV_LoadFont(input);
    if (!font) {
        CloseWindow();
        printf("Error: Could not load font file\n");
        return 1;
    }
    
    printf("Font charset: %d, bpp: %d\n", font->charset, font->bpp);
    
    // Criar diretório
    EnsureDirectory(output_dir);
    
    // Exportar informações da fonte
    char info_file[512];
    snprintf(info_file, sizeof(info_file), "%s/font_info.txt", output_dir);
    FILE* f = fopen(info_file, "w");
    if (f) {
        fprintf(f, "DIV Font Information\n");
        fprintf(f, "====================\n\n");
        fprintf(f, "Charset: %d\n", font->charset);
        fprintf(f, "Bits per pixel: %d\n", font->bpp);
        fprintf(f, "Has palette: %s\n\n", font->has_palette ? "Yes" : "No");
        fprintf(f, "Character Data:\n");
        fprintf(f, "---------------\n");
        
        for (int i = 0; i < 256; i++) {
            if (font->chardata[i].width > 0) {
                fprintf(f, "Char %3d (0x%02X '%c'): %dx%d, advance: (%d,%d), offset: (%d,%d)\n",
                    i, i, (i >= 32 && i < 127) ? i : '?',
                    font->chardata[i].width, font->chardata[i].height,
                    font->chardata[i].xadvance, font->chardata[i].yadvance,
                    font->chardata[i].xoffset, font->chardata[i].yoffset);
            }
        }
        fclose(f);
        printf("Font info saved to: %s\n", info_file);
    }
    
    // Exportar cada glifo
    int exported = 0;
    for (int i = 0; i < 256; i++) {
        if (font->glyphs[i].data) {
            char glyph_file[512];
            snprintf(glyph_file, sizeof(glyph_file), 
                "%s/char_%03d.png", output_dir, i);
            
            if (ExportImage(font->glyphs[i], glyph_file)) {
                exported++;
            }
        }
    }
    
    printf("Exported %d glyphs\n", exported);
    
    // Criar sheet com todos os caracteres ASCII imprimíveis
    int chars_per_row = 16;
    int char_width = 16;
    int char_height = 16;
    
    // Calcular tamanho máximo dos caracteres
    for (int i = 32; i < 127; i++) {
        if (font->chardata[i].width > char_width) {
            char_width = font->chardata[i].width;
        }
        if (font->chardata[i].height > char_height) {
            char_height = font->chardata[i].height;
        }
    }
    
    int sheet_width = chars_per_row * (char_width + 2);
    int sheet_height = 6 * (char_height + 2);
    
    Image sheet = GenImageColor(sheet_width, sheet_height, BLACK);
    
    for (int i = 32; i < 127; i++) {
        if (font->glyphs[i].data) {
            int col = (i - 32) % chars_per_row;
            int row = (i - 32) / chars_per_row;
            int x = col * (char_width + 2) + 1;
            int y = row * (char_height + 2) + 1;
            
            Rectangle dest = { x, y, font->glyphs[i].width, font->glyphs[i].height };
            ImageDraw(&sheet, font->glyphs[i], 
                (Rectangle){ 0, 0, font->glyphs[i].width, font->glyphs[i].height },
                dest, WHITE);
        }
    }
    
    char sheet_file[512];
    snprintf(sheet_file, sizeof(sheet_file), "%s/font_sheet.png", output_dir);
    ExportImage(sheet, sheet_file);
    printf("Font sheet saved to: %s\n", sheet_file);
    
    UnloadImage(sheet);
    DIV_FreeFont(font);
    CloseWindow();
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        PrintUsage(argv[0]);
        return 1;
    }
    
    const char* input = argv[1];
    const char* output = argv[2];
    
    // Verificar se arquivo de entrada existe
    FILE* test = fopen(input, "rb");
    if (!test) {
        printf("Error: Input file does not exist: %s\n", input);
        return 1;
    }
    fclose(test);
    
    // Determinar tipo de conversão baseado na extensão
    const char* ext = GetFileExtension(input);
    
    if (strcasecmp(ext, "fpg") == 0 || 
        strcasecmp(ext, "f32") == 0 ||
        strcasecmp(ext, "f16") == 0 ||
        strcasecmp(ext, "f01") == 0) {
        
        if (!IsDirectory(output)) {
            printf("Output must be a directory for FPG conversion\n");
            return 1;
        }
        return ConvertFPG(input, output);
        
    } else if (strcasecmp(ext, "map") == 0 ||
               strcasecmp(ext, "m32") == 0 ||
               strcasecmp(ext, "m16") == 0 ||
               strcasecmp(ext, "m01") == 0) {
        
        if (IsDirectory(output)) {
            printf("Output must be a file for MAP conversion\n");
            return 1;
        }
        return ConvertMAP(input, output);
        
    } else if (strcasecmp(ext, "fnt") == 0 ||
               strcasecmp(ext, "fnx") == 0) {
        
        if (!IsDirectory(output)) {
            printf("Output must be a directory for font conversion\n");
            return 1;
        }
        return ConvertFont(input, output);
        
    } else {
        printf("Error: Unknown file format: .%s\n", ext);
        PrintUsage(argv[0]);
        return 1;
    }
    
    return 0;
}
