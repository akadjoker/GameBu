/*
 *  DIV/Bennu File Loader for Raylib - Implementation
 *  Adapted to use Raylib instead of SDL
 */

#include "file_div_raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* --------------------------------------------------------------------------- */
/* PALETTE LOADING */
/* --------------------------------------------------------------------------- */

static int ReadPalette(FILE *fp, Color *palette)
{
    uint8_t colors[768];

    if (fread(colors, 1, 768, fp) != 768)
    {
        return 0;
    }

    // Convert 6-bit color values to 8-bit
    for (int i = 0; i < 256; i++)
    {
        palette[i].r = colors[i * 3 + 0] << 2;
        palette[i].g = colors[i * 3 + 1] << 2;
        palette[i].b = colors[i * 3 + 2] << 2;
        palette[i].a = 255;
    }

    return 1;
}

static int ReadPaletteWithGamma(FILE *fp, Color *palette)
{
    if (!ReadPalette(fp, palette))
    {
        return 0;
    }

    // Skip gamma correction data
    fseek(fp, 576, SEEK_CUR);

    return 1;
}

/* --------------------------------------------------------------------------- */
/* FPG LOADING */
/* --------------------------------------------------------------------------- */

DIV_FPG *DIV_LoadFPG(const char *filename)
{
    FILE *fp;
    char header[8];
    int bpp;

    fp = fopen(filename, "rb");
    if (!fp)
    {
        TraceLog(LOG_ERROR, "DIV_LoadFPG: Cannot open file %s", filename);
        return NULL;
    }

    // Read and validate header
    if (fread(header, 1, 8, fp) != 8)
    {
        fclose(fp);
        TraceLog(LOG_ERROR, "DIV_LoadFPG: Cannot read header");
        return NULL;
    }

    // Determine bit depth from magic number
    if (memcmp(header, F32_MAGIC, 7) == 0)
        bpp = 32;
    else if (memcmp(header, F16_MAGIC, 7) == 0)
        bpp = 16;
    else if (memcmp(header, FPG_MAGIC, 7) == 0)
        bpp = 8;
    else if (memcmp(header, F01_MAGIC, 7) == 0)
        bpp = 1;
    else
    {
        fclose(fp);
        TraceLog(LOG_ERROR, "DIV_LoadFPG: Invalid magic number");
        return NULL;
    }

    // Create FPG structure
    DIV_FPG *fpg = (DIV_FPG *)malloc(sizeof(DIV_FPG));
    if (!fpg)
    {
        fclose(fp);
        return NULL;
    }

    fpg->num_graphics = 0;
    fpg->capacity = 1000; // DIV standard max
    fpg->graphics = (DIV_GRAPHIC **)calloc(fpg->capacity, sizeof(DIV_GRAPHIC *));
    fpg->has_palette = 0;

    if (!fpg->graphics)
    {
        free(fpg);
        fclose(fp);
        return NULL;
    }

    // Read palette for 8-bit images
    if (bpp == 8)
    {
        if (!ReadPaletteWithGamma(fp, fpg->palette))
        {
            free(fpg->graphics);
            free(fpg);
            fclose(fp);
            TraceLog(LOG_ERROR, "DIV_LoadFPG: Cannot read palette");
            return NULL;
        }
        fpg->has_palette = 1;
        fpg->palette[0].a = 0; // First color is transparent
    }

    // Read each graphic chunk
    while (!feof(fp))
    {
        struct
        {
            uint32_t code;
            uint32_t regsize;
            char name[32];
            char fpname[12];
            uint32_t width;
            uint32_t height;
            uint32_t flags;
        } chunk;

        if (fread(&chunk, 1, sizeof(chunk), fp) != sizeof(chunk))
        {
            break;
        }

        // Arrange byte order
        ARRANGE_DWORD(&chunk.code);
        ARRANGE_DWORD(&chunk.regsize);
        ARRANGE_DWORD(&chunk.width);
        ARRANGE_DWORD(&chunk.height);
        ARRANGE_DWORD(&chunk.flags);

        if (chunk.code >= fpg->capacity)
        {
            break;
        }

        // Create graphic structure
        DIV_GRAPHIC *graphic = (DIV_GRAPHIC *)malloc(sizeof(DIV_GRAPHIC));
        if (!graphic)
        {
            continue;
        }

        graphic->code = chunk.code;
        graphic->width = chunk.width;
        graphic->height = chunk.height;
        memcpy(graphic->name, chunk.name, 32);
        graphic->name[31] = 0;
        graphic->ncpoints = chunk.flags;
        graphic->cpoints = NULL;

        // Read control points
        if (graphic->ncpoints > 0)
        {
            graphic->cpoints = (CPOINT *)malloc(graphic->ncpoints * sizeof(CPOINT));
            if (graphic->cpoints)
            {
                for (int c = 0; c < graphic->ncpoints; c++)
                {
                    int16_t px, py;
                    fread(&px, sizeof(int16_t), 1, fp);
                    fread(&py, sizeof(int16_t), 1, fp);
                    ARRANGE_WORD(&px);
                    ARRANGE_WORD(&py);

                    if (px == -1 && py == -1)
                    {
                        graphic->cpoints[c].x = CPOINT_UNDEFINED;
                        graphic->cpoints[c].y = CPOINT_UNDEFINED;
                    }
                    else
                    {
                        graphic->cpoints[c].x = px;
                        graphic->cpoints[c].y = py;
                    }
                }
            }
        }

        // Calculate bytes per line
        int widthb = (chunk.width * bpp + 7) / 8;

        // Allocate pixel data
        int format;
        int pixelSize;

        switch (bpp)
        {
        case 32:
            format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
            pixelSize = 4;
            break;
        case 16:
            format = PIXELFORMAT_UNCOMPRESSED_R5G6B5;
            pixelSize = 2;
            break;
        case 8:
            format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8; // We'll convert to RGBA
            pixelSize = 4;
            break;
        case 1:
            format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8; // We'll convert to RGBA
            pixelSize = 4;
            break;
        default:
            format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
            pixelSize = 4;
            break;
        }

        // Create image
        graphic->image.width = chunk.width;
        graphic->image.height = chunk.height;
        graphic->image.format = format;
        graphic->image.mipmaps = 1;

        // For 8-bit and 1-bit, we need to convert to RGBA
        if (bpp == 8 || bpp == 1)
        {
            graphic->image.data = malloc(chunk.width * chunk.height * 4);
            uint8_t *dest = (uint8_t *)graphic->image.data;

            for (int y = 0; y < chunk.height; y++)
            {
                uint8_t *line = (uint8_t *)malloc(widthb);
                fread(line, 1, widthb, fp);

                for (int x = 0; x < chunk.width; x++)
                {
                    uint8_t colorIndex;

                    if (bpp == 1)
                    {
                        int bytePos = x / 8;
                        int bitPos = 7 - (x % 8);
                        colorIndex = (~line[bytePos] >> bitPos) & 1;
                    }
                    else
                    {
                        colorIndex = line[x];
                    }

                    int destPos = (y * chunk.width + x) * 4;

                    if (fpg->has_palette)
                    {
                        dest[destPos + 0] = fpg->palette[colorIndex].r;
                        dest[destPos + 1] = fpg->palette[colorIndex].g;
                        dest[destPos + 2] = fpg->palette[colorIndex].b;
                        dest[destPos + 3] = fpg->palette[colorIndex].a;
                    }
                    else
                    {
                        // Grayscale
                        dest[destPos + 0] = colorIndex;
                        dest[destPos + 1] = colorIndex;
                        dest[destPos + 2] = colorIndex;
                        dest[destPos + 3] = 255;
                    }
                }

                free(line);
            }
        }
        else
        {
            // 16-bit or 32-bit - direct read
            graphic->image.data = malloc(chunk.width * chunk.height * pixelSize);

            for (int y = 0; y < chunk.height; y++)
            {
                uint8_t *dest = (uint8_t *)graphic->image.data + y * chunk.width * pixelSize;
                fread(dest, pixelSize, chunk.width, fp);

                // Arrange byte order for 16-bit and 32-bit
                if (bpp == 16)
                {
                    uint16_t *pixels = (uint16_t *)dest;
                    for (int x = 0; x < chunk.width; x++)
                    {
                        ARRANGE_WORD(&pixels[x]);
                    }
                }
                else if (bpp == 32)
                {
                    uint32_t *pixels = (uint32_t *)dest;
                    for (int x = 0; x < chunk.width; x++)
                    {
                        ARRANGE_DWORD(&pixels[x]);
                    }
                }
            }
        }

        // Create texture from image
        graphic->texture = LoadTextureFromImage(graphic->image);

        // Store in FPG
        fpg->graphics[chunk.code] = graphic;
        fpg->num_graphics++;

        TraceLog(LOG_INFO, "DIV_LoadFPG: Loaded graphic %d '%s' (%dx%d)",
                 chunk.code, graphic->name, chunk.width, chunk.height);
    }

    fclose(fp);
    TraceLog(LOG_INFO, "DIV_LoadFPG: Loaded %d graphics from %s", fpg->num_graphics, filename);
    return fpg;
}

void DIV_FreeFPG(DIV_FPG *fpg)
{
    if (!fpg)
        return;

    for (int i = 0; i < fpg->capacity; i++)
    {
        if (fpg->graphics[i])
        {
            DIV_FreeGraphic(fpg->graphics[i]);
        }
    }

    free(fpg->graphics);
    free(fpg);
}

DIV_GRAPHIC *DIV_GetGraphic(DIV_FPG *fpg, int code)
{
    if (!fpg || code < 0 || code >= fpg->capacity)
    {
        return NULL;
    }
    return fpg->graphics[code];
}

Texture2D DIV_GetTexture(DIV_FPG *fpg, int code)
{
    DIV_GRAPHIC *graphic = DIV_GetGraphic(fpg, code);
    if (graphic)
    {
        return graphic->texture;
    }
    return (Texture2D){0};
}

/* --------------------------------------------------------------------------- */
/* MAP LOADING (Single Image) */
/* --------------------------------------------------------------------------- */

DIV_GRAPHIC *DIV_LoadMAP(const char *filename)
{
    FILE *fp;
    MAP_HEADER header;

    fp = fopen(filename, "rb");
    if (!fp)
    {
        TraceLog(LOG_ERROR, "DIV_LoadMAP: Cannot open file %s", filename);
        return NULL;
    }

    // Read header
    if (fread(&header, 1, sizeof(MAP_HEADER), fp) != sizeof(MAP_HEADER))
    {
        fclose(fp);
        TraceLog(LOG_ERROR, "DIV_LoadMAP: Cannot read header");
        return NULL;
    }

    // Validate magic number
    int bpp;
    if (memcmp(header.magic, M32_MAGIC, 7) == 0)
        bpp = 32;
    else if (memcmp(header.magic, M16_MAGIC, 7) == 0)
        bpp = 16;
    else if (memcmp(header.magic, MAP_MAGIC, 7) == 0)
        bpp = 8;
    else if (memcmp(header.magic, M01_MAGIC, 7) == 0)
        bpp = 1;
    else
    {
        fclose(fp);
        TraceLog(LOG_ERROR, "DIV_LoadMAP: Invalid magic number");
        return NULL;
    }

    // Arrange byte order
    ARRANGE_WORD(&header.width);
    ARRANGE_WORD(&header.height);
    ARRANGE_DWORD(&header.code);

    // Create graphic
    DIV_GRAPHIC *graphic = (DIV_GRAPHIC *)malloc(sizeof(DIV_GRAPHIC));
    if (!graphic)
    {
        fclose(fp);
        return NULL;
    }

    graphic->code = header.code;
    graphic->width = header.width;
    graphic->height = header.height;
    memcpy(graphic->name, header.name, 32);
    graphic->name[31] = 0;
    graphic->ncpoints = 0;
    graphic->cpoints = NULL;

    // Read palette if 8-bit
    Color palette[256];
    int has_palette = 0;

    if (bpp == 8)
    {
        if (!ReadPaletteWithGamma(fp, palette))
        {
            free(graphic);
            fclose(fp);
            return NULL;
        }
        has_palette = 1;
        palette[0].a = 0; // Transparent color
    }

    // Calculate bytes per line
    int widthb = (header.width * bpp + 7) / 8;

    // Create image based on bit depth
    graphic->image.width = header.width;
    graphic->image.height = header.height;
    graphic->image.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
    graphic->image.mipmaps = 1;

    if (bpp == 8 || bpp == 1)
    {
        // Convert indexed to RGBA
        graphic->image.data = malloc(header.width * header.height * 4);
        uint8_t *dest = (uint8_t *)graphic->image.data;

        for (int y = 0; y < header.height; y++)
        {
            uint8_t *line = (uint8_t *)malloc(widthb);
            fread(line, 1, widthb, fp);

            for (int x = 0; x < header.width; x++)
            {
                uint8_t colorIndex;

                if (bpp == 1)
                {
                    int bytePos = x / 8;
                    int bitPos = 7 - (x % 8);
                    colorIndex = (~line[bytePos] >> bitPos) & 1;
                }
                else
                {
                    colorIndex = line[x];
                }

                int destPos = (y * header.width + x) * 4;

                if (has_palette)
                {
                    dest[destPos + 0] = palette[colorIndex].r;
                    dest[destPos + 1] = palette[colorIndex].g;
                    dest[destPos + 2] = palette[colorIndex].b;
                    dest[destPos + 3] = palette[colorIndex].a;
                }
                else
                {
                    dest[destPos + 0] = colorIndex;
                    dest[destPos + 1] = colorIndex;
                    dest[destPos + 2] = colorIndex;
                    dest[destPos + 3] = 255;
                }
            }

            free(line);
        }
    }
    else
    {
        // 16-bit or 32-bit
        int pixelSize = (bpp == 32) ? 4 : 2;
        if (bpp == 16)
        {
            graphic->image.format = PIXELFORMAT_UNCOMPRESSED_R5G6B5;
        }

        graphic->image.data = malloc(header.width * header.height * pixelSize);

        for (int y = 0; y < header.height; y++)
        {
            uint8_t *dest = (uint8_t *)graphic->image.data + y * header.width * pixelSize;
            fread(dest, pixelSize, header.width, fp);

            if (bpp == 16)
            {
                uint16_t *pixels = (uint16_t *)dest;
                for (int x = 0; x < header.width; x++)
                {
                    ARRANGE_WORD(&pixels[x]);
                }
            }
            else if (bpp == 32)
            {
                uint32_t *pixels = (uint32_t *)dest;
                for (int x = 0; x < header.width; x++)
                {
                    ARRANGE_DWORD(&pixels[x]);
                }
            }
        }
    }

    fclose(fp);

    // Create texture
    graphic->texture = LoadTextureFromImage(graphic->image);

    TraceLog(LOG_INFO, "DIV_LoadMAP: Loaded '%s' (%dx%d)",
             graphic->name, header.width, header.height);

    return graphic;
}

void DIV_FreeGraphic(DIV_GRAPHIC *graphic)
{
    if (!graphic)
        return;

    if (graphic->texture.id > 0)
    {
        UnloadTexture(graphic->texture);
    }

    if (graphic->image.data)
    {
        UnloadImage(graphic->image);
    }

    if (graphic->cpoints)
    {
        free(graphic->cpoints);
    }

    free(graphic);
}

/* --------------------------------------------------------------------------- */
/* FONT LOADING */
/* --------------------------------------------------------------------------- */

DIV_FONT *DIV_LoadFont(const char *filename)
{
    FILE *fp;
    char header[8];

    fp = fopen(filename, "rb");
    if (!fp)
    {
        TraceLog(LOG_ERROR, "DIV_LoadFont: Cannot open file %s", filename);
        return NULL;
    }

    // Read header
    if (fread(header, 1, 8, fp) != 8)
    {
        fclose(fp);
        return NULL;
    }

    // Validate magic
    if (memcmp(header, FNT_MAGIC, 7) != 0 && memcmp(header, FNX_MAGIC, 7) != 0)
    {
        fclose(fp);
        TraceLog(LOG_ERROR, "DIV_LoadFont: Invalid magic number");
        return NULL;
    }

    int bpp = header[7];

    // Create font structure
    DIV_FONT *font = (DIV_FONT *)malloc(sizeof(DIV_FONT));
    if (!font)
    {
        fclose(fp);
        return NULL;
    }

    font->bpp = bpp;
    font->has_palette = 0;
    memset(font->glyphs, 0, sizeof(font->glyphs));
    memset(font->textures, 0, sizeof(font->textures));

    // Read charset
    fread(&font->charset, sizeof(int32_t), 1, fp);
    ARRANGE_DWORD(&font->charset);

    // Read palette if 8-bit
    if (bpp == 8)
    {
        if (!ReadPaletteWithGamma(fp, font->palette))
        {
            free(font);
            fclose(fp);
            return NULL;
        }
        font->has_palette = 1;
    }

    // Read character data
    fread(font->chardata, sizeof(DIV_CHARDATA), 256, fp);

    // Arrange byte order for all character data
    for (int i = 0; i < 256; i++)
    {
        ARRANGE_DWORD(&font->chardata[i].width);
        ARRANGE_DWORD(&font->chardata[i].height);
        ARRANGE_DWORD(&font->chardata[i].xadvance);
        ARRANGE_DWORD(&font->chardata[i].yadvance);
        ARRANGE_DWORD(&font->chardata[i].xoffset);
        ARRANGE_DWORD(&font->chardata[i].yoffset);
        ARRANGE_DWORD(&font->chardata[i].fileoffset);
    }

    // Load each character glyph
    for (int n = 0; n < 256; n++)
    {
        if (font->chardata[n].width == 0 || font->chardata[n].height == 0)
        {
            continue;
        }

        fseek(fp, font->chardata[n].fileoffset, SEEK_SET);

        int width = font->chardata[n].width;
        int height = font->chardata[n].height;
        int widthb = (width * bpp + 7) / 8;

        // Create glyph image
        font->glyphs[n].width = width;
        font->glyphs[n].height = height;
        font->glyphs[n].format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
        font->glyphs[n].mipmaps = 1;
        font->glyphs[n].data = malloc(width * height * 4);

        uint8_t *dest = (uint8_t *)font->glyphs[n].data;

        for (int y = 0; y < height; y++)
        {
            uint8_t *line = (uint8_t *)malloc(widthb);
            fread(line, 1, widthb, fp);

            for (int x = 0; x < width; x++)
            {
                uint8_t colorIndex;

                if (bpp == 8)
                {
                    colorIndex = line[x];
                }
                else
                {
                    // For other bpp, simplified handling
                    colorIndex = line[x * bpp / 8];
                }

                int destPos = (y * width + x) * 4;

                if (font->has_palette)
                {
                    dest[destPos + 0] = font->palette[colorIndex].r;
                    dest[destPos + 1] = font->palette[colorIndex].g;
                    dest[destPos + 2] = font->palette[colorIndex].b;
                    dest[destPos + 3] = font->palette[colorIndex].a;
                }
                else
                {
                    dest[destPos + 0] = 255;
                    dest[destPos + 1] = 255;
                    dest[destPos + 2] = 255;
                    dest[destPos + 3] = colorIndex;
                }
            }

            free(line);
        }

        font->textures[n] = LoadTextureFromImage(font->glyphs[n]);
    }

    fclose(fp);
    TraceLog(LOG_INFO, "DIV_LoadFont: Loaded font %s", filename);
    return font;
}

void DIV_FreeFont(DIV_FONT *font)
{
    if (!font)
        return;

    for (int i = 0; i < 256; i++)
    {
        if (font->textures[i].id > 0)
        {
            UnloadTexture(font->textures[i]);
        }
        if (font->glyphs[i].data)
        {
            UnloadImage(font->glyphs[i]);
        }
    }

    free(font);
}

void DIV_DrawText(DIV_FONT *font, const char *text, int x, int y, Color tint)
{
    if (!font || !text)
        return;

    int cursorX = x;
    int cursorY = y;

    while (*text)
    {
        unsigned char c = (unsigned char)*text;

        if (font->textures[c].id > 0)
        {
            int drawX = cursorX + font->chardata[c].xoffset;
            int drawY = cursorY + font->chardata[c].yoffset;

            DrawTexture(font->textures[c], drawX, drawY, tint);

            cursorX += font->chardata[c].xadvance;
            cursorY += font->chardata[c].yadvance;
        }

        text++;
    }
}

/* --------------------------------------------------------------------------- */
/* HELPER FUNCTIONS */
/* --------------------------------------------------------------------------- */

void DIV_DrawGraphic(DIV_GRAPHIC *graphic, int x, int y, Color tint)
{
    if (!graphic || graphic->texture.id == 0)
        return;

    DrawTexture(graphic->texture, x, y, tint);
}

void DIV_DrawGraphicEx(DIV_GRAPHIC *graphic, Vector2 position, float rotation,
                       float scale, Color tint)
{
    if (!graphic || graphic->texture.id == 0)
        return;

    Rectangle source = {0, 0, (float)graphic->width, (float)graphic->height};
    Rectangle dest = {position.x, position.y, graphic->width * scale, graphic->height * scale};
    Vector2 origin = {0, 0};

    // Use first control point as origin if available
    if (graphic->ncpoints > 0 && graphic->cpoints[0].x != CPOINT_UNDEFINED)
    {
        origin.x = graphic->cpoints[0].x * scale;
        origin.y = graphic->cpoints[0].y * scale;
    }

    DrawTexturePro(graphic->texture, source, dest, origin, rotation, tint);
}

int DIV_SaveImage(DIV_GRAPHIC *graphic, const char *filename)
{
    if (!graphic)
        return 0;

    return ExportImage(graphic->image, filename);
}
