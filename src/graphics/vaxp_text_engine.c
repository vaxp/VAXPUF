/*
 * VAXPUI Text Engine Implementation
 *
 * Stack: FreeType 2 + HarfBuzz + FriBidi + GPU Glyph Atlas
 *
 * Pipeline:
 *   UTF-8 → FriBidi (BiDi) → HarfBuzz (Shape) → FreeType (Raster) → Atlas → GL Batch
 */

#define _GNU_SOURCE
#include "vaxp_text_engine.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

/* FreeType */
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_BITMAP_H
#include FT_OUTLINE_H

/* HarfBuzz */
#include <hb.h>
#include <hb-ft.h>

/* FriBidi */
#include <fribidi.h>

/* OpenGL */
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glext.h>

/* ============================================================
 * CONSTANTS
 * ============================================================ */

#define TE_ATLAS_SIZE       2048    /* Atlas texture dimension (2048x2048) */
#define TE_ATLAS_PADDING    2       /* Pixels between glyphs */
#define TE_MAX_FONTS        32      /* Max simultaneously loaded font faces */
#define TE_GLYPH_CACHE_MAX  8192    /* Max cached glyphs (per font) */
#define TE_BATCH_MAX        65536   /* Max vertices per flush */
#define TE_MAX_TEXT_LEN     4096    /* Max chars in a single draw_string call */

/* ============================================================
 * TYPES
 * ============================================================ */

/* Cached glyph entry in atlas */
typedef struct {
    uint32_t codepoint;    /* Unicode codepoint */
    VaxpFontId font_id;    /* Which font */
    
    /* UV coordinates in atlas texture [0..1] */
    float u0, v0, u1, v1;
    
    /* Rendering metrics (in pixels) */
    int   bitmap_w, bitmap_h;
    int   bearing_x, bearing_y; /* FreeType bitmap_left, bitmap_top */
    float advance;               /* Horizontal advance in pixels */
    
    bool  is_color;  /* True for color Emoji (RGBA) */
} GlyphEntry;

/* Glyph cache hash map - open addressing */
typedef struct {
    uint64_t    key;       /* (font_id << 32) | codepoint */
    GlyphEntry* entry;     /* Pointer into entry pool, NULL = empty slot */
} GlyphSlot;

#define GLYPH_MAP_SIZE  16384  /* Must be power of 2 */

/* Font face record */
typedef struct {
    FT_Face   ft_face;
    hb_font_t* hb_font;
    float     size_px;
    bool      in_use;
    char      path[256];
} FontRecord;

/* Interleaved vertex: pos(2) + uv(2) + color(4) + mode(1) */
typedef struct {
    float x, y;
    float u, v;
    float r, g, b, a;
    float mode;  /* 0 = solid color quad, 1 = grayscale atlas, 2 = color atlas */
} TeVertex;

/* ============================================================
 * GL FUNCTION POINTERS (loaded at runtime)
 * ============================================================ */

typedef void (*PFNGLGENBUFFERSPROC_TE)(GLsizei, GLuint*);
typedef void (*PFNGLBINDBUFFERPROC_TE)(GLenum, GLuint);
typedef void (*PFNGLBUFFERDATAPROC_TE)(GLenum, GLsizeiptr, const void*, GLenum);
typedef void (*PFNGLBUFFERSUBDATAPROC_TE)(GLenum, GLintptr, GLsizeiptr, const void*);
typedef void (*PFNGLDELETEBUFFERSPROC_TE)(GLsizei, const GLuint*);
typedef void (*PFNGLGENVERTEXARRAYSPROC_TE)(GLsizei, GLuint*);
typedef void (*PFNGLBINDVERTEXARRAYPROC_TE)(GLuint);
typedef void (*PFNGLDELETEVERTEXARRAYSPROC_TE)(GLsizei, const GLuint*);
typedef void (*PFNGLENABLEVERTEXATTRIBARRAYPROC_TE)(GLuint);
typedef void (*PFNGLVERTEXATTRIBPOINTERPROC_TE)(GLuint, GLint, GLenum, GLboolean, GLsizei, const void*);
typedef GLuint (*PFNGLCREATESHADERPROC_TE)(GLenum);
typedef void (*PFNGLSHADERSOURCEPROC_TE)(GLuint, GLsizei, const GLchar**, const GLint*);
typedef void (*PFNGLCOMPILESHADERPROC_TE)(GLuint);
typedef GLuint (*PFNGLCREATEPROGRAMPROC_TE)(void);
typedef void (*PFNGLATTACHSHADERPROC_TE)(GLuint, GLuint);
typedef void (*PFNGLLINKPROGRAMPROC_TE)(GLuint);
typedef void (*PFNGLUSEPROGRAMPROC_TE)(GLuint);
typedef void (*PFNGLDELETESHADERPROC_TE)(GLuint);
typedef void (*PFNGLDELETEPROGRAMPROC_TE)(GLuint);
typedef GLint (*PFNGLGETUNIFORMLOCATIONPROC_TE)(GLuint, const GLchar*);
typedef void (*PFNGLUNIFORM1IPROC_TE)(GLint, GLint);
typedef void (*PFNGLUNIFORMMATRIX4FVPROC_TE)(GLint, GLsizei, GLboolean, const GLfloat*);
typedef void (*PFNGLGETSHADERIVPROC_TE)(GLuint, GLenum, GLint*);
typedef void (*PFNGLGETPROGRAMIVPROC_TE)(GLuint, GLenum, GLint*);
typedef void (*PFNGLGETSHADERINFOLOGPROC_TE)(GLuint, GLsizei, GLsizei*, GLchar*);
typedef void (*PFNGLGETPROGRAMINFOLOGPROC_TE)(GLuint, GLsizei, GLsizei*, GLchar*);


static PFNGLGENBUFFERSPROC_TE           te_glGenBuffers;
static PFNGLBINDBUFFERPROC_TE           te_glBindBuffer;
static PFNGLBUFFERDATAPROC_TE           te_glBufferData;
static PFNGLBUFFERSUBDATAPROC_TE        te_glBufferSubData;
static PFNGLDELETEBUFFERSPROC_TE        te_glDeleteBuffers;
static PFNGLGENVERTEXARRAYSPROC_TE      te_glGenVertexArrays;
static PFNGLBINDVERTEXARRAYPROC_TE      te_glBindVertexArray;
static PFNGLDELETEVERTEXARRAYSPROC_TE   te_glDeleteVertexArrays;
static PFNGLENABLEVERTEXATTRIBARRAYPROC_TE te_glEnableVertexAttribArray;
static PFNGLVERTEXATTRIBPOINTERPROC_TE  te_glVertexAttribPointer;
static PFNGLCREATESHADERPROC_TE         te_glCreateShader;
static PFNGLSHADERSOURCEPROC_TE         te_glShaderSource;
static PFNGLCOMPILESHADERPROC_TE        te_glCompileShader;
static PFNGLCREATEPROGRAMPROC_TE        te_glCreateProgram;
static PFNGLATTACHSHADERPROC_TE         te_glAttachShader;
static PFNGLLINKPROGRAMPROC_TE          te_glLinkProgram;
static PFNGLUSEPROGRAMPROC_TE           te_glUseProgram;
static PFNGLDELETESHADERPROC_TE         te_glDeleteShader;
static PFNGLDELETEPROGRAMPROC_TE        te_glDeleteProgram;
static PFNGLGETUNIFORMLOCATIONPROC_TE   te_glGetUniformLocation;
static PFNGLUNIFORM1IPROC_TE            te_glUniform1i;
static PFNGLUNIFORMMATRIX4FVPROC_TE     te_glUniformMatrix4fv;
static PFNGLGETSHADERIVPROC_TE          te_glGetShaderiv;
static PFNGLGETPROGRAMIVPROC_TE         te_glGetProgramiv;
static PFNGLGETSHADERINFOLOGPROC_TE     te_glGetShaderInfoLog;
static PFNGLGETPROGRAMINFOLOGPROC_TE    te_glGetProgramInfoLog;

static bool te_gl_loaded = false;

static bool te_load_gl_functions(void) {
    if (te_gl_loaded) return true;
    #define TE_LOAD(var, name) \
        var = (void*)glXGetProcAddress((const GLubyte*)#name); \
        if (!var) { fprintf(stderr, "[TE] Failed to load " #name "\n"); }
    
    TE_LOAD(te_glGenBuffers,             glGenBuffers)
    TE_LOAD(te_glBindBuffer,             glBindBuffer)
    TE_LOAD(te_glBufferData,             glBufferData)
    TE_LOAD(te_glBufferSubData,          glBufferSubData)
    TE_LOAD(te_glDeleteBuffers,          glDeleteBuffers)
    TE_LOAD(te_glGenVertexArrays,        glGenVertexArrays)
    TE_LOAD(te_glBindVertexArray,        glBindVertexArray)
    TE_LOAD(te_glDeleteVertexArrays,     glDeleteVertexArrays)
    TE_LOAD(te_glEnableVertexAttribArray,glEnableVertexAttribArray)
    TE_LOAD(te_glVertexAttribPointer,    glVertexAttribPointer)
    TE_LOAD(te_glCreateShader,           glCreateShader)
    TE_LOAD(te_glShaderSource,           glShaderSource)
    TE_LOAD(te_glCompileShader,          glCompileShader)
    TE_LOAD(te_glCreateProgram,          glCreateProgram)
    TE_LOAD(te_glAttachShader,           glAttachShader)
    TE_LOAD(te_glLinkProgram,            glLinkProgram)
    TE_LOAD(te_glUseProgram,             glUseProgram)
    TE_LOAD(te_glDeleteShader,           glDeleteShader)
    TE_LOAD(te_glDeleteProgram,          glDeleteProgram)
    TE_LOAD(te_glGetUniformLocation,     glGetUniformLocation)
    TE_LOAD(te_glUniform1i,              glUniform1i)
    TE_LOAD(te_glUniformMatrix4fv,       glUniformMatrix4fv)
    TE_LOAD(te_glGetShaderiv,            glGetShaderiv)
    TE_LOAD(te_glGetProgramiv,           glGetProgramiv)
    TE_LOAD(te_glGetShaderInfoLog,       glGetShaderInfoLog)
    TE_LOAD(te_glGetProgramInfoLog,      glGetProgramInfoLog)
    #undef TE_LOAD
    
    te_gl_loaded = true;
    return true;
}

/* ============================================================
 * SHADERS
 * ============================================================ */

static const char* TE_VERT_SRC =
    "#version 330 core\n"
    "layout(location=0) in vec2 aPos;\n"
    "layout(location=1) in vec2 aUV;\n"
    "layout(location=2) in vec4 aColor;\n"
    "layout(location=3) in float aMode;\n"
    "out vec2 vUV;\n"
    "out vec4 vColor;\n"
    "out float vMode;\n"
    "uniform mat4 uProj;\n"
    "uniform mat4 uModel;\n"
    "void main() {\n"
    "    gl_Position = uProj * uModel * vec4(aPos, 0.0, 1.0);\n"
    "    vUV    = aUV;\n"
    "    vColor = aColor;\n"
    "    vMode  = aMode;\n"
    "}\n";

static const char* TE_FRAG_SRC =
    "#version 330 core\n"
    "in vec2  vUV;\n"
    "in vec4  vColor;\n"
    "in float vMode;\n"
    "out vec4 FragColor;\n"
    "uniform sampler2D uAtlas;\n"
    "void main() {\n"
    "    if (vMode < 0.5) {\n"
    "        /* Solid color quad (background) */\n"
    "        FragColor = vColor;\n"
    "    } else if (vMode < 1.5) {\n"
    "        /* Grayscale glyph: use red channel as alpha mask */\n"
    "        float alpha = texture(uAtlas, vUV).r;\n"
    "        FragColor = vec4(vColor.rgb, vColor.a * alpha);\n"
    "    } else {\n"
    "        /* Color glyph (Emoji CBDT/SBIX): full RGBA from atlas */\n"
    "        vec4 s = texture(uAtlas, vUV);\n"
    "        FragColor = vec4(s.rgb, s.a * vColor.a);\n"
    "    }\n"
    "}\n";

/* ============================================================
 * MAIN ENGINE STRUCT
 * ============================================================ */

struct VaxpTextEngine {
    /* FreeType library handle */
    FT_Library ft_lib;

    /* Font records */
    FontRecord  fonts[TE_MAX_FONTS];
    int         font_count;

    /* Glyph atlas - single 2048x2048 GL texture */
    GLuint      atlas_tex;
    int         atlas_x;   /* Next free column */
    int         atlas_y;   /* Current row top */
    int         atlas_row_h; /* Current row height */

    /* Glyph cache: hash map + pool */
    GlyphSlot   glyph_map[GLYPH_MAP_SIZE];
    GlyphEntry* glyph_pool;
    int         glyph_pool_count;
    int         glyph_pool_cap;

    /* GL batch */
    GLuint      vao, vbo;
    GLuint      shader;
    TeVertex*   batch;
    int         batch_count;

    /* Projection matrix (orthographic, updated on viewport change) */
    float       proj[16];
    int         vp_w, vp_h;  /* Last known viewport */
    
    /* Model transform matrix */
    float       model[16];
};

/* ============================================================
 * SHADER HELPERS
 * ============================================================ */

static GLuint te_compile_shader(GLenum type, const char* src) {
    GLuint s = te_glCreateShader(type);
    te_glShaderSource(s, 1, &src, NULL);
    te_glCompileShader(s);
    GLint ok = 0;
    te_glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char buf[512];
        te_glGetShaderInfoLog(s, sizeof(buf), NULL, buf);
        fprintf(stderr, "[TE] Shader compile error: %s\n", buf);
    }
    return s;
}

static GLuint te_link_program(const char* vert, const char* frag) {
    GLuint vs = te_compile_shader(GL_VERTEX_SHADER, vert);
    GLuint fs = te_compile_shader(GL_FRAGMENT_SHADER, frag);
    GLuint p  = te_glCreateProgram();
    te_glAttachShader(p, vs);
    te_glAttachShader(p, fs);
    te_glLinkProgram(p);
    
    GLint ok = 0;
    te_glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok) {
        char buf[512];
        te_glGetProgramInfoLog(p, sizeof(buf), NULL, buf);
        fprintf(stderr, "[TE] Program link error: %s\n", buf);
    }
    
    te_glDeleteShader(vs);
    te_glDeleteShader(fs);
    return p;
}

/* ============================================================
 * ORTHO PROJECTION
 * ============================================================ */

static void te_ortho(float* m, float l, float r, float b, float t) {
    memset(m, 0, 16 * sizeof(float));
    m[0]  =  2.0f / (r - l);
    m[5]  =  2.0f / (t - b);
    m[10] = -1.0f;
    m[12] = -(r + l) / (r - l);
    m[13] = -(t + b) / (t - b);
    m[15] =  1.0f;
}

static void te_update_projection(VaxpTextEngine* e) {
    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);
    if (vp[2] != e->vp_w || vp[3] != e->vp_h) {
        e->vp_w = vp[2];
        e->vp_h = vp[3];
        te_ortho(e->proj, 0, (float)vp[2], (float)vp[3], 0);
    }
}

/* ============================================================
 * GLYPH CACHE: hash map
 * ============================================================ */

static uint64_t te_glyph_key(VaxpFontId fid, uint32_t cp) {
    return ((uint64_t)(uint32_t)fid << 32) | (uint64_t)cp;
}

static GlyphEntry* te_cache_lookup(VaxpTextEngine* e, VaxpFontId fid, uint32_t cp) {
    uint64_t key = te_glyph_key(fid, cp);
    uint32_t idx = (uint32_t)(key ^ (key >> 32)) & (GLYPH_MAP_SIZE - 1);
    for (int i = 0; i < GLYPH_MAP_SIZE; i++) {
        uint32_t slot = (idx + i) & (GLYPH_MAP_SIZE - 1);
        if (!e->glyph_map[slot].entry) return NULL;
        if (e->glyph_map[slot].key == key) return e->glyph_map[slot].entry;
    }
    return NULL;
}

static GlyphEntry* te_cache_insert(VaxpTextEngine* e, VaxpFontId fid, uint32_t cp) {
    /* Grow pool if needed */
    if (e->glyph_pool_count >= e->glyph_pool_cap) {
        int new_cap = e->glyph_pool_cap * 2;
        e->glyph_pool = realloc(e->glyph_pool, new_cap * sizeof(GlyphEntry));
        e->glyph_pool_cap = new_cap;
    }
    GlyphEntry* entry = &e->glyph_pool[e->glyph_pool_count++];
    memset(entry, 0, sizeof(*entry));
    entry->codepoint = cp;
    entry->font_id   = fid;

    uint64_t key = te_glyph_key(fid, cp);
    uint32_t idx = (uint32_t)(key ^ (key >> 32)) & (GLYPH_MAP_SIZE - 1);
    for (int i = 0; i < GLYPH_MAP_SIZE; i++) {
        uint32_t slot = (idx + i) & (GLYPH_MAP_SIZE - 1);
        if (!e->glyph_map[slot].entry) {
            e->glyph_map[slot].key   = key;
            e->glyph_map[slot].entry = entry;
            return entry;
        }
    }
    /* Map full - shouldn't happen with GLYPH_MAP_SIZE=16384 */
    e->glyph_pool_count--;
    return NULL;
}

/* ============================================================
 * ATLAS MANAGEMENT
 * ============================================================ */

/* Reset atlas - called when atlas is full */
static void te_atlas_reset(VaxpTextEngine* e) {
    e->atlas_x     = TE_ATLAS_PADDING;
    e->atlas_y     = TE_ATLAS_PADDING;
    e->atlas_row_h = 0;
    /* Clear glyph cache so everything gets re-rasterized */
    memset(e->glyph_map,  0, sizeof(e->glyph_map));
    e->glyph_pool_count = 0;
    /* Clear the texture */
    uint8_t* zeros = calloc(TE_ATLAS_SIZE * TE_ATLAS_SIZE * 4, 1);
    glBindTexture(GL_TEXTURE_2D, e->atlas_tex);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, TE_ATLAS_SIZE, TE_ATLAS_SIZE,
                    GL_RGBA, GL_UNSIGNED_BYTE, zeros);
    free(zeros);
}

/* Upload a grayscale bitmap into the atlas, return UV coords */
static bool te_atlas_upload_gray(VaxpTextEngine* e,
                                   const uint8_t* bitmap, int w, int h,
                                   float* u0, float* v0, float* u1, float* v1) {
    if (w <= 0 || h <= 0) return false;

    if (e->atlas_x + w + TE_ATLAS_PADDING > TE_ATLAS_SIZE) {
        /* Next row */
        e->atlas_x  = TE_ATLAS_PADDING;
        e->atlas_y += e->atlas_row_h + TE_ATLAS_PADDING;
        e->atlas_row_h = 0;
    }
    if (e->atlas_y + h + TE_ATLAS_PADDING > TE_ATLAS_SIZE) {
        /* Atlas full - reset */
        te_atlas_reset(e);
    }

    if (h > e->atlas_row_h) e->atlas_row_h = h;

    /* Convert 1-byte gray to RGBA for the texture */
    uint8_t* rgba = malloc(w * h * 4);
    for (int i = 0; i < w * h; i++) {
        rgba[i*4+0] = bitmap[i]; /* R = coverage */
        rgba[i*4+1] = bitmap[i]; /* G */
        rgba[i*4+2] = bitmap[i]; /* B */
        rgba[i*4+3] = bitmap[i]; /* A */
    }

    glBindTexture(GL_TEXTURE_2D, e->atlas_tex);
    glTexSubImage2D(GL_TEXTURE_2D, 0,
                    e->atlas_x, e->atlas_y, w, h,
                    GL_RGBA, GL_UNSIGNED_BYTE, rgba);
    free(rgba);

    *u0 = (float)e->atlas_x            / TE_ATLAS_SIZE;
    *v0 = (float)e->atlas_y            / TE_ATLAS_SIZE;
    *u1 = (float)(e->atlas_x + w)      / TE_ATLAS_SIZE;
    *v1 = (float)(e->atlas_y + h)      / TE_ATLAS_SIZE;

    e->atlas_x += w + TE_ATLAS_PADDING;
    return true;
}

/* Upload a full-color BGRA bitmap (Emoji) into the atlas */
static bool te_atlas_upload_color(VaxpTextEngine* e,
                                   const uint8_t* bgra, int w, int h,
                                   float* u0, float* v0, float* u1, float* v1) {
    if (w <= 0 || h <= 0) return false;

    if (e->atlas_x + w + TE_ATLAS_PADDING > TE_ATLAS_SIZE) {
        e->atlas_x  = TE_ATLAS_PADDING;
        e->atlas_y += e->atlas_row_h + TE_ATLAS_PADDING;
        e->atlas_row_h = 0;
    }
    if (e->atlas_y + h + TE_ATLAS_PADDING > TE_ATLAS_SIZE) {
        te_atlas_reset(e);
    }
    if (h > e->atlas_row_h) e->atlas_row_h = h;

    /* Convert BGRA to RGBA */
    uint8_t* rgba = malloc(w * h * 4);
    for (int i = 0; i < w * h; i++) {
        rgba[i*4+0] = bgra[i*4+2]; /* R */
        rgba[i*4+1] = bgra[i*4+1]; /* G */
        rgba[i*4+2] = bgra[i*4+0]; /* B */
        rgba[i*4+3] = bgra[i*4+3]; /* A */
    }

    glBindTexture(GL_TEXTURE_2D, e->atlas_tex);
    glTexSubImage2D(GL_TEXTURE_2D, 0,
                    e->atlas_x, e->atlas_y, w, h,
                    GL_RGBA, GL_UNSIGNED_BYTE, rgba);
    free(rgba);

    *u0 = (float)e->atlas_x            / TE_ATLAS_SIZE;
    *v0 = (float)e->atlas_y            / TE_ATLAS_SIZE;
    *u1 = (float)(e->atlas_x + w)      / TE_ATLAS_SIZE;
    *v1 = (float)(e->atlas_y + h)      / TE_ATLAS_SIZE;

    e->atlas_x += w + TE_ATLAS_PADDING;
    return true;
}

/* ============================================================
 * GLYPH RASTERIZATION (FreeType)
 * ============================================================ */

static GlyphEntry* te_rasterize_glyph(VaxpTextEngine* e,
                                        VaxpFontId fid,
                                        uint32_t   codepoint) {
    if (fid < 0 || fid >= e->font_count || !e->fonts[fid].in_use) return NULL;

    FT_Face face = e->fonts[fid].ft_face;

    /* Try to load as color (Emoji) first */
    FT_Error err;
    bool color_glyph = false;

    if (face->num_fixed_sizes > 0) {
        /* Select best fixed size for color fonts */
        int best = 0;
        float want = e->fonts[fid].size_px;
        float best_diff = fabsf((float)face->available_sizes[0].height - want);
        for (int i = 1; i < face->num_fixed_sizes; i++) {
            float diff = fabsf((float)face->available_sizes[i].height - want);
            if (diff < best_diff) { best_diff = diff; best = i; }
        }
        FT_Select_Size(face, best);
        err = FT_Load_Glyph(face, FT_Get_Char_Index(face, codepoint),
                             FT_LOAD_COLOR);
        if (!err && face->glyph->format == FT_GLYPH_FORMAT_BITMAP) {
            color_glyph = (face->glyph->bitmap.pixel_mode == FT_PIXEL_MODE_BGRA);
        } else {
            /* Fall back to scaled rendering */
            FT_Set_Pixel_Sizes(face, 0, (FT_UInt)e->fonts[fid].size_px);
            err = FT_Load_Glyph(face, FT_Get_Char_Index(face, codepoint),
                                 FT_LOAD_RENDER | FT_LOAD_TARGET_LIGHT);
        }
    } else {
        FT_Set_Pixel_Sizes(face, 0, (FT_UInt)e->fonts[fid].size_px);
        uint32_t glyph_idx = FT_Get_Char_Index(face, codepoint);
        err = FT_Load_Glyph(face, glyph_idx, FT_LOAD_COLOR);
        if (!err) {
            if (face->glyph->format != FT_GLYPH_FORMAT_BITMAP) {
                err = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
            }
            color_glyph = (face->glyph->bitmap.pixel_mode == FT_PIXEL_MODE_BGRA);
        }
    }

    if (err) return NULL;

    FT_GlyphSlot slot = face->glyph;
    FT_Bitmap*   bm   = &slot->bitmap;

    GlyphEntry* entry = te_cache_insert(e, fid, codepoint);
    if (!entry) return NULL;

    entry->advance   = (float)(slot->advance.x >> 6);
    entry->bearing_x = slot->bitmap_left;
    entry->bearing_y = slot->bitmap_top;
    entry->bitmap_w  = (int)bm->width;
    entry->bitmap_h  = (int)bm->rows;
    entry->is_color  = color_glyph;

    if (bm->width == 0 || bm->rows == 0) {
        /* Whitespace or invisible glyph - valid, no atlas space needed */
        entry->u0 = entry->v0 = entry->u1 = entry->v1 = 0;
        return entry;
    }

    bool uploaded;
    if (color_glyph) {
        uploaded = te_atlas_upload_color(e, bm->buffer,
                                          (int)bm->width, (int)bm->rows,
                                          &entry->u0, &entry->v0,
                                          &entry->u1, &entry->v1);
    } else {
        /* Ensure gray bitmap */
        if (bm->pixel_mode == FT_PIXEL_MODE_GRAY) {
            uploaded = te_atlas_upload_gray(e, bm->buffer,
                                             (int)bm->width, (int)bm->rows,
                                             &entry->u0, &entry->v0,
                                             &entry->u1, &entry->v1);
        } else {
            /* Convert mono to gray */
            uint8_t* gray = calloc(bm->width * bm->rows, 1);
            for (int row = 0; row < (int)bm->rows; row++) {
                for (int col = 0; col < (int)bm->width; col++) {
                    uint8_t byte = bm->buffer[row * bm->pitch + col / 8];
                    gray[row * bm->width + col] = (byte >> (7 - (col % 8))) & 1 ? 255 : 0;
                }
            }
            uploaded = te_atlas_upload_gray(e, gray,
                                             (int)bm->width, (int)bm->rows,
                                             &entry->u0, &entry->v0,
                                             &entry->u1, &entry->v1);
            free(gray);
        }
    }
    if (!uploaded) return NULL;
    return entry;
}

static GlyphEntry* te_get_glyph(VaxpTextEngine* e, VaxpFontId fid, uint32_t cp) {
    GlyphEntry* g = te_cache_lookup(e, fid, cp);
    if (g) return g;
    return te_rasterize_glyph(e, fid, cp);
}

/* ============================================================
 * BATCH HELPERS
 * ============================================================ */

static void te_flush_internal(VaxpTextEngine* e);

static void te_push_quad(VaxpTextEngine* e,
                          float x0, float y0, float x1, float y1,
                          float u0, float v0, float u1, float v1,
                          VaxpTextColor color, float mode) {
    if (e->batch_count + 6 > TE_BATCH_MAX) te_flush_internal(e);

    float r = color.r / 255.0f;
    float g = color.g / 255.0f;
    float b = color.b / 255.0f;
    float a = color.a / 255.0f;

    TeVertex* v = e->batch + e->batch_count;
    /* Triangle 1 */
    v[0] = (TeVertex){x0, y0, u0, v0, r, g, b, a, mode};
    v[1] = (TeVertex){x1, y0, u1, v0, r, g, b, a, mode};
    v[2] = (TeVertex){x1, y1, u1, v1, r, g, b, a, mode};
    /* Triangle 2 */
    v[3] = (TeVertex){x0, y0, u0, v0, r, g, b, a, mode};
    v[4] = (TeVertex){x1, y1, u1, v1, r, g, b, a, mode};
    v[5] = (TeVertex){x0, y1, u0, v1, r, g, b, a, mode};
    e->batch_count += 6;
}

static void te_flush_internal(VaxpTextEngine* e) {
    if (e->batch_count == 0) return;

    te_update_projection(e);

    te_glUseProgram(e->shader);

    GLint proj_loc = te_glGetUniformLocation(e->shader, "uProj");
    te_glUniformMatrix4fv(proj_loc, 1, GL_FALSE, e->proj);

    GLint model_loc = te_glGetUniformLocation(e->shader, "uModel");
    te_glUniformMatrix4fv(model_loc, 1, GL_FALSE, e->model);

    GLint atlas_loc = te_glGetUniformLocation(e->shader, "uAtlas");
    te_glUniform1i(atlas_loc, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, e->atlas_tex);

    te_glBindVertexArray(e->vao);
    te_glBindBuffer(GL_ARRAY_BUFFER, e->vbo);
    te_glBufferSubData(GL_ARRAY_BUFFER, 0,
                       e->batch_count * sizeof(TeVertex),
                       e->batch);
    glDrawArrays(GL_TRIANGLES, 0, e->batch_count);

    e->batch_count = 0;
}

/* ============================================================
 * UTF-8 DECODING
 * ============================================================ */

/* Decode one UTF-8 codepoint from *p, advance *p */
static uint32_t te_utf8_next(const char** p) {
    const unsigned char* s = (const unsigned char*)*p;
    uint32_t cp;
    if (*s < 0x80) {
        cp = *s++;
    } else if (*s < 0xE0) {
        cp = (*s++ & 0x1F) << 6;
        cp |= (*s++ & 0x3F);
    } else if (*s < 0xF0) {
        cp = (*s++ & 0x0F) << 12;
        cp |= (*s++ & 0x3F) << 6;
        cp |= (*s++ & 0x3F);
    } else {
        cp = (*s++ & 0x07) << 18;
        cp |= (*s++ & 0x3F) << 12;
        cp |= (*s++ & 0x3F) << 6;
        cp |= (*s++ & 0x3F);
    }
    *p = (const char*)s;
    return cp;
}

/* Count UTF-8 codepoints in string */
static int te_utf8_count(const char* s) {
    int n = 0;
    while (*s) { if ((*s & 0xC0) != 0x80) n++; s++; }
    return n;
}

/* ============================================================
 * HARFBUZZ SHAPING PIPELINE
 * ============================================================ */

typedef struct {
    uint32_t codepoint;   /* Unicode codepoint */
    float    x_advance;   /* Shaped advance */
    float    x_offset;    /* Shaped X offset */
    float    y_offset;    /* Shaped Y offset */
} ShapedGlyph;

/* Shape a UTF-8 run using HarfBuzz, returns glyph count */
static int te_shape_run(VaxpTextEngine* e, VaxpFontId fid,
                         const char* utf8, int byte_len,
                         bool rtl,
                         ShapedGlyph* out, int out_max) {
    if (fid < 0 || fid >= e->font_count || !e->fonts[fid].in_use) return 0;

    hb_buffer_t* buf = hb_buffer_create();
    hb_buffer_set_direction(buf, rtl ? HB_DIRECTION_RTL : HB_DIRECTION_LTR);
    hb_buffer_set_script(buf, rtl ? HB_SCRIPT_ARABIC : HB_SCRIPT_COMMON);
    hb_buffer_set_language(buf, hb_language_from_string("und", -1));
    hb_buffer_add_utf8(buf, utf8, byte_len, 0, -1);
    hb_buffer_guess_segment_properties(buf);

    hb_shape(e->fonts[fid].hb_font, buf, NULL, 0);

    unsigned int count = 0;
    hb_glyph_info_t*     info = hb_buffer_get_glyph_infos(buf, &count);
    hb_glyph_position_t* pos  = hb_buffer_get_glyph_positions(buf, NULL);

    int n = (int)count < out_max ? (int)count : out_max;
    for (int i = 0; i < n; i++) {
        /* HarfBuzz returns glyph IDs after shaping; we need codepoints for caching.
         * We map back to codepoints via FT_Get_Char_Index reverse lookup.
         * For our atlas the key is the HarfBuzz glyph ID directly - use it as codepoint. */
        out[i].codepoint = info[i].codepoint; /* This is the HB glyph ID */
        out[i].x_advance = (float)(pos[i].x_advance) / 64.0f;
        out[i].x_offset  = (float)(pos[i].x_offset)  / 64.0f;
        out[i].y_offset  = (float)(pos[i].y_offset)   / 64.0f;
    }

    hb_buffer_destroy(buf);
    return n;
}

/* Rasterize by glyph ID (HarfBuzz-shaped) instead of codepoint */
static GlyphEntry* te_get_glyph_by_id(VaxpTextEngine* e,
                                        VaxpFontId fid,
                                        uint32_t glyph_id) {
    GlyphEntry* g = te_cache_lookup(e, fid, glyph_id);
    if (g) return g;

    if (fid < 0 || fid >= e->font_count || !e->fonts[fid].in_use) return NULL;
    FT_Face face = e->fonts[fid].ft_face;

    FT_Set_Pixel_Sizes(face, 0, (FT_UInt)e->fonts[fid].size_px);

    FT_Error err = FT_Load_Glyph(face, glyph_id, FT_LOAD_COLOR);
    if (err) return NULL;
    if (face->glyph->format != FT_GLYPH_FORMAT_BITMAP) {
        err = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
        if (err) return NULL;
    }

    FT_GlyphSlot slot = face->glyph;
    FT_Bitmap*   bm   = &slot->bitmap;
    bool color_glyph  = (bm->pixel_mode == FT_PIXEL_MODE_BGRA);

    GlyphEntry* entry = te_cache_insert(e, fid, glyph_id);
    if (!entry) return NULL;

    entry->advance   = (float)(slot->advance.x >> 6);
    entry->bearing_x = slot->bitmap_left;
    entry->bearing_y = slot->bitmap_top;
    entry->bitmap_w  = (int)bm->width;
    entry->bitmap_h  = (int)bm->rows;
    entry->is_color  = color_glyph;

    if (bm->width == 0 || bm->rows == 0) {
        entry->u0 = entry->v0 = entry->u1 = entry->v1 = 0;
        return entry;
    }

    if (color_glyph) {
        te_atlas_upload_color(e, bm->buffer, (int)bm->width, (int)bm->rows,
                               &entry->u0, &entry->v0, &entry->u1, &entry->v1);
    } else {
        te_atlas_upload_gray(e, bm->buffer, (int)bm->width, (int)bm->rows,
                              &entry->u0, &entry->v0, &entry->u1, &entry->v1);
    }
    return entry;
}

/* ============================================================
 * FRIBIDI BIDI ANALYSIS
 * ============================================================ */

typedef struct {
    int   start;  /* byte offset in source string */
    int   end;
    bool  rtl;
} BidiRun;

/* Perform BiDi analysis on UTF-8 string, output runs.
   Returns number of runs. */
static int te_bidi_runs(const char* utf8,
                         BidiRun* runs, int max_runs,
                         uint32_t* cps_out, int* cp_count_out) {
    int len = te_utf8_count(utf8);
    if (len <= 0) { *cp_count_out = 0; return 0; }

    FriBidiChar* logical = malloc(len * sizeof(FriBidiChar));
    FriBidiCharType* types = malloc(len * sizeof(FriBidiCharType));
    FriBidiLevel*  levels = malloc(len * sizeof(FriBidiLevel));

    /* Decode UTF-8 to codepoints */
    const char* p = utf8;
    for (int i = 0; i < len; i++) {
        logical[i] = (FriBidiChar)te_utf8_next(&p);
        if (cps_out) cps_out[i] = (uint32_t)logical[i];
    }
    *cp_count_out = len;

    FriBidiParType base = FRIBIDI_PAR_ON;
    fribidi_get_bidi_types(logical, len, types);
    fribidi_get_par_embedding_levels(types, len, &base, levels);

    int run_count = 0;
    int i = 0;
    while (i < len && run_count < max_runs) {
        bool rtl = (levels[i] % 2) == 1;
        int j = i + 1;
        while (j < len && ((levels[j] % 2) == 1) == rtl) j++;
        runs[run_count].start = i;
        runs[run_count].end   = j;
        runs[run_count].rtl   = rtl;
        run_count++;
        i = j;
    }

    free(logical);
    free(types);
    free(levels);
    return run_count;
}

/* ============================================================
 * FONT SEARCH (no Fontconfig)
 * ============================================================ */

/* Common monospace font paths on Linux */
static const char* MONO_FONT_PATHS[] = {
    "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
    "/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf",
    "/usr/share/fonts/truetype/ubuntu/UbuntuMono-R.ttf",
    "/usr/share/fonts/opentype/noto/NotoSansMono-Regular.ttf",
    "/usr/share/fonts/truetype/noto/NotoSansMono-Regular.ttf",
    "/usr/share/fonts/truetype/freefont/FreeMono.ttf",
    NULL
};

static const char* MONO_BOLD_PATHS[] = {
    "/usr/share/fonts/truetype/dejavu/DejaVuSansMono-Bold.ttf",
    "/usr/share/fonts/truetype/liberation/LiberationMono-Bold.ttf",
    "/usr/share/fonts/truetype/ubuntu/UbuntuMono-B.ttf",
    NULL
};

/* Arabic / RTL fallback fonts */
static const char* ARABIC_FONT_PATHS[] = {
    "/usr/share/fonts/truetype/noto/NotoNaskhArabic-Regular.ttf",
    "/usr/share/fonts/opentype/noto/NotoNaskhArabic-Regular.ttf",
    "/usr/share/fonts/truetype/arabeyes/ae_AlArabiya.ttf",
    "/usr/share/fonts/truetype/kacst/KacstNaskh.ttf",
    NULL
};

/* Emoji fallback fonts */
static const char* EMOJI_FONT_PATHS[] = {
    "/usr/share/fonts/truetype/noto/NotoColorEmoji.ttf",
    "/usr/share/fonts/opentype/noto/NotoColorEmoji.ttf",
    "/usr/share/fonts/google-noto-emoji/NotoColorEmoji.ttf",
    NULL
};

static const char* te_find_font(const char* family, bool bold, bool italic) {
    (void)italic;
    /* Simple name matching */
    if (family && (strcasestr(family, "arabic") || strcasestr(family, "naskh"))) {
        for (int i = 0; ARABIC_FONT_PATHS[i]; i++) {
            FILE* f = fopen(ARABIC_FONT_PATHS[i], "rb");
            if (f) { fclose(f); return ARABIC_FONT_PATHS[i]; }
        }
    }
    if (family && strcasestr(family, "emoji")) {
        for (int i = 0; EMOJI_FONT_PATHS[i]; i++) {
            FILE* f = fopen(EMOJI_FONT_PATHS[i], "rb");
            if (f) { fclose(f); return EMOJI_FONT_PATHS[i]; }
        }
    }
    if (bold) {
        for (int i = 0; MONO_BOLD_PATHS[i]; i++) {
            FILE* f = fopen(MONO_BOLD_PATHS[i], "rb");
            if (f) { fclose(f); return MONO_BOLD_PATHS[i]; }
        }
    }
    for (int i = 0; MONO_FONT_PATHS[i]; i++) {
        FILE* f = fopen(MONO_FONT_PATHS[i], "rb");
        if (f) { fclose(f); return MONO_FONT_PATHS[i]; }
    }
    return NULL;
}

/* ============================================================
 * PUBLIC API IMPLEMENTATION
 * ============================================================ */

VaxpTextEngine* vaxp_text_engine_create(void) {
    if (!te_load_gl_functions()) return NULL;

    VaxpTextEngine* e = calloc(1, sizeof(VaxpTextEngine));
    if (!e) return NULL;

    /* Init FreeType */
    if (FT_Init_FreeType(&e->ft_lib)) {
        fprintf(stderr, "[TE] FreeType init failed\n");
        free(e);
        return NULL;
    }

    /* Create GL Atlas texture - RGBA, nearest filtering for crisp glyphs */
    glGenTextures(1, &e->atlas_tex);
    glBindTexture(GL_TEXTURE_2D, e->atlas_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 TE_ATLAS_SIZE, TE_ATLAS_SIZE, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    /* Clear atlas texture */
    uint8_t* zeros = calloc(TE_ATLAS_SIZE * TE_ATLAS_SIZE * 4, 1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, TE_ATLAS_SIZE, TE_ATLAS_SIZE,
                    GL_RGBA, GL_UNSIGNED_BYTE, zeros);
    free(zeros);

    e->atlas_x     = TE_ATLAS_PADDING;
    e->atlas_y     = TE_ATLAS_PADDING;
    e->atlas_row_h = 0;

    /* Init glyph pool */
    e->glyph_pool_cap = 1024;
    e->glyph_pool     = calloc(e->glyph_pool_cap, sizeof(GlyphEntry));

    /* Create GL objects */
    e->shader = te_link_program(TE_VERT_SRC, TE_FRAG_SRC);

    te_glGenVertexArrays(1, &e->vao);
    te_glGenBuffers(1, &e->vbo);
    te_glBindVertexArray(e->vao);
    te_glBindBuffer(GL_ARRAY_BUFFER, e->vbo);
    te_glBufferData(GL_ARRAY_BUFFER, TE_BATCH_MAX * sizeof(TeVertex), NULL, GL_DYNAMIC_DRAW);

    te_glEnableVertexAttribArray(0);
    te_glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(TeVertex),
                              (void*)offsetof(TeVertex, x));
    te_glEnableVertexAttribArray(1);
    te_glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(TeVertex),
                              (void*)offsetof(TeVertex, u));
    te_glEnableVertexAttribArray(2);
    te_glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(TeVertex),
                              (void*)offsetof(TeVertex, r));
    te_glEnableVertexAttribArray(3);
    te_glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(TeVertex),
                              (void*)offsetof(TeVertex, mode));

    e->batch = calloc(TE_BATCH_MAX, sizeof(TeVertex));
    
    /* Identity model matrix */
    memset(e->model, 0, sizeof(e->model));
    e->model[0] = 1.0f; e->model[5] = 1.0f; e->model[10] = 1.0f; e->model[15] = 1.0f;

    fprintf(stderr, "[TE] Text engine created (FreeType + HarfBuzz + FriBidi)\n");
    return e;
}

void vaxp_text_engine_set_transform(VaxpTextEngine* engine, const float* matrix) {
    if (!engine || !matrix) return;
    
    /* Check if it changed */
    bool changed = false;
    for (int i = 0; i < 16; i++) {
        if (engine->model[i] != matrix[i]) {
            changed = true;
            break;
        }
    }
    
    if (changed) {
        te_flush_internal(engine);
        memcpy(engine->model, matrix, 16 * sizeof(float));
    }
}

void vaxp_text_engine_destroy(VaxpTextEngine* engine) {
    if (!engine) return;

    /* Free font faces */
    for (int i = 0; i < engine->font_count; i++) {
        if (engine->fonts[i].in_use) {
            hb_font_destroy(engine->fonts[i].hb_font);
            FT_Done_Face(engine->fonts[i].ft_face);
        }
    }

    FT_Done_FreeType(engine->ft_lib);

    /* Free GL resources */
    if (engine->atlas_tex) glDeleteTextures(1, &engine->atlas_tex);
    if (engine->vao) te_glDeleteVertexArrays(1, &engine->vao);
    if (engine->vbo) te_glDeleteBuffers(1, &engine->vbo);
    if (engine->shader) te_glDeleteProgram(engine->shader);

    free(engine->glyph_pool);
    free(engine->batch);
    free(engine);
}

VaxpFontId vaxp_text_engine_load_font(VaxpTextEngine* engine,
                                       const char* path,
                                       float size_px) {
    if (!engine || !path || size_px <= 0) return VAXP_FONT_INVALID;
    if (engine->font_count >= TE_MAX_FONTS) return VAXP_FONT_INVALID;

    /* Check if already loaded at same size */
    for (int i = 0; i < engine->font_count; i++) {
        if (engine->fonts[i].in_use &&
            fabsf(engine->fonts[i].size_px - size_px) < 0.5f &&
            strcmp(engine->fonts[i].path, path) == 0) {
            return i;
        }
    }

    int slot = engine->font_count++;
    FontRecord* rec = &engine->fonts[slot];

    if (FT_New_Face(engine->ft_lib, path, 0, &rec->ft_face)) {
        fprintf(stderr, "[TE] Failed to load font: %s\n", path);
        engine->font_count--;
        return VAXP_FONT_INVALID;
    }

    FT_Set_Pixel_Sizes(rec->ft_face, 0, (FT_UInt)size_px);
    rec->hb_font = hb_ft_font_create_referenced(rec->ft_face);
    rec->size_px = size_px;
    rec->in_use  = true;
    strncpy(rec->path, path, sizeof(rec->path) - 1);

    fprintf(stderr, "[TE] Loaded font: %s @ %.0fpx (id=%d)\n", path, size_px, slot);
    return slot;
}

VaxpFontId vaxp_text_engine_load_font_by_name(VaxpTextEngine* engine,
                                               const char* family,
                                               float size_px,
                                               bool bold,
                                               bool italic) {
    const char* path = te_find_font(family, bold, italic);
    if (!path) {
        fprintf(stderr, "[TE] Could not find font for family: %s\n", family ? family : "(null)");
        return VAXP_FONT_INVALID;
    }
    return vaxp_text_engine_load_font(engine, path, size_px);
}

void vaxp_text_engine_draw_string(VaxpTextEngine* engine,
                                   VaxpFontId font_id,
                                   const char* text,
                                   float x, float y,
                                   VaxpTextColor color) {
    if (!engine || !text || !*text) return;
    if (font_id < 0 || font_id >= engine->font_count) return;

    float size_px = engine->fonts[font_id].size_px;

    /* BiDi analysis */
    BidiRun runs[64];
    uint32_t cps[TE_MAX_TEXT_LEN];
    int cp_count = 0;
    int run_count = te_bidi_runs(text, runs, 64, cps, &cp_count);

    ShapedGlyph shaped[TE_MAX_TEXT_LEN];
    float pen_x = x;
    float baseline_y = y + size_px * 0.8f; /* Approximate baseline */

    for (int r = 0; r < run_count; r++) {
        /* Extract the substring for this run */
        /* (For now use entire text per run - full BiDi substring needs byte mapping) */
        int n = te_shape_run(engine, font_id, text, (int)strlen(text),
                              runs[r].rtl, shaped, TE_MAX_TEXT_LEN);

        /* Render RTL runs right-to-left */
        int start = runs[r].rtl ? n - 1 : 0;
        int end   = runs[r].rtl ? -1 : n;
        int step  = runs[r].rtl ? -1 : 1;

        for (int i = start; i != end; i += step) {
            GlyphEntry* g = te_get_glyph_by_id(engine, font_id, shaped[i].codepoint);
            if (!g) { pen_x += shaped[i].x_advance; continue; }

            if (g->bitmap_w > 0 && g->bitmap_h > 0) {
                float gx = pen_x + shaped[i].x_offset + g->bearing_x;
                float gy = baseline_y - g->bearing_y - shaped[i].y_offset;
                float gw = (float)g->bitmap_w;
                float gh = (float)g->bitmap_h;
                float mode = g->is_color ? 2.0f : 1.0f;
                te_push_quad(engine, gx, gy, gx+gw, gy+gh,
                              g->u0, g->v0, g->u1, g->v1, color, mode);
            }
            pen_x += shaped[i].x_advance;
        }
    }
}

void vaxp_text_engine_draw_cells(VaxpTextEngine* engine,
                                  VaxpFontId regular_font,
                                  VaxpFontId bold_font,
                                  const VaxpTextCell* cells,
                                  int count,
                                  float start_x, float y,
                                  float cell_w, float cell_h) {
    if (!engine || !cells || count <= 0) return;

    float size_px = (regular_font >= 0 && regular_font < engine->font_count)
                     ? engine->fonts[regular_font].size_px : 14.0f;
    float baseline_off = size_px * 0.8f; /* y offset to baseline within cell */

    for (int i = 0; i < count; i++) {
        const VaxpTextCell* cell = &cells[i];
        if (!cell->ch[0] || cell->ch[0] == ' ') continue;

        bool is_bold = (cell->flags & 1) != 0;
        VaxpFontId fid = (is_bold && bold_font >= 0) ? bold_font : regular_font;
        if (fid < 0) continue;

        /* Decode UTF-8 codepoint */
        const char* p = cell->ch;
        uint32_t cp = te_utf8_next(&p);
        if (!cp) continue;

        /* Shape single character through HarfBuzz for correct ligature/Arabic handling */
        ShapedGlyph shaped[8];
        int n = te_shape_run(engine, fid, cell->ch, (int)strlen(cell->ch),
                              false, shaped, 8);

        float cx = start_x + i * cell_w;
        float baseline = y + baseline_off;

        float pen = cx;
        for (int s = 0; s < n; s++) {
            GlyphEntry* g = te_get_glyph_by_id(engine, fid, shaped[s].codepoint);
            if (!g) { pen += shaped[s].x_advance; continue; }

            if (g->bitmap_w > 0 && g->bitmap_h > 0) {
                float gx = pen + shaped[s].x_offset + g->bearing_x;
                float gy = baseline - g->bearing_y - shaped[s].y_offset;

                /* Scale color emoji to fit cell height */
                float gw = (float)g->bitmap_w;
                float gh = (float)g->bitmap_h;
                if (g->is_color && gh > cell_h) {
                    float scale = cell_h / gh;
                    gw *= scale;
                    gh = cell_h;
                    gy = y;
                }

                VaxpTextColor fg = cell->fg;
                float mode = g->is_color ? 2.0f : 1.0f;
                te_push_quad(engine, gx, gy, gx+gw, gy+gh,
                              g->u0, g->v0, g->u1, g->v1, fg, mode);

                /* Underline */
                if (cell->flags & 4) {
                    VaxpTextColor uc = fg;
                    float uy = y + cell_h - 2;
                    te_push_quad(engine, cx, uy, cx+cell_w, uy+1.5f,
                                  0,0,0,0, uc, 0.0f);
                }
            }
            pen += shaped[s].x_advance;
        }
    }
}

float vaxp_text_engine_measure_width(VaxpTextEngine* engine,
                                      VaxpFontId font_id,
                                      const char* text) {
    if (!engine || !text || !*text) return 0;
    if (font_id < 0 || font_id >= engine->font_count) return 0;

    ShapedGlyph shaped[TE_MAX_TEXT_LEN];
    int n = te_shape_run(engine, font_id, text, (int)strlen(text),
                          false, shaped, TE_MAX_TEXT_LEN);
    float total = 0;
    for (int i = 0; i < n; i++) total += shaped[i].x_advance;
    return total;
}

void vaxp_text_engine_flush(VaxpTextEngine* engine) {
    if (engine) te_flush_internal(engine);
}

GLuint vaxp_text_engine_atlas_texture(VaxpTextEngine* engine) {
    return engine ? engine->atlas_tex : 0;
}
