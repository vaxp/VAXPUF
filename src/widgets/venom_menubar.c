/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_menubar.c - Menu Bar Widget implementation
 */

#include "venom/widgets/venom_menubar.h"
#include "venom/core/venom_memory.h"
#include "venom/graphics/venom_canvas.h"
#include <string.h>
#include <ctype.h>

/* ============================================================================
 * FORWARD DECLARATIONS
 * ============================================================================ */

static void menubar_init(VenomWidget* widget);
static void menubar_destroy(VenomWidget* widget);
static void menubar_measure(VenomWidget* widget, VenomF32 available_width, VenomF32 available_height,
                             VenomF32* out_width, VenomF32* out_height);
static void menubar_layout(VenomWidget* widget, VenomRectF bounds);
static void menubar_draw(VenomWidget* widget, VenomCanvas* canvas);
static VenomBool menubar_on_event(VenomWidget* widget, const VenomEvent* event);

/* ============================================================================
 * WIDGET CLASS
 * ============================================================================ */

const VenomWidgetClass venom_menubar_class = {
    .class_name = "VenomMenuBar",
    .instance_size = sizeof(VenomMenuBar),
    .parent_class = &venom_widget_class,
    .init = menubar_init,
    .destroy = menubar_destroy,
    .measure = menubar_measure,
    .layout = menubar_layout,
    .draw = menubar_draw,
    .on_event = menubar_on_event,
    .on_state_changed = NULL,
};

/* ============================================================================
 * HELPER FUNCTIONS
 * ============================================================================ */

static char* str_dup(const char* s) {
    if (!s) return NULL;
    size_t len = strlen(s);
    char* copy = venom_alloc(len + 1);
    if (copy) memcpy(copy, s, len + 1);
    return copy;
}

static void str_free(char* s) {
    if (s) venom_free(s, strlen(s) + 1);
}

static void parse_mnemonic(const char* label, char** out_text, char* out_mnemonic) {
    if (!label) {
        *out_text = NULL;
        *out_mnemonic = '\0';
        return;
    }
    
    size_t len = strlen(label);
    *out_text = venom_alloc(len + 1);
    if (!*out_text) return;
    
    size_t j = 0;
    *out_mnemonic = '\0';
    
    for (size_t i = 0; i < len; i++) {
        if (label[i] == '_' && i + 1 < len && label[i + 1] != '_') {
            *out_mnemonic = (char)toupper((unsigned char)label[i + 1]);
        } else if (label[i] == '_' && i + 1 < len && label[i + 1] == '_') {
            (*out_text)[j++] = '_';
            i++;
        } else {
            (*out_text)[j++] = label[i];
        }
    }
    (*out_text)[j] = '\0';
}

static VenomF32 calculate_text_width(const char* text, VenomF32 font_size) {
    if (!text) return 0;
    return (VenomF32)strlen(text) * font_size * 0.6f;
}

/* ============================================================================
 * LIFECYCLE
 * ============================================================================ */

static void menubar_init(VenomWidget* widget) {
    VenomMenuBar* bar = (VenomMenuBar*)widget;
    
    /* Initialize state */
    bar->items = NULL;
    bar->item_count = 0;
    bar->item_capacity = 0;
    bar->hover_index = -1;
    bar->active_index = -1;
    bar->menu_open = VENOM_FALSE;
    
    /* Default appearance */
    bar->background = venom_color_rgb(245, 245, 245);
    bar->hover_background = venom_color_rgba(0, 0, 0, 20);
    bar->active_background = venom_color_rgba(0, 0, 0, 40);
    bar->text_color = venom_color_rgb(30, 30, 30);
    bar->text_color_disabled = venom_color_rgb(150, 150, 150);
    bar->mnemonic_color = venom_color_rgb(30, 30, 30);
    bar->height = 28.0f;
    bar->item_padding_h = 12.0f;
    bar->item_padding_v = 4.0f;
    bar->corner_radius = 4.0f;
    
    /* Layout */
    widget->layout.min_height = bar->height;
}

static void menubar_destroy(VenomWidget* widget) {
    VenomMenuBar* bar = (VenomMenuBar*)widget;
    
    for (VenomU32 i = 0; i < bar->item_count; i++) {
        str_free(bar->items[i].label);
        str_free(bar->items[i].mnemonic);
    }
    
    if (bar->items) {
        venom_free(bar->items, bar->item_capacity * sizeof(VenomMenuBarItem));
    }
    
    venom_widget_class.destroy(widget);
}

/* ============================================================================
 * MENU MANAGEMENT
 * ============================================================================ */

static VenomResult menubar_grow(VenomMenuBar* bar) {
    VenomU32 new_capacity = bar->item_capacity == 0 ? 8 : bar->item_capacity * 2;
    VenomMenuBarItem* new_items = venom_alloc(new_capacity * sizeof(VenomMenuBarItem));
    if (!new_items) {
        return VENOM_ERR_UNIT(VENOM_ERROR_OUT_OF_MEMORY);
    }
    
    if (bar->items) {
        memcpy(new_items, bar->items, bar->item_count * sizeof(VenomMenuBarItem));
        venom_free(bar->items, bar->item_capacity * sizeof(VenomMenuBarItem));
    }
    
    bar->items = new_items;
    bar->item_capacity = new_capacity;
    return VENOM_OK_UNIT();
}

VenomResultPtr venom_menubar_create(void) {
    VenomResultPtr result = venom_widget_create(&venom_menubar_class);
    return result;
}

VenomResult venom_menubar_add_menu(VenomMenuBar* bar, const char* label,
                                    VenomContextMenu* submenu) {
    if (!bar || !label) {
        return VENOM_ERR_UNIT(VENOM_ERROR_INVALID_ARGUMENT);
    }
    
    if (bar->item_count >= bar->item_capacity) {
        VenomResult r = menubar_grow(bar);
        if (!r.ok) return r;
    }
    
    VenomMenuBarItem* item = &bar->items[bar->item_count];
    memset(item, 0, sizeof(VenomMenuBarItem));
    
    item->label = str_dup(label);
    item->submenu = submenu;
    item->enabled = VENOM_TRUE;
    item->mnemonic = NULL;
    
    bar->item_count++;
    venom_widget_invalidate(&bar->base);
    
    return VENOM_OK_UNIT();
}

VenomResult venom_menubar_add_menu_with_mnemonic(VenomMenuBar* bar, 
                                                   const char* label,
                                                   VenomContextMenu* submenu) {
    if (!bar || !label) {
        return VENOM_ERR_UNIT(VENOM_ERROR_INVALID_ARGUMENT);
    }
    
    if (bar->item_count >= bar->item_capacity) {
        VenomResult r = menubar_grow(bar);
        if (!r.ok) return r;
    }
    
    VenomMenuBarItem* item = &bar->items[bar->item_count];
    memset(item, 0, sizeof(VenomMenuBarItem));
    
    char mnemonic;
    parse_mnemonic(label, &item->label, &mnemonic);
    
    if (mnemonic != '\0') {
        item->mnemonic = venom_alloc(2);
        if (item->mnemonic) {
            item->mnemonic[0] = mnemonic;
            item->mnemonic[1] = '\0';
        }
    }
    
    item->submenu = submenu;
    item->enabled = VENOM_TRUE;
    
    bar->item_count++;
    venom_widget_invalidate(&bar->base);
    
    return VENOM_OK_UNIT();
}

VenomResult venom_menubar_remove_menu(VenomMenuBar* bar, VenomU32 index) {
    if (!bar || index >= bar->item_count) {
        return VENOM_ERR_UNIT(VENOM_ERROR_OUT_OF_BOUNDS);
    }
    
    VenomMenuBarItem* item = &bar->items[index];
    str_free(item->label);
    str_free(item->mnemonic);
    
    for (VenomU32 i = index; i < bar->item_count - 1; i++) {
        bar->items[i] = bar->items[i + 1];
    }
    bar->item_count--;
    
    venom_widget_invalidate(&bar->base);
    return VENOM_OK_UNIT();
}

void venom_menubar_clear(VenomMenuBar* bar) {
    if (!bar) return;
    
    for (VenomU32 i = 0; i < bar->item_count; i++) {
        str_free(bar->items[i].label);
        str_free(bar->items[i].mnemonic);
    }
    bar->item_count = 0;
    
    venom_widget_invalidate(&bar->base);
}

VenomU32 venom_menubar_get_count(const VenomMenuBar* bar) {
    return bar ? bar->item_count : 0;
}

void venom_menubar_set_enabled(VenomMenuBar* bar, VenomU32 index, VenomBool enabled) {
    if (!bar || index >= bar->item_count) return;
    bar->items[index].enabled = enabled;
    venom_widget_invalidate(&bar->base);
}

/* ============================================================================
 * APPEARANCE
 * ============================================================================ */

void venom_menubar_set_background(VenomMenuBar* bar, VenomColor color) {
    if (!bar) return;
    bar->background = color;
    venom_widget_invalidate(&bar->base);
}

void venom_menubar_set_hover_background(VenomMenuBar* bar, VenomColor color) {
    if (!bar) return;
    bar->hover_background = color;
    venom_widget_invalidate(&bar->base);
}

void venom_menubar_set_text_color(VenomMenuBar* bar, VenomColor color) {
    if (!bar) return;
    bar->text_color = color;
    venom_widget_invalidate(&bar->base);
}

void venom_menubar_set_height(VenomMenuBar* bar, VenomF32 height) {
    if (!bar) return;
    bar->height = height;
    bar->base.layout.min_height = height;
    venom_widget_invalidate(&bar->base);
}

void venom_menubar_set_padding(VenomMenuBar* bar, VenomF32 horizontal, VenomF32 vertical) {
    if (!bar) return;
    bar->item_padding_h = horizontal;
    bar->item_padding_v = vertical;
    venom_widget_invalidate(&bar->base);
}

/* ============================================================================
 * MENU OPEN/CLOSE
 * ============================================================================ */

void venom_menubar_open_menu(VenomMenuBar* bar, VenomU32 index) {
    if (!bar || index >= bar->item_count) return;
    if (!bar->items[index].enabled) return;
    
    if (bar->menu_open && bar->active_index != (VenomI32)index) {
        if (bar->active_index >= 0 && bar->items[bar->active_index].submenu) {
            venom_context_menu_hide(bar->items[bar->active_index].submenu);
        }
    }
    
    bar->active_index = (VenomI32)index;
    bar->menu_open = VENOM_TRUE;
    
    VenomMenuBarItem* item = &bar->items[index];
    if (item->submenu) {
        VenomF32 x = bar->base.bounds.x + item->x;
        VenomF32 y = bar->base.bounds.y + bar->height;
        venom_context_menu_show(item->submenu, x, y);
    }
    
    venom_widget_invalidate(&bar->base);
}

void venom_menubar_close_menus(VenomMenuBar* bar) {
    if (!bar) return;
    
    if (bar->menu_open && bar->active_index >= 0 && 
        bar->items[bar->active_index].submenu) {
        venom_context_menu_hide(bar->items[bar->active_index].submenu);
    }
    
    bar->active_index = -1;
    bar->menu_open = VENOM_FALSE;
    venom_widget_invalidate(&bar->base);
}

VenomBool venom_menubar_is_open(const VenomMenuBar* bar) {
    return bar ? bar->menu_open : VENOM_FALSE;
}

/* ============================================================================
 * KEYBOARD NAVIGATION
 * ============================================================================ */

VenomBool venom_menubar_handle_mnemonic(VenomMenuBar* bar, char key) {
    if (!bar) return VENOM_FALSE;
    
    char upper_key = (char)toupper((unsigned char)key);
    
    for (VenomU32 i = 0; i < bar->item_count; i++) {
        if (bar->items[i].mnemonic && 
            bar->items[i].mnemonic[0] == upper_key &&
            bar->items[i].enabled) {
            venom_menubar_open_menu(bar, i);
            return VENOM_TRUE;
        }
    }
    
    return VENOM_FALSE;
}

/* ============================================================================
 * WIDGET CALLBACKS
 * ============================================================================ */

static void menubar_measure(VenomWidget* widget, VenomF32 available_width, 
                             VenomF32 available_height, VenomF32* out_width, VenomF32* out_height) {
    VenomMenuBar* bar = (VenomMenuBar*)widget;
    (void)available_height;
    
    VenomF32 total_width = 0;
    VenomF32 font_size = 13.0f;
    
    for (VenomU32 i = 0; i < bar->item_count; i++) {
        VenomMenuBarItem* item = &bar->items[i];
        item->x = total_width;
        item->width = calculate_text_width(item->label, font_size) + 
                      bar->item_padding_h * 2;
        total_width += item->width;
    }
    
    *out_width = available_width > 0 ? available_width : total_width;
    *out_height = bar->height;
}

static void menubar_layout(VenomWidget* widget, VenomRectF bounds) {
    widget->bounds = bounds;
}

static void menubar_draw(VenomWidget* widget, VenomCanvas* canvas) {
    VenomMenuBar* bar = (VenomMenuBar*)widget;
    VenomRectF bounds = widget->bounds;
    
    /* Draw background */
    VenomPaint bg_paint = venom_paint_fill(bar->background);
    venom_canvas_draw_rect(canvas, bounds, &bg_paint);
    
    /* Draw bottom border */
    VenomPaint border_paint = venom_paint_stroke(venom_color_rgba(0, 0, 0, 30), 1.0f);
    venom_canvas_draw_line(canvas, bounds.x, bounds.y + bounds.height - 0.5f,
                            bounds.x + bounds.width, bounds.y + bounds.height - 0.5f,
                            &border_paint);
    
    /* Draw menu items */
    VenomF32 x = bounds.x;
    
    for (VenomU32 i = 0; i < bar->item_count; i++) {
        VenomMenuBarItem* item = &bar->items[i];
        VenomRectF item_rect = {
            x, bounds.y + bar->item_padding_v,
            item->width, bounds.height - bar->item_padding_v * 2
        };
        
        /* Draw hover/active background */
        if ((VenomI32)i == bar->active_index && bar->menu_open) {
            VenomPaint active_paint = venom_paint_fill(bar->active_background);
            venom_canvas_draw_rounded_rect(canvas, item_rect, bar->corner_radius, &active_paint);
        } else if ((VenomI32)i == bar->hover_index) {
            VenomPaint hover_paint = venom_paint_fill(bar->hover_background);
            venom_canvas_draw_rounded_rect(canvas, item_rect, bar->corner_radius, &hover_paint);
        }
        
        /* Draw text */
        VenomColor text_color = item->enabled ? bar->text_color : bar->text_color_disabled;
        VenomPaint text_paint = venom_paint_fill(text_color);
        VenomF32 text_x = x + bar->item_padding_h;
        VenomF32 text_y = bounds.y + bounds.height / 2 + 5;
        venom_canvas_draw_text(canvas, item->label, text_x, text_y, NULL, &text_paint);
        
        x += item->width;
    }
}

static VenomI32 menubar_hit_test(VenomMenuBar* bar, VenomF32 x) {
    VenomF32 rel_x = x - bar->base.bounds.x;
    
    for (VenomU32 i = 0; i < bar->item_count; i++) {
        VenomMenuBarItem* item = &bar->items[i];
        if (rel_x >= item->x && rel_x < item->x + item->width) {
            return (VenomI32)i;
        }
    }
    return -1;
}

static VenomBool menubar_on_event(VenomWidget* widget, const VenomEvent* event) {
    VenomMenuBar* bar = (VenomMenuBar*)widget;
    
    switch (event->type) {
        case VENOM_EVENT_MOUSE_MOVE: {
            VenomI32 index = menubar_hit_test(bar, (VenomF32)event->mouse.x);
            if (index != bar->hover_index) {
                bar->hover_index = index;
                
                if (bar->menu_open && index >= 0 && index != bar->active_index) {
                    venom_menubar_open_menu(bar, (VenomU32)index);
                }
                
                venom_widget_invalidate(widget);
            }
            return VENOM_TRUE;
        }
        
        case VENOM_EVENT_MOUSE_LEAVE:
            if (!bar->menu_open) {
                bar->hover_index = -1;
                venom_widget_invalidate(widget);
            }
            return VENOM_TRUE;
            
        case VENOM_EVENT_MOUSE_BUTTON_DOWN: {
            if (event->mouse.button != VENOM_MOUSE_BUTTON_LEFT) {
                return VENOM_FALSE;
            }
            
            VenomI32 index = menubar_hit_test(bar, (VenomF32)event->mouse.x);
            if (index >= 0) {
                if (bar->menu_open && bar->active_index == index) {
                    venom_menubar_close_menus(bar);
                } else {
                    venom_menubar_open_menu(bar, (VenomU32)index);
                }
                return VENOM_TRUE;
            }
            return VENOM_FALSE;
        }
        
        case VENOM_EVENT_KEY_DOWN: {
            if (event->key.modifiers & VENOM_KEYMOD_ALT) {
                if (event->key.key >= VENOM_KEY_A && event->key.key <= VENOM_KEY_Z) {
                    char key = (char)('A' + (event->key.key - VENOM_KEY_A));
                    return venom_menubar_handle_mnemonic(bar, key);
                }
            }
            
            if (bar->menu_open) {
                if (event->key.key == VENOM_KEY_LEFT) {
                    VenomI32 new_index = bar->active_index - 1;
                    if (new_index < 0) new_index = (VenomI32)bar->item_count - 1;
                    venom_menubar_open_menu(bar, (VenomU32)new_index);
                    return VENOM_TRUE;
                } else if (event->key.key == VENOM_KEY_RIGHT) {
                    VenomU32 new_index = (VenomU32)bar->active_index + 1;
                    if (new_index >= bar->item_count) new_index = 0;
                    venom_menubar_open_menu(bar, new_index);
                    return VENOM_TRUE;
                } else if (event->key.key == VENOM_KEY_ESCAPE) {
                    venom_menubar_close_menus(bar);
                    return VENOM_TRUE;
                }
            }
            return VENOM_FALSE;
        }
        
        default:
            return VENOM_FALSE;
    }
}
