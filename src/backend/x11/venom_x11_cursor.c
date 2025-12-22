/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_x11_cursor.c - X11 Cursor implementation
 * 
 * Uses X11 standard cursors from cursorfont.h with caching for performance.
 */

#include "venom/backend/venom_cursor.h"
#include "venom/core/venom_memory.h"

#include <X11/Xlib.h>
#include <X11/cursorfont.h>
#include <string.h>

/* ============================================================================
 * CURSOR STATE
 * ============================================================================ */

#define MAX_CURSOR_WINDOWS 64

static struct {
    Display* display;
    VenomBool initialized;
    
    /* Cached cursors */
    Cursor cursors[VENOM_CURSOR_COUNT];
    VenomBool cursor_created[VENOM_CURSOR_COUNT];
    
    /* Hidden cursor */
    Cursor blank_cursor;
    VenomBool blank_cursor_created;
    
    /* Registered windows */
    struct {
        VenomU32 id;
        Window xwindow;
        VenomCursorType current_cursor;
        VenomBool hidden;
    } windows[MAX_CURSOR_WINDOWS];
    VenomU32 window_count;
    
} g_cursor = {0};

/* ============================================================================
 * X11 CURSOR FONT MAPPING
 * ============================================================================ */

/**
 * @brief Map VenomCursorType to X11 cursor font glyph
 */
static unsigned int cursor_type_to_glyph(VenomCursorType type) {
    switch (type) {
        case VENOM_CURSOR_DEFAULT:
        case VENOM_CURSOR_ARROW:
            return XC_left_ptr;
            
        case VENOM_CURSOR_HAND:
            return XC_hand2;
            
        case VENOM_CURSOR_TEXT:
            return XC_xterm;
            
        case VENOM_CURSOR_WAIT:
            return XC_watch;
            
        case VENOM_CURSOR_HELP:
            return XC_question_arrow;
            
        case VENOM_CURSOR_CROSSHAIR:
            return XC_crosshair;
            
        case VENOM_CURSOR_MOVE:
            return XC_fleur;
            
        case VENOM_CURSOR_NOT_ALLOWED:
            return XC_X_cursor;
            
        case VENOM_CURSOR_RESIZE_N:
            return XC_top_side;
            
        case VENOM_CURSOR_RESIZE_S:
            return XC_bottom_side;
            
        case VENOM_CURSOR_RESIZE_E:
            return XC_right_side;
            
        case VENOM_CURSOR_RESIZE_W:
            return XC_left_side;
            
        case VENOM_CURSOR_RESIZE_NE:
            return XC_top_right_corner;
            
        case VENOM_CURSOR_RESIZE_NW:
            return XC_top_left_corner;
            
        case VENOM_CURSOR_RESIZE_SE:
            return XC_bottom_right_corner;
            
        case VENOM_CURSOR_RESIZE_SW:
            return XC_bottom_left_corner;
            
        case VENOM_CURSOR_RESIZE_EW:
            return XC_sb_h_double_arrow;
            
        case VENOM_CURSOR_RESIZE_NS:
            return XC_sb_v_double_arrow;
            
        case VENOM_CURSOR_RESIZE_NESW:
            return XC_sizing;  /* Fallback */
            
        case VENOM_CURSOR_RESIZE_NWSE:
            return XC_sizing;  /* Fallback */
            
        case VENOM_CURSOR_GRAB:
            return XC_hand1;
            
        case VENOM_CURSOR_GRABBING:
            return XC_hand2;
            
        case VENOM_CURSOR_DND_COPY:
            return XC_plus;
            
        case VENOM_CURSOR_DND_MOVE:
            return XC_fleur;
            
        case VENOM_CURSOR_DND_LINK:
            return XC_target;
            
        case VENOM_CURSOR_DND_NO_DROP:
            return XC_pirate;
            
        default:
            return XC_left_ptr;
    }
}

/* ============================================================================
 * HELPER FUNCTIONS
 * ============================================================================ */

static Window find_xwindow(VenomU32 id) {
    for (VenomU32 i = 0; i < g_cursor.window_count; i++) {
        if (g_cursor.windows[i].id == id) {
            return g_cursor.windows[i].xwindow;
        }
    }
    return None;
}

static int find_window_index(VenomU32 id) {
    for (VenomU32 i = 0; i < g_cursor.window_count; i++) {
        if (g_cursor.windows[i].id == id) {
            return (int)i;
        }
    }
    return -1;
}

static Cursor get_or_create_cursor(VenomCursorType type) {
    if (type >= VENOM_CURSOR_COUNT) {
        type = VENOM_CURSOR_DEFAULT;
    }
    
    if (type == VENOM_CURSOR_NONE) {
        /* Return blank cursor */
        if (!g_cursor.blank_cursor_created) {
            /* Create a blank cursor using a 1x1 transparent pixmap */
            Pixmap pixmap = XCreatePixmap(g_cursor.display, 
                                           DefaultRootWindow(g_cursor.display),
                                           1, 1, 1);
            XColor color = {0};
            g_cursor.blank_cursor = XCreatePixmapCursor(g_cursor.display,
                                                         pixmap, pixmap,
                                                         &color, &color, 0, 0);
            XFreePixmap(g_cursor.display, pixmap);
            g_cursor.blank_cursor_created = VENOM_TRUE;
        }
        return g_cursor.blank_cursor;
    }
    
    if (!g_cursor.cursor_created[type]) {
        unsigned int glyph = cursor_type_to_glyph(type);
        g_cursor.cursors[type] = XCreateFontCursor(g_cursor.display, glyph);
        g_cursor.cursor_created[type] = VENOM_TRUE;
    }
    
    return g_cursor.cursors[type];
}

/* ============================================================================
 * PUBLIC API
 * ============================================================================ */

VenomResult venom_cursor_init(void* display_handle) {
    if (g_cursor.initialized) {
        return VENOM_OK_UNIT();
    }
    
    Display* display = (Display*)display_handle;
    if (!display) {
        return VENOM_ERR_UNIT(VENOM_ERROR_INVALID_ARGUMENT);
    }
    
    g_cursor.display = display;
    
    /* Initialize arrays */
    memset(g_cursor.cursors, 0, sizeof(g_cursor.cursors));
    memset(g_cursor.cursor_created, 0, sizeof(g_cursor.cursor_created));
    g_cursor.window_count = 0;
    g_cursor.blank_cursor_created = VENOM_FALSE;
    
    g_cursor.initialized = VENOM_TRUE;
    
    return VENOM_OK_UNIT();
}

void venom_cursor_shutdown(void) {
    if (!g_cursor.initialized) return;
    
    /* Free all created cursors */
    for (int i = 0; i < VENOM_CURSOR_COUNT; i++) {
        if (g_cursor.cursor_created[i]) {
            XFreeCursor(g_cursor.display, g_cursor.cursors[i]);
            g_cursor.cursor_created[i] = VENOM_FALSE;
        }
    }
    
    if (g_cursor.blank_cursor_created) {
        XFreeCursor(g_cursor.display, g_cursor.blank_cursor);
        g_cursor.blank_cursor_created = VENOM_FALSE;
    }
    
    g_cursor.window_count = 0;
    g_cursor.initialized = VENOM_FALSE;
}

VenomResult venom_cursor_register_window(VenomU32 window_id, void* native_window) {
    if (!g_cursor.initialized) {
        return VENOM_ERR_UNIT(VENOM_ERROR_NOT_INITIALIZED);
    }
    
    if (g_cursor.window_count >= MAX_CURSOR_WINDOWS) {
        return VENOM_ERR_UNIT(VENOM_ERROR_OUT_OF_MEMORY);
    }
    
    Window xwindow = (Window)(uintptr_t)native_window;
    
    /* Check if already registered */
    if (find_window_index(window_id) >= 0) {
        return VENOM_OK_UNIT();
    }
    
    g_cursor.windows[g_cursor.window_count].id = window_id;
    g_cursor.windows[g_cursor.window_count].xwindow = xwindow;
    g_cursor.windows[g_cursor.window_count].current_cursor = VENOM_CURSOR_DEFAULT;
    g_cursor.windows[g_cursor.window_count].hidden = VENOM_FALSE;
    g_cursor.window_count++;
    
    return VENOM_OK_UNIT();
}

void venom_cursor_unregister_window(VenomU32 window_id) {
    int index = find_window_index(window_id);
    if (index < 0) return;
    
    /* Shift remaining windows */
    for (VenomU32 i = (VenomU32)index; i < g_cursor.window_count - 1; i++) {
        g_cursor.windows[i] = g_cursor.windows[i + 1];
    }
    g_cursor.window_count--;
}

VenomResult venom_cursor_set(VenomU32 window_id, VenomCursorType cursor) {
    if (!g_cursor.initialized) {
        return VENOM_ERR_UNIT(VENOM_ERROR_NOT_INITIALIZED);
    }
    
    int index = find_window_index(window_id);
    if (index < 0) {
        return VENOM_ERR_UNIT(VENOM_ERROR_NOT_FOUND);
    }
    
    Window xwindow = g_cursor.windows[index].xwindow;
    
    if (cursor == VENOM_CURSOR_NONE) {
        g_cursor.windows[index].hidden = VENOM_TRUE;
    } else {
        g_cursor.windows[index].hidden = VENOM_FALSE;
    }
    
    Cursor xcursor = get_or_create_cursor(cursor);
    XDefineCursor(g_cursor.display, xwindow, xcursor);
    XFlush(g_cursor.display);
    
    g_cursor.windows[index].current_cursor = cursor;
    
    return VENOM_OK_UNIT();
}

void venom_cursor_reset(VenomU32 window_id) {
    if (!g_cursor.initialized) return;
    
    int index = find_window_index(window_id);
    if (index < 0) return;
    
    Window xwindow = g_cursor.windows[index].xwindow;
    XUndefineCursor(g_cursor.display, xwindow);
    XFlush(g_cursor.display);
    
    g_cursor.windows[index].current_cursor = VENOM_CURSOR_DEFAULT;
    g_cursor.windows[index].hidden = VENOM_FALSE;
}

VenomCursorType venom_cursor_get(VenomU32 window_id) {
    if (!g_cursor.initialized) return VENOM_CURSOR_DEFAULT;
    
    int index = find_window_index(window_id);
    if (index < 0) return VENOM_CURSOR_DEFAULT;
    
    return g_cursor.windows[index].current_cursor;
}

void venom_cursor_hide(VenomU32 window_id) {
    venom_cursor_set(window_id, VENOM_CURSOR_NONE);
}

void venom_cursor_show(VenomU32 window_id) {
    if (!g_cursor.initialized) return;
    
    int index = find_window_index(window_id);
    if (index < 0) return;
    
    if (g_cursor.windows[index].hidden) {
        /* Restore previous cursor or default */
        VenomCursorType prev = g_cursor.windows[index].current_cursor;
        if (prev == VENOM_CURSOR_NONE) {
            prev = VENOM_CURSOR_DEFAULT;
        }
        venom_cursor_set(window_id, prev);
    }
}
