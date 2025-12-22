/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_toolbar.c - Toolbar Widget implementation
 */

#include "venom/widgets/venom_toolbar.h"
#include "venom/core/venom_memory.h"
#include "venom/graphics/venom_canvas.h"
#include <string.h>

/* ============================================================================
 * FORWARD DECLARATIONS
 * ============================================================================ */

static void toolbar_init(VenomWidget* widget);
static void toolbar_destroy(VenomWidget* widget);
static void toolbar_measure(VenomWidget* widget, VenomF32 available_width, VenomF32 available_height,
                             VenomF32* out_width, VenomF32* out_height);
static void toolbar_layout(VenomWidget* widget, VenomRectF bounds);
static void toolbar_draw(VenomWidget* widget, VenomCanvas* canvas);
static VenomBool toolbar_on_event(VenomWidget* widget, const VenomEvent* event);

/* ============================================================================
 * WIDGET CLASS
 * ============================================================================ */

const VenomWidgetClass venom_toolbar_class = {
    .class_name = "VenomToolbar",
    .instance_size = sizeof(VenomToolbar),
    .parent_class = &venom_widget_class,
    .init = toolbar_init,
    .destroy = toolbar_destroy,
    .measure = toolbar_measure,
    .layout = toolbar_layout,
    .draw = toolbar_draw,
    .on_event = toolbar_on_event,
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

/* ============================================================================
 * LIFECYCLE
 * ============================================================================ */

static void toolbar_init(VenomWidget* widget) {
    VenomToolbar* toolbar = (VenomToolbar*)widget;
    
    toolbar->items = NULL;
    toolbar->item_count = 0;
    toolbar->item_capacity = 0;
    toolbar->hover_index = -1;
    toolbar->pressed_index = -1;
    
    toolbar->button_style = VENOM_TOOLBAR_STYLE_ICON_ONLY;
    toolbar->background = venom_color_rgb(250, 250, 250);
    toolbar->button_hover = venom_color_rgba(0, 0, 0, 20);
    toolbar->button_pressed = venom_color_rgba(0, 0, 0, 40);
    toolbar->button_toggled = venom_color_rgba(100, 150, 255, 60);
    toolbar->icon_color = venom_color_rgb(50, 50, 50);
    toolbar->text_color = venom_color_rgb(50, 50, 50);
    toolbar->separator_color = venom_color_rgba(0, 0, 0, 30);
    toolbar->height = 40.0f;
    toolbar->button_size = 32.0f;
    toolbar->spacing = 2.0f;
    toolbar->padding_h = 8.0f;
    toolbar->corner_radius = 6.0f;
    toolbar->icon_size = 20.0f;
    
    widget->layout.min_height = toolbar->height;
}

static void toolbar_destroy(VenomWidget* widget) {
    VenomToolbar* toolbar = (VenomToolbar*)widget;
    
    for (VenomU32 i = 0; i < toolbar->item_count; i++) {
        VenomToolbarItem* item = &toolbar->items[i];
        str_free(item->label);
        str_free(item->icon);
        str_free(item->tooltip);
    }
    
    if (toolbar->items) {
        venom_free(toolbar->items, toolbar->item_capacity * sizeof(VenomToolbarItem));
    }
    
    venom_widget_class.destroy(widget);
}

VenomResultPtr venom_toolbar_create(void) {
    return venom_widget_create(&venom_toolbar_class);
}

/* ============================================================================
 * ITEM MANAGEMENT
 * ============================================================================ */

static VenomResult toolbar_grow(VenomToolbar* toolbar) {
    VenomU32 new_capacity = toolbar->item_capacity == 0 ? 16 : toolbar->item_capacity * 2;
    VenomToolbarItem* new_items = venom_alloc(new_capacity * sizeof(VenomToolbarItem));
    if (!new_items) {
        return VENOM_ERR_UNIT(VENOM_ERROR_OUT_OF_MEMORY);
    }
    
    if (toolbar->items) {
        memcpy(new_items, toolbar->items, toolbar->item_count * sizeof(VenomToolbarItem));
        venom_free(toolbar->items, toolbar->item_capacity * sizeof(VenomToolbarItem));
    }
    
    toolbar->items = new_items;
    toolbar->item_capacity = new_capacity;
    return VENOM_OK_UNIT();
}

VenomI32 venom_toolbar_add_button(VenomToolbar* toolbar,
                                   const char* icon,
                                   const char* label,
                                   const char* tooltip,
                                   VenomToolbarCallback callback,
                                   void* user_data) {
    if (!toolbar) return -1;
    
    if (toolbar->item_count >= toolbar->item_capacity) {
        if (!toolbar_grow(toolbar).ok) return -1;
    }
    
    VenomToolbarItem* item = &toolbar->items[toolbar->item_count];
    memset(item, 0, sizeof(VenomToolbarItem));
    
    item->type = VENOM_TOOLBAR_ITEM_BUTTON;
    item->icon = str_dup(icon);
    item->label = str_dup(label);
    item->tooltip = str_dup(tooltip);
    item->enabled = VENOM_TRUE;
    item->toggled = VENOM_FALSE;
    item->on_click = callback;
    item->user_data = user_data;
    
    VenomI32 index = (VenomI32)toolbar->item_count;
    toolbar->item_count++;
    venom_widget_invalidate(&toolbar->base);
    
    return index;
}

VenomI32 venom_toolbar_add_toggle(VenomToolbar* toolbar,
                                   const char* icon,
                                   const char* label,
                                   const char* tooltip,
                                   VenomBool initial_state,
                                   VenomToolbarCallback callback,
                                   void* user_data) {
    if (!toolbar) return -1;
    
    if (toolbar->item_count >= toolbar->item_capacity) {
        if (!toolbar_grow(toolbar).ok) return -1;
    }
    
    VenomToolbarItem* item = &toolbar->items[toolbar->item_count];
    memset(item, 0, sizeof(VenomToolbarItem));
    
    item->type = VENOM_TOOLBAR_ITEM_TOGGLE;
    item->icon = str_dup(icon);
    item->label = str_dup(label);
    item->tooltip = str_dup(tooltip);
    item->enabled = VENOM_TRUE;
    item->toggled = initial_state;
    item->on_click = callback;
    item->user_data = user_data;
    
    VenomI32 index = (VenomI32)toolbar->item_count;
    toolbar->item_count++;
    venom_widget_invalidate(&toolbar->base);
    
    return index;
}

VenomI32 venom_toolbar_add_separator(VenomToolbar* toolbar) {
    if (!toolbar) return -1;
    
    if (toolbar->item_count >= toolbar->item_capacity) {
        if (!toolbar_grow(toolbar).ok) return -1;
    }
    
    VenomToolbarItem* item = &toolbar->items[toolbar->item_count];
    memset(item, 0, sizeof(VenomToolbarItem));
    
    item->type = VENOM_TOOLBAR_ITEM_SEPARATOR;
    item->enabled = VENOM_TRUE;
    
    VenomI32 index = (VenomI32)toolbar->item_count;
    toolbar->item_count++;
    venom_widget_invalidate(&toolbar->base);
    
    return index;
}

VenomI32 venom_toolbar_add_spacer(VenomToolbar* toolbar) {
    if (!toolbar) return -1;
    
    if (toolbar->item_count >= toolbar->item_capacity) {
        if (!toolbar_grow(toolbar).ok) return -1;
    }
    
    VenomToolbarItem* item = &toolbar->items[toolbar->item_count];
    memset(item, 0, sizeof(VenomToolbarItem));
    
    item->type = VENOM_TOOLBAR_ITEM_SPACER;
    item->enabled = VENOM_TRUE;
    
    VenomI32 index = (VenomI32)toolbar->item_count;
    toolbar->item_count++;
    venom_widget_invalidate(&toolbar->base);
    
    return index;
}

VenomI32 venom_toolbar_add_widget(VenomToolbar* toolbar, VenomWidget* widget) {
    if (!toolbar || !widget) return -1;
    
    if (toolbar->item_count >= toolbar->item_capacity) {
        if (!toolbar_grow(toolbar).ok) return -1;
    }
    
    VenomToolbarItem* item = &toolbar->items[toolbar->item_count];
    memset(item, 0, sizeof(VenomToolbarItem));
    
    item->type = VENOM_TOOLBAR_ITEM_WIDGET;
    item->widget = widget;
    item->enabled = VENOM_TRUE;
    
    VenomI32 index = (VenomI32)toolbar->item_count;
    toolbar->item_count++;
    venom_widget_invalidate(&toolbar->base);
    
    return index;
}

VenomResult venom_toolbar_remove_item(VenomToolbar* toolbar, VenomU32 index) {
    if (!toolbar || index >= toolbar->item_count) {
        return VENOM_ERR_UNIT(VENOM_ERROR_OUT_OF_BOUNDS);
    }
    
    VenomToolbarItem* item = &toolbar->items[index];
    str_free(item->label);
    str_free(item->icon);
    str_free(item->tooltip);
    
    for (VenomU32 i = index; i < toolbar->item_count - 1; i++) {
        toolbar->items[i] = toolbar->items[i + 1];
    }
    toolbar->item_count--;
    
    venom_widget_invalidate(&toolbar->base);
    return VENOM_OK_UNIT();
}

void venom_toolbar_clear(VenomToolbar* toolbar) {
    if (!toolbar) return;
    
    for (VenomU32 i = 0; i < toolbar->item_count; i++) {
        str_free(toolbar->items[i].label);
        str_free(toolbar->items[i].icon);
        str_free(toolbar->items[i].tooltip);
    }
    toolbar->item_count = 0;
    
    venom_widget_invalidate(&toolbar->base);
}

VenomU32 venom_toolbar_get_count(const VenomToolbar* toolbar) {
    return toolbar ? toolbar->item_count : 0;
}

/* ============================================================================
 * ITEM STATE
 * ============================================================================ */

void venom_toolbar_set_enabled(VenomToolbar* toolbar, VenomU32 index, VenomBool enabled) {
    if (!toolbar || index >= toolbar->item_count) return;
    toolbar->items[index].enabled = enabled;
    venom_widget_invalidate(&toolbar->base);
}

VenomBool venom_toolbar_get_toggled(const VenomToolbar* toolbar, VenomU32 index) {
    if (!toolbar || index >= toolbar->item_count) return VENOM_FALSE;
    return toolbar->items[index].toggled;
}

void venom_toolbar_set_toggled(VenomToolbar* toolbar, VenomU32 index, VenomBool toggled) {
    if (!toolbar || index >= toolbar->item_count) return;
    toolbar->items[index].toggled = toggled;
    venom_widget_invalidate(&toolbar->base);
}

void venom_toolbar_set_tooltip(VenomToolbar* toolbar, VenomU32 index, const char* tooltip) {
    if (!toolbar || index >= toolbar->item_count) return;
    str_free(toolbar->items[index].tooltip);
    toolbar->items[index].tooltip = str_dup(tooltip);
}

/* ============================================================================
 * APPEARANCE
 * ============================================================================ */

void venom_toolbar_set_style(VenomToolbar* toolbar, VenomToolbarButtonStyle style) {
    if (!toolbar) return;
    toolbar->button_style = style;
    venom_widget_invalidate(&toolbar->base);
}

void venom_toolbar_set_background(VenomToolbar* toolbar, VenomColor color) {
    if (!toolbar) return;
    toolbar->background = color;
    venom_widget_invalidate(&toolbar->base);
}

void venom_toolbar_set_height(VenomToolbar* toolbar, VenomF32 height) {
    if (!toolbar) return;
    toolbar->height = height;
    toolbar->base.layout.min_height = height;
    venom_widget_invalidate(&toolbar->base);
}

void venom_toolbar_set_button_size(VenomToolbar* toolbar, VenomF32 size) {
    if (!toolbar) return;
    toolbar->button_size = size;
    venom_widget_invalidate(&toolbar->base);
}

void venom_toolbar_set_spacing(VenomToolbar* toolbar, VenomF32 spacing) {
    if (!toolbar) return;
    toolbar->spacing = spacing;
    venom_widget_invalidate(&toolbar->base);
}

void venom_toolbar_set_icon_size(VenomToolbar* toolbar, VenomF32 size) {
    if (!toolbar) return;
    toolbar->icon_size = size;
    venom_widget_invalidate(&toolbar->base);
}

/* ============================================================================
 * WIDGET CALLBACKS
 * ============================================================================ */

static void toolbar_measure(VenomWidget* widget, VenomF32 available_width, 
                             VenomF32 available_height, VenomF32* out_width, VenomF32* out_height) {
    VenomToolbar* toolbar = (VenomToolbar*)widget;
    (void)available_height;
    
    VenomF32 total_width = toolbar->padding_h * 2;
    VenomU32 spacer_count = 0;
    
    for (VenomU32 i = 0; i < toolbar->item_count; i++) {
        VenomToolbarItem* item = &toolbar->items[i];
        
        switch (item->type) {
            case VENOM_TOOLBAR_ITEM_BUTTON:
            case VENOM_TOOLBAR_ITEM_TOGGLE:
                item->width = toolbar->button_size;
                total_width += item->width + toolbar->spacing;
                break;
                
            case VENOM_TOOLBAR_ITEM_SEPARATOR:
                item->width = 8.0f;
                total_width += item->width + toolbar->spacing;
                break;
                
            case VENOM_TOOLBAR_ITEM_SPACER:
                spacer_count++;
                item->width = 0;
                break;
                
            case VENOM_TOOLBAR_ITEM_WIDGET:
                if (item->widget) {
                    item->width = item->widget->layout.preferred_width > 0 ? 
                                  item->widget->layout.preferred_width : 100.0f;
                    total_width += item->width + toolbar->spacing;
                }
                break;
        }
    }
    
    if (spacer_count > 0 && available_width > total_width) {
        VenomF32 spacer_width = (available_width - total_width) / spacer_count;
        for (VenomU32 i = 0; i < toolbar->item_count; i++) {
            if (toolbar->items[i].type == VENOM_TOOLBAR_ITEM_SPACER) {
                toolbar->items[i].width = spacer_width;
            }
        }
    }
    
    *out_width = available_width > 0 ? available_width : total_width;
    *out_height = toolbar->height;
}

static void toolbar_layout(VenomWidget* widget, VenomRectF bounds) {
    VenomToolbar* toolbar = (VenomToolbar*)widget;
    widget->bounds = bounds;
    
    VenomF32 x = toolbar->padding_h;
    for (VenomU32 i = 0; i < toolbar->item_count; i++) {
        VenomToolbarItem* item = &toolbar->items[i];
        item->x = x;
        x += item->width + toolbar->spacing;
    }
}

static void toolbar_draw(VenomWidget* widget, VenomCanvas* canvas) {
    VenomToolbar* toolbar = (VenomToolbar*)widget;
    VenomRectF bounds = widget->bounds;
    
    VenomPaint bg_paint = venom_paint_fill(toolbar->background);
    venom_canvas_draw_rect(canvas, bounds, &bg_paint);
    
    VenomPaint border_paint = venom_paint_stroke(venom_color_rgba(0, 0, 0, 25), 1.0f);
    venom_canvas_draw_line(canvas, bounds.x, bounds.y + bounds.height - 0.5f,
                            bounds.x + bounds.width, bounds.y + bounds.height - 0.5f,
                            &border_paint);
    
    VenomF32 button_y = bounds.y + (bounds.height - toolbar->button_size) / 2;
    
    for (VenomU32 i = 0; i < toolbar->item_count; i++) {
        VenomToolbarItem* item = &toolbar->items[i];
        VenomF32 x = bounds.x + item->x;
        
        switch (item->type) {
            case VENOM_TOOLBAR_ITEM_BUTTON:
            case VENOM_TOOLBAR_ITEM_TOGGLE: {
                VenomRectF btn_rect = { x, button_y, toolbar->button_size, toolbar->button_size };
                
                VenomColor bg_color = toolbar->background;
                if (!item->enabled) {
                    /* Disabled */
                } else if ((VenomI32)i == toolbar->pressed_index) {
                    bg_color = toolbar->button_pressed;
                } else if (item->toggled) {
                    bg_color = toolbar->button_toggled;
                } else if ((VenomI32)i == toolbar->hover_index) {
                    bg_color = toolbar->button_hover;
                }
                
                if (bg_color.a > 0) {
                    VenomPaint btn_paint = venom_paint_fill(bg_color);
                    venom_canvas_draw_rounded_rect(canvas, btn_rect, toolbar->corner_radius, &btn_paint);
                }
                
                VenomColor icon_color = item->enabled ? toolbar->icon_color : 
                                         venom_color_rgba(toolbar->icon_color.r, 
                                                          toolbar->icon_color.g,
                                                          toolbar->icon_color.b, 80);
                VenomPaint icon_paint = venom_paint_fill(icon_color);
                VenomF32 icon_x = x + toolbar->button_size / 2;
                VenomF32 icon_y = button_y + toolbar->button_size / 2;
                venom_canvas_draw_circle(canvas, icon_x, icon_y, toolbar->icon_size / 2 - 2, &icon_paint);
                break;
            }
            
            case VENOM_TOOLBAR_ITEM_SEPARATOR: {
                VenomF32 sep_x = x + item->width / 2;
                VenomPaint sep_paint = venom_paint_stroke(toolbar->separator_color, 1.0f);
                venom_canvas_draw_line(canvas, sep_x, bounds.y + 6, 
                                        sep_x, bounds.y + bounds.height - 6, &sep_paint);
                break;
            }
            
            case VENOM_TOOLBAR_ITEM_SPACER:
            case VENOM_TOOLBAR_ITEM_WIDGET:
                break;
        }
    }
}

static VenomI32 toolbar_hit_test(VenomToolbar* toolbar, VenomF32 x, VenomF32 y) {
    VenomF32 rel_x = x - toolbar->base.bounds.x;
    VenomF32 button_y = (toolbar->height - toolbar->button_size) / 2;
    VenomF32 rel_y = y - toolbar->base.bounds.y - button_y;
    
    if (rel_y < 0 || rel_y > toolbar->button_size) return -1;
    
    for (VenomU32 i = 0; i < toolbar->item_count; i++) {
        VenomToolbarItem* item = &toolbar->items[i];
        if (item->type != VENOM_TOOLBAR_ITEM_BUTTON && 
            item->type != VENOM_TOOLBAR_ITEM_TOGGLE) continue;
        
        if (rel_x >= item->x && rel_x < item->x + item->width) {
            return (VenomI32)i;
        }
    }
    return -1;
}

static VenomBool toolbar_on_event(VenomWidget* widget, const VenomEvent* event) {
    VenomToolbar* toolbar = (VenomToolbar*)widget;
    
    switch (event->type) {
        case VENOM_EVENT_MOUSE_MOVE: {
            VenomI32 index = toolbar_hit_test(toolbar, (VenomF32)event->mouse.x, 
                                               (VenomF32)event->mouse.y);
            if (index != toolbar->hover_index) {
                toolbar->hover_index = index;
                venom_widget_invalidate(widget);
            }
            return index >= 0;
        }
        
        case VENOM_EVENT_MOUSE_LEAVE:
            toolbar->hover_index = -1;
            toolbar->pressed_index = -1;
            venom_widget_invalidate(widget);
            return VENOM_TRUE;
            
        case VENOM_EVENT_MOUSE_BUTTON_DOWN: {
            if (event->mouse.button != VENOM_MOUSE_BUTTON_LEFT) return VENOM_FALSE;
            
            VenomI32 index = toolbar_hit_test(toolbar, (VenomF32)event->mouse.x,
                                               (VenomF32)event->mouse.y);
            if (index >= 0 && toolbar->items[index].enabled) {
                toolbar->pressed_index = index;
                venom_widget_invalidate(widget);
                return VENOM_TRUE;
            }
            return VENOM_FALSE;
        }
        
        case VENOM_EVENT_MOUSE_BUTTON_UP: {
            if (event->mouse.button != VENOM_MOUSE_BUTTON_LEFT) return VENOM_FALSE;
            
            VenomI32 pressed = toolbar->pressed_index;
            toolbar->pressed_index = -1;
            
            if (pressed >= 0) {
                VenomI32 index = toolbar_hit_test(toolbar, (VenomF32)event->mouse.x,
                                                   (VenomF32)event->mouse.y);
                if (index == pressed && toolbar->items[index].enabled) {
                    VenomToolbarItem* item = &toolbar->items[index];
                    
                    if (item->type == VENOM_TOOLBAR_ITEM_TOGGLE) {
                        item->toggled = !item->toggled;
                    }
                    
                    if (item->on_click) {
                        item->on_click(toolbar, (VenomU32)index, item->user_data);
                    }
                }
                venom_widget_invalidate(widget);
                return VENOM_TRUE;
            }
            return VENOM_FALSE;
        }
        
        default:
            return VENOM_FALSE;
    }
}
