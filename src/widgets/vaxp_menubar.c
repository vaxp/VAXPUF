/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_menubar.c - Menu Bar Widget implementation
 */

#include "vaxp/widgets/vaxp_menubar.h"
#include "vaxp/core/vaxp_memory.h"
#include "vaxp/graphics/vaxp_canvas.h"
#include <string.h>
#include <ctype.h>

/* ============================================================================
 * FORWARD DECLARATIONS
 * ============================================================================ */

static void menubar_init(VaxpWidget* widget);
static void menubar_destroy(VaxpWidget* widget);
static void menubar_measure(VaxpWidget* widget, VaxpF32 available_width, VaxpF32 available_height,
                             VaxpF32* out_width, VaxpF32* out_height);
static void menubar_layout(VaxpWidget* widget, VaxpRectF bounds);
static void menubar_draw(VaxpWidget* widget, VaxpCanvas* canvas);
static VaxpBool menubar_on_event(VaxpWidget* widget, const VaxpEvent* event);

/* ============================================================================
 * WIDGET CLASS
 * ============================================================================ */

const VaxpWidgetClass vaxp_menubar_class = {
    .class_name = "VaxpMenuBar",
    .instance_size = sizeof(VaxpMenuBar),
    .parent_class = &vaxp_widget_class,
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
    char* copy = vaxp_alloc(len + 1);
    if (copy) memcpy(copy, s, len + 1);
    return copy;
}

static void str_free(char* s) {
    if (s) vaxp_free(s, strlen(s) + 1);
}

static void parse_mnemonic(const char* label, char** out_text, char* out_mnemonic) {
    if (!label) {
        *out_text = NULL;
        *out_mnemonic = '\0';
        return;
    }
    
    size_t len = strlen(label);
    *out_text = vaxp_alloc(len + 1);
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

static VaxpF32 calculate_text_width(const char* text, VaxpF32 font_size) {
    if (!text) return 0;
    return (VaxpF32)strlen(text) * font_size * 0.6f;
}

/* ============================================================================
 * LIFECYCLE
 * ============================================================================ */

static void menubar_init(VaxpWidget* widget) {
    VaxpMenuBar* bar = (VaxpMenuBar*)widget;
    
    /* Initialize state */
    bar->items = NULL;
    bar->item_count = 0;
    bar->item_capacity = 0;
    bar->hover_index = -1;
    bar->active_index = -1;
    bar->menu_open = VAXP_FALSE;
    
    /* Default appearance */
    bar->background = vaxp_color_rgb(245, 245, 245);
    bar->hover_background = vaxp_color_rgba(0, 0, 0, 20);
    bar->active_background = vaxp_color_rgba(0, 0, 0, 40);
    bar->text_color = vaxp_color_rgb(30, 30, 30);
    bar->text_color_disabled = vaxp_color_rgb(150, 150, 150);
    bar->mnemonic_color = vaxp_color_rgb(30, 30, 30);
    bar->height = 28.0f;
    bar->item_padding_h = 12.0f;
    bar->item_padding_v = 4.0f;
    bar->corner_radius = 4.0f;
    
    /* Layout */
    widget->layout.min_height = bar->height;
}

static void menubar_destroy(VaxpWidget* widget) {
    VaxpMenuBar* bar = (VaxpMenuBar*)widget;
    
    for (VaxpU32 i = 0; i < bar->item_count; i++) {
        str_free(bar->items[i].label);
        str_free(bar->items[i].mnemonic);
    }
    
    if (bar->items) {
        vaxp_free(bar->items, bar->item_capacity * sizeof(VaxpMenuBarItem));
    }
    
    vaxp_widget_class.destroy(widget);
}

/* ============================================================================
 * MENU MANAGEMENT
 * ============================================================================ */

static VaxpResult menubar_grow(VaxpMenuBar* bar) {
    VaxpU32 new_capacity = bar->item_capacity == 0 ? 8 : bar->item_capacity * 2;
    VaxpMenuBarItem* new_items = vaxp_alloc(new_capacity * sizeof(VaxpMenuBarItem));
    if (!new_items) {
        return VAXP_ERR_UNIT(VAXP_ERROR_OUT_OF_MEMORY);
    }
    
    if (bar->items) {
        memcpy(new_items, bar->items, bar->item_count * sizeof(VaxpMenuBarItem));
        vaxp_free(bar->items, bar->item_capacity * sizeof(VaxpMenuBarItem));
    }
    
    bar->items = new_items;
    bar->item_capacity = new_capacity;
    return VAXP_OK_UNIT();
}

VaxpResultPtr vaxp_menubar_create(void) {
    VaxpResultPtr result = vaxp_widget_create(&vaxp_menubar_class);
    return result;
}

VaxpResult vaxp_menubar_add_menu(VaxpMenuBar* bar, const char* label,
                                    VaxpContextMenu* submenu) {
    if (!bar || !label) {
        return VAXP_ERR_UNIT(VAXP_ERROR_INVALID_ARGUMENT);
    }
    
    if (bar->item_count >= bar->item_capacity) {
        VaxpResult r = menubar_grow(bar);
        if (!r.ok) return r;
    }
    
    VaxpMenuBarItem* item = &bar->items[bar->item_count];
    memset(item, 0, sizeof(VaxpMenuBarItem));
    
    item->label = str_dup(label);
    item->submenu = submenu;
    item->enabled = VAXP_TRUE;
    item->mnemonic = NULL;
    
    bar->item_count++;
    vaxp_widget_invalidate(&bar->base);
    
    return VAXP_OK_UNIT();
}

VaxpResult vaxp_menubar_add_menu_with_mnemonic(VaxpMenuBar* bar, 
                                                   const char* label,
                                                   VaxpContextMenu* submenu) {
    if (!bar || !label) {
        return VAXP_ERR_UNIT(VAXP_ERROR_INVALID_ARGUMENT);
    }
    
    if (bar->item_count >= bar->item_capacity) {
        VaxpResult r = menubar_grow(bar);
        if (!r.ok) return r;
    }
    
    VaxpMenuBarItem* item = &bar->items[bar->item_count];
    memset(item, 0, sizeof(VaxpMenuBarItem));
    
    char mnemonic;
    parse_mnemonic(label, &item->label, &mnemonic);
    
    if (mnemonic != '\0') {
        item->mnemonic = vaxp_alloc(2);
        if (item->mnemonic) {
            item->mnemonic[0] = mnemonic;
            item->mnemonic[1] = '\0';
        }
    }
    
    item->submenu = submenu;
    item->enabled = VAXP_TRUE;
    
    bar->item_count++;
    vaxp_widget_invalidate(&bar->base);
    
    return VAXP_OK_UNIT();
}

VaxpResult vaxp_menubar_remove_menu(VaxpMenuBar* bar, VaxpU32 index) {
    if (!bar || index >= bar->item_count) {
        return VAXP_ERR_UNIT(VAXP_ERROR_OUT_OF_BOUNDS);
    }
    
    VaxpMenuBarItem* item = &bar->items[index];
    str_free(item->label);
    str_free(item->mnemonic);
    
    for (VaxpU32 i = index; i < bar->item_count - 1; i++) {
        bar->items[i] = bar->items[i + 1];
    }
    bar->item_count--;
    
    vaxp_widget_invalidate(&bar->base);
    return VAXP_OK_UNIT();
}

void vaxp_menubar_clear(VaxpMenuBar* bar) {
    if (!bar) return;
    
    for (VaxpU32 i = 0; i < bar->item_count; i++) {
        str_free(bar->items[i].label);
        str_free(bar->items[i].mnemonic);
    }
    bar->item_count = 0;
    
    vaxp_widget_invalidate(&bar->base);
}

VaxpU32 vaxp_menubar_get_count(const VaxpMenuBar* bar) {
    return bar ? bar->item_count : 0;
}

void vaxp_menubar_set_enabled(VaxpMenuBar* bar, VaxpU32 index, VaxpBool enabled) {
    if (!bar || index >= bar->item_count) return;
    bar->items[index].enabled = enabled;
    vaxp_widget_invalidate(&bar->base);
}

/* ============================================================================
 * APPEARANCE
 * ============================================================================ */

void vaxp_menubar_set_background(VaxpMenuBar* bar, VaxpColor color) {
    if (!bar) return;
    bar->background = color;
    vaxp_widget_invalidate(&bar->base);
}

void vaxp_menubar_set_hover_background(VaxpMenuBar* bar, VaxpColor color) {
    if (!bar) return;
    bar->hover_background = color;
    vaxp_widget_invalidate(&bar->base);
}

void vaxp_menubar_set_text_color(VaxpMenuBar* bar, VaxpColor color) {
    if (!bar) return;
    bar->text_color = color;
    vaxp_widget_invalidate(&bar->base);
}

void vaxp_menubar_set_height(VaxpMenuBar* bar, VaxpF32 height) {
    if (!bar) return;
    bar->height = height;
    bar->base.layout.min_height = height;
    vaxp_widget_invalidate(&bar->base);
}

void vaxp_menubar_set_padding(VaxpMenuBar* bar, VaxpF32 horizontal, VaxpF32 vertical) {
    if (!bar) return;
    bar->item_padding_h = horizontal;
    bar->item_padding_v = vertical;
    vaxp_widget_invalidate(&bar->base);
}

/* ============================================================================
 * MENU OPEN/CLOSE
 * ============================================================================ */

void vaxp_menubar_open_menu(VaxpMenuBar* bar, VaxpU32 index) {
    if (!bar || index >= bar->item_count) return;
    if (!bar->items[index].enabled) return;
    
    if (bar->menu_open && bar->active_index != (VaxpI32)index) {
        if (bar->active_index >= 0 && bar->items[bar->active_index].submenu) {
            vaxp_context_menu_hide(bar->items[bar->active_index].submenu);
        }
    }
    
    bar->active_index = (VaxpI32)index;
    bar->menu_open = VAXP_TRUE;
    
    VaxpMenuBarItem* item = &bar->items[index];
    if (item->submenu) {
        VaxpF32 x = bar->base.bounds.x + item->x;
        VaxpF32 y = bar->base.bounds.y + bar->height;
        vaxp_context_menu_show(item->submenu, x, y);
    }
    
    vaxp_widget_invalidate(&bar->base);
}

void vaxp_menubar_close_menus(VaxpMenuBar* bar) {
    if (!bar) return;
    
    if (bar->menu_open && bar->active_index >= 0 && 
        bar->items[bar->active_index].submenu) {
        vaxp_context_menu_hide(bar->items[bar->active_index].submenu);
    }
    
    bar->active_index = -1;
    bar->menu_open = VAXP_FALSE;
    vaxp_widget_invalidate(&bar->base);
}

VaxpBool vaxp_menubar_is_open(const VaxpMenuBar* bar) {
    return bar ? bar->menu_open : VAXP_FALSE;
}

/* ============================================================================
 * KEYBOARD NAVIGATION
 * ============================================================================ */

VaxpBool vaxp_menubar_handle_mnemonic(VaxpMenuBar* bar, char key) {
    if (!bar) return VAXP_FALSE;
    
    char upper_key = (char)toupper((unsigned char)key);
    
    for (VaxpU32 i = 0; i < bar->item_count; i++) {
        if (bar->items[i].mnemonic && 
            bar->items[i].mnemonic[0] == upper_key &&
            bar->items[i].enabled) {
            vaxp_menubar_open_menu(bar, i);
            return VAXP_TRUE;
        }
    }
    
    return VAXP_FALSE;
}

/* ============================================================================
 * WIDGET CALLBACKS
 * ============================================================================ */

static void menubar_measure(VaxpWidget* widget, VaxpF32 available_width, 
                             VaxpF32 available_height, VaxpF32* out_width, VaxpF32* out_height) {
    VaxpMenuBar* bar = (VaxpMenuBar*)widget;
    (void)available_height;
    
    VaxpF32 total_width = 0;
    VaxpF32 font_size = 13.0f;
    
    for (VaxpU32 i = 0; i < bar->item_count; i++) {
        VaxpMenuBarItem* item = &bar->items[i];
        item->x = total_width;
        item->width = calculate_text_width(item->label, font_size) + 
                      bar->item_padding_h * 2;
        total_width += item->width;
    }
    
    *out_width = available_width > 0 ? available_width : total_width;
    *out_height = bar->height;
}

static void menubar_layout(VaxpWidget* widget, VaxpRectF bounds) {
    widget->bounds = bounds;
}

static void menubar_draw(VaxpWidget* widget, VaxpCanvas* canvas) {
    VaxpMenuBar* bar = (VaxpMenuBar*)widget;
    VaxpRectF bounds = widget->bounds;
    
    /* Draw background */
    VaxpPaint bg_paint = vaxp_paint_fill(bar->background);
    vaxp_canvas_draw_rect(canvas, bounds, &bg_paint);
    
    /* Draw bottom border */
    VaxpPaint border_paint = vaxp_paint_stroke(vaxp_color_rgba(0, 0, 0, 30), 1.0f);
    vaxp_canvas_draw_line(canvas, bounds.x, bounds.y + bounds.height - 0.5f,
                            bounds.x + bounds.width, bounds.y + bounds.height - 0.5f,
                            &border_paint);
    
    /* Draw menu items */
    VaxpF32 x = bounds.x;
    
    for (VaxpU32 i = 0; i < bar->item_count; i++) {
        VaxpMenuBarItem* item = &bar->items[i];
        VaxpRectF item_rect = {
            x, bounds.y + bar->item_padding_v,
            item->width, bounds.height - bar->item_padding_v * 2
        };
        
        /* Draw hover/active background */
        if ((VaxpI32)i == bar->active_index && bar->menu_open) {
            VaxpPaint active_paint = vaxp_paint_fill(bar->active_background);
            vaxp_canvas_draw_rounded_rect(canvas, item_rect, bar->corner_radius, &active_paint);
        } else if ((VaxpI32)i == bar->hover_index) {
            VaxpPaint hover_paint = vaxp_paint_fill(bar->hover_background);
            vaxp_canvas_draw_rounded_rect(canvas, item_rect, bar->corner_radius, &hover_paint);
        }
        
        /* Draw text */
        VaxpColor text_color = item->enabled ? bar->text_color : bar->text_color_disabled;
        VaxpPaint text_paint = vaxp_paint_fill(text_color);
        VaxpF32 text_x = x + bar->item_padding_h;
        VaxpF32 text_y = bounds.y + bounds.height / 2 + 5;
        vaxp_canvas_draw_text(canvas, item->label, text_x, text_y, NULL, &text_paint);
        
        x += item->width;
    }
}

static VaxpI32 menubar_hit_test(VaxpMenuBar* bar, VaxpF32 x) {
    VaxpF32 rel_x = x - bar->base.bounds.x;
    
    for (VaxpU32 i = 0; i < bar->item_count; i++) {
        VaxpMenuBarItem* item = &bar->items[i];
        if (rel_x >= item->x && rel_x < item->x + item->width) {
            return (VaxpI32)i;
        }
    }
    return -1;
}

static VaxpBool menubar_on_event(VaxpWidget* widget, const VaxpEvent* event) {
    VaxpMenuBar* bar = (VaxpMenuBar*)widget;
    
    switch (event->type) {
        case VAXP_EVENT_MOUSE_MOVE: {
            VaxpI32 index = menubar_hit_test(bar, (VaxpF32)event->mouse.x);
            if (index != bar->hover_index) {
                bar->hover_index = index;
                
                if (bar->menu_open && index >= 0 && index != bar->active_index) {
                    vaxp_menubar_open_menu(bar, (VaxpU32)index);
                }
                
                vaxp_widget_invalidate(widget);
            }
            return VAXP_TRUE;
        }
        
        case VAXP_EVENT_MOUSE_LEAVE:
            if (!bar->menu_open) {
                bar->hover_index = -1;
                vaxp_widget_invalidate(widget);
            }
            return VAXP_TRUE;
            
        case VAXP_EVENT_MOUSE_BUTTON_DOWN: {
            if (event->mouse.button != VAXP_MOUSE_BUTTON_LEFT) {
                return VAXP_FALSE;
            }
            
            VaxpI32 index = menubar_hit_test(bar, (VaxpF32)event->mouse.x);
            if (index >= 0) {
                if (bar->menu_open && bar->active_index == index) {
                    vaxp_menubar_close_menus(bar);
                } else {
                    vaxp_menubar_open_menu(bar, (VaxpU32)index);
                }
                return VAXP_TRUE;
            }
            return VAXP_FALSE;
        }
        
        case VAXP_EVENT_KEY_DOWN: {
            if (event->key.modifiers & VAXP_KEYMOD_ALT) {
                if (event->key.key >= VAXP_KEY_A && event->key.key <= VAXP_KEY_Z) {
                    char key = (char)('A' + (event->key.key - VAXP_KEY_A));
                    return vaxp_menubar_handle_mnemonic(bar, key);
                }
            }
            
            if (bar->menu_open) {
                if (event->key.key == VAXP_KEY_LEFT) {
                    VaxpI32 new_index = bar->active_index - 1;
                    if (new_index < 0) new_index = (VaxpI32)bar->item_count - 1;
                    vaxp_menubar_open_menu(bar, (VaxpU32)new_index);
                    return VAXP_TRUE;
                } else if (event->key.key == VAXP_KEY_RIGHT) {
                    VaxpU32 new_index = (VaxpU32)bar->active_index + 1;
                    if (new_index >= bar->item_count) new_index = 0;
                    vaxp_menubar_open_menu(bar, new_index);
                    return VAXP_TRUE;
                } else if (event->key.key == VAXP_KEY_ESCAPE) {
                    vaxp_menubar_close_menus(bar);
                    return VAXP_TRUE;
                }
            }
            return VAXP_FALSE;
        }
        
        default:
            return VAXP_FALSE;
    }
}
