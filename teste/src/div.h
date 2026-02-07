#ifndef __FILE_DIV_RAYLIB_H
#define __FILE_DIV_RAYLIB_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <raylib.h>

/* --------------------------------------------------------------------------- */
/* MAGIC NUMBERS - File format identifiers */
/* --------------------------------------------------------------------------- */

#define MAP_MAGIC "map\x1A\x0D\x0A\x00"
#define M32_MAGIC "m32\x1A\x0D\x0A\x00"
#define M16_MAGIC "m16\x1A\x0D\x0A\x00"
#define M01_MAGIC "m01\x1A\x0D\x0A\x00"

#define PAL_MAGIC "pal\x1A\x0D\x0A\x00"

#define FNT_MAGIC "fnt\x1A\x0D\x0A\x00"
#define FNX_MAGIC "fnx\x1A\x0D\x0A\x00"

#define FPG_MAGIC "fpg\x1A\x0D\x0A\x00"
#define F32_MAGIC "f32\x1A\x0D\x0A\x00"
#define F16_MAGIC "f16\x1A\x0D\x0A\x00"
#define F01_MAGIC "f01\x1A\x0D\x0A\x00"

/* --------------------------------------------------------------------------- */
/* STRUCTURES */
/* --------------------------------------------------------------------------- */

#pragma pack(push, 1)
typedef struct
{
    uint8_t magic[7];
    uint8_t version;
    uint16_t width;
    uint16_t height;
    uint32_t code;
    int8_t name[32];
} MAP_HEADER;
#pragma pack(pop)

/* Control Point */
typedef struct
{
    int16_t x;
    int16_t y;
} CPOINT;

#define CPOINT_UNDEFINED -32768

/* DIV Graphic */
typedef struct
{
    uint32_t code;
    char name[32];
    uint16_t width;
    uint16_t height;
    Image image;       // Raylib Image
    Texture2D texture; // Raylib Texture (GPU)
    int ncpoints;      // Number of control points
    CPOINT *cpoints;   // Control points array
} DIV_GRAPHIC;

/* FPG Library - Collection of graphics */
typedef struct
{
    int num_graphics;
    int capacity;
    DIV_GRAPHIC **graphics; // Array of graphics
    Color palette[256];     // For 8-bit images
    int has_palette;
} DIV_FPG;

/* Font Character Data */
typedef struct
{
    int width;
    int height;
    int xadvance;
    int yadvance;
    int xoffset;
    int yoffset;
    int fileoffset;
} DIV_CHARDATA;

/* DIV Font */
typedef struct
{
    int charset;
    int bpp;
    DIV_CHARDATA chardata[256];
    Image glyphs[256];
    Texture2D textures[256];
    Color palette[256];
    int has_palette;
} DIV_FONT;

/* --------------------------------------------------------------------------- */
/* BYTE ORDER CONVERSION (for cross-platform compatibility) */
/* --------------------------------------------------------------------------- */

#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define NEEDS_SWAP 1
#else
#define NEEDS_SWAP 0
#endif

static inline uint16_t swap16(uint16_t val)
{
    return (val << 8) | (val >> 8);
}

static inline uint32_t swap32(uint32_t val)
{
    return ((val << 24) & 0xFF000000) |
           ((val << 8) & 0x00FF0000) |
           ((val >> 8) & 0x0000FF00) |
           ((val >> 24) & 0x000000FF);
}

#if NEEDS_SWAP
#define ARRANGE_WORD(p) (*(uint16_t *)(p) = swap16(*(uint16_t *)(p)))
#define ARRANGE_DWORD(p) (*(uint32_t *)(p) = swap32(*(uint32_t *)(p)))
#else
#define ARRANGE_WORD(p)
#define ARRANGE_DWORD(p)
#endif

/* --------------------------------------------------------------------------- */
/* FUNCTION DECLARATIONS */
/* --------------------------------------------------------------------------- */

/* FPG Functions */
DIV_FPG *DIV_LoadFPG(const char *filename);
void DIV_FreeFPG(DIV_FPG *fpg);
DIV_GRAPHIC *DIV_GetGraphic(DIV_FPG *fpg, int code);
Texture2D DIV_GetTexture(DIV_FPG *fpg, int code);

/* MAP Functions */
DIV_GRAPHIC *DIV_LoadMAP(const char *filename);
void DIV_FreeGraphic(DIV_GRAPHIC *graphic);

/* Font Functions */
DIV_FONT *DIV_LoadFont(const char *filename);
void DIV_FreeFont(DIV_FONT *font);
void DIV_DrawText(DIV_FONT *font, const char *text, int x, int y, Color tint);

/* Helper Functions */
int DIV_SaveImage(DIV_GRAPHIC *graphic, const char *filename);
void DIV_DrawGraphic(DIV_GRAPHIC *graphic, int x, int y, Color tint);
void DIV_DrawGraphicEx(DIV_GRAPHIC *graphic, Vector2 position, float rotation,
                       float scale, Color tint);

#endif // __FILE_DIV_RAYLIB_H
