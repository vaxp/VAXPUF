/*
 * VAXPUI Text Engine
 *
 * Hardware-accelerated text rendering using:
 *   - FreeType 2  : Font loading & glyph rasterization
 *   - HarfBuzz    : Text shaping (ligatures, kerning, Arabic, Emoji)
 *   - FriBidi     : Unicode BiDi algorithm (RTL/LTR mixing)
 *   - GPU Atlas   : 2048x2048 VRAM texture for zero-copy glyph access
 *
 * Zero dependency on Pango or Cairo. No memory leaks.
 */

#ifndef VAXP_TEXT_ENGINE_H
#define VAXP_TEXT_ENGINE_H

#include <stdint.h>
#include <stdbool.h>

#include <GL/gl.h>

/* ============================================================
 * FORWARD DECLARATIONS
 * ============================================================ */

typedef struct VaxpTextEngine VaxpTextEngine;

/* Color: RGBA u8 */
typedef struct { uint8_t r, g, b, a; } VaxpTextColor;

/* A single terminal cell to render */
typedef struct {
    char     ch[5];    /* UTF-8 encoded character (max 4 bytes + NUL) */
    VaxpTextColor fg;
    uint8_t  flags;    /* bit0=Bold, bit1=Italic, bit2=Underline */
} VaxpTextCell;

/* Loaded font face handle */
typedef int VaxpFontId;
#define VAXP_FONT_INVALID (-1)

/* ============================================================
 * ENGINE LIFECYCLE
 * ============================================================ */

/**
 * Create a text engine instance.
 * Must be called after OpenGL context is current.
 * Returns NULL on failure.
 */
VaxpTextEngine* vaxp_text_engine_create(void);

/**
 * Destroy the text engine and free all GPU + CPU resources.
 * No leaks. Safe to call with NULL.
 */
void vaxp_text_engine_destroy(VaxpTextEngine* engine);

/* ============================================================
 * FONT MANAGEMENT
 * ============================================================ */

/**
 * Load a font from a filesystem path at a given pixel size.
 * Returns a VaxpFontId >= 0 on success, VAXP_FONT_INVALID on failure.
 * Multiple sizes of the same font are treated as separate fonts.
 */
VaxpFontId vaxp_text_engine_load_font(VaxpTextEngine* engine,
                                       const char* path,
                                       float size_px);

/**
 * Load a font by family name using system font search (via fontconfig-free
 * FcPattern substitute). Falls back to a built-in path list on failure.
 * Returns a VaxpFontId >= 0 on success, VAXP_FONT_INVALID on failure.
 */
VaxpFontId vaxp_text_engine_load_font_by_name(VaxpTextEngine* engine,
                                               const char* family,
                                               float size_px,
                                               bool bold,
                                               bool italic);

/* ============================================================
 * DRAWING API
 * ============================================================ */

/**
 * Draw a UTF-8 string with full HarfBuzz shaping + FriBidi BiDi.
 * Supports: Arabic, Hebrew, mixed RTL/LTR, Ligatures, Emoji.
 *
 * @param engine      The text engine
 * @param font_id     Font to use
 * @param text        UTF-8 string
 * @param x, y       Screen position (y = baseline)
 * @param color      Foreground color
 */
void vaxp_text_engine_draw_string(VaxpTextEngine* engine,
                                   VaxpFontId font_id,
                                   const char* text,
                                   float x, float y,
                                   VaxpTextColor color);

/**
 * Draw terminal cells in a single optimized batch.
 * This is the hot path for the terminal emulator.
 *
 * @param engine        The text engine
 * @param regular_font  Font ID for regular text
 * @param bold_font     Font ID for bold text
 * @param cells         Array of cells to render
 * @param count         Number of cells
 * @param start_x       X origin for cell 0
 * @param y             Y origin (top of cell row)
 * @param cell_w        Cell width in pixels
 * @param cell_h        Cell height in pixels
 */
void vaxp_text_engine_draw_cells(VaxpTextEngine* engine,
                                  VaxpFontId regular_font,
                                  VaxpFontId bold_font,
                                  const VaxpTextCell* cells,
                                  int count,
                                  float start_x, float y,
                                  float cell_w, float cell_h);

/**
 * Measure the pixel width of a UTF-8 string without rendering.
 */
float vaxp_text_engine_measure_width(VaxpTextEngine* engine,
                                      VaxpFontId font_id,
                                      const char* text);

/**
 * Flush any pending draw calls to GPU.
 * Must be called at end of frame.
 */
void vaxp_text_engine_flush(VaxpTextEngine* engine);

/**
 * Set the current model transform matrix for text drawing.
 * Flushes the current batch if the matrix changes.
 */
void vaxp_text_engine_set_transform(VaxpTextEngine* engine, const float* matrix);

/* ============================================================
 * ATLAS DEBUGGING
 * ============================================================ */

/** Returns the OpenGL texture ID of the glyph atlas (for debugging). */
GLuint vaxp_text_engine_atlas_texture(VaxpTextEngine* engine);

#endif /* VAXP_TEXT_ENGINE_H */
