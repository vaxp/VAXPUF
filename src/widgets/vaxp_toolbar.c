/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_toolbar.c - Toolbar Widget implementation
 */

#include "vaxp/widgets/vaxp_toolbar.h"
#include "vaxp/core/vaxp_memory.h"
#include "vaxp/graphics/vaxp_canvas.h"
#include <string.h>

/* ============================================================================
 * FORWARD DECLARATIONS
 * ============================================================================ */

static void toolbar_init(VaxpWidget* widget);
static void toolbar_destroy(VaxpWidget* widget);
static void toolbar_measure(VaxpWidget* widget, VaxpF32 available_width, VaxpF32 available_height,
                             VaxpF32* out_width, VaxpF32* out_height);
static void toolbar_layout(VaxpWidget* widget, VaxpRectF bounds);
static void toolbar_draw(VaxpWidget* widget, VaxpCanvas* canvas);
static VaxpBool toolbar_on_event(VaxpWidget* widget, const VaxpEvent* event);

/* ============================================================================
 * WIDGET CLASS
 * ============================================================================ */

const VaxpWidgetClass vaxp_toolbar_class = {
    .class_name = "VaxpToolbar",
    .instance_size = sizeof(VaxpToolbar),
    .parent_class = &vaxp_widget_class,
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
    char* copy = vaxp_alloc(len + 1);
    if (copy) memcpy(copy, s, len + 1);
    return copy;
}

static void str_free(char* s) {
    if (s) vaxp_free(s, strlen(s) + 1);
}

/* ============================================================================
 * LIFECYCLE
 * ============================================================================ */

static void toolbar_init(VaxpWidget* widget) {
    VaxpToolbar* toolbar = (VaxpToolbar*)widget;
    
    toolbar->items = NULL;
    toolbar->item_count = 0;
    toolbar->item_capacity = 0;
    toolbar->hover_index = -1;
    toolbar->pressed_index = -1;
    
    toolbar->button_style = VAXP_TOOLBAR_STYLE_ICON_ONLY;
    toolbar->background = vaxp_color_rgb(250, 250, 250);
    toolbar->button_hover = vaxp_color_rgba(0, 0, 0, 20);
    toolbar->button_pressed = vaxp_color_rgba(0, 0, 0, 40);
    toolbar->button_toggled = vaxp_color_rgba(100, 150, 255, 60);
    toolbar->icon_color = vaxp_color_rgb(50, 50, 50);
    toolbar->text_color = vaxp_color_rgb(50, 50, 50);
    toolbar->separator_color = vaxp_color_rgba(0, 0, 0, 30);
    toolbar->height = 40.0f;
    toolbar->button_size = 32.0f;
    toolbar->spacing = 2.0f;
    toolbar->padding_h = 8.0f;
    toolbar->corner_radius = 6.0f;
    toolbar->icon_size = 20.0f;
    
    widget->layout.min_height = toolbar->height;
}

static void toolbar_destroy(VaxpWidget* widget) {
    VaxpToolbar* toolbar = (VaxpToolbar*)widget;
    
    for (VaxpU32 i = 0; i < toolbar->item_count; i++) {
        VaxpToolbarItem* item = &toolbar->items[i];
        str_free(item->label);
        str_free(item->icon);
        str_free(item->tooltip);
    }
    
    if (toolbar->items) {
        vaxp_free(toolbar->items, toolbar->item_capacity * sizeof(VaxpToolbarItem));
    }
    
    vaxp_widget_class.destroy(widget);
}

VaxpResultPtr vaxp_toolbar_create(void) {
    return vaxp_widget_create(&vaxp_toolbar_class);
}

/* ============================================================================
 * ITEM MANAGEMENT
 * ============================================================================ */

static VaxpResult toolbar_grow(VaxpToolbar* toolbar) {
    VaxpU32 new_capacity = toolbar->item_capacity == 0 ? 16 : toolbar->item_capacity * 2;
    VaxpToolbarItem* new_items = vaxp_alloc(new_capacity * sizeof(VaxpToolbarItem));
    if (!new_items) {
        return VAXP_ERR_UNIT(VAXP_ERROR_OUT_OF_MEMORY);
    }
    
    if (toolbar->items) {
        memcpy(new_items, toolbar->items, toolbar->item_count * sizeof(VaxpToolbarItem));
        vaxp_free(toolbar->items, toolbar->item_capacity * sizeof(VaxpToolbarItem));
    }
    
    toolbar->items = new_items;
    toolbar->item_capacity = new_capacity;
    return VAXP_OK_UNIT();
}

VaxpI32 vaxp_toolbar_add_button(VaxpToolbar* toolbar,
                                   const char* icon,
                                   const char* label,
                                   const char* tooltip,
                                   VaxpToolbarCallback callback,
                                   void* user_data) {
    if (!toolbar) return -1;
    
    if (toolbar->item_count >= toolbar->item_capacity) {
        if (!toolbar_grow(toolbar).ok) return -1;
    }
    
    VaxpToolbarItem* item = &toolbar->items[toolbar->item_count];
    memset(item, 0, sizeof(VaxpToolbarItem));
    
    item->type = VAXP_TOOLBAR_ITEM_BUTTON;
    item->icon = str_dup(icon);
    item->label = str_dup(label);
    item->tooltip = str_dup(tooltip);
    item->enabled = VAXP_TRUE;
    item->toggled = VAXP_FALSE;
    item->on_click = callback;
    item->user_data = user_data;
    
    VaxpI32 index = (VaxpI32)toolbar->item_count;
    toolbar->item_count++;
    vaxp_widget_invalidate(&toolbar->base);
    
    return index;
}

VaxpI32 vaxp_toolbar_add_toggle(VaxpToolbar* toolbar,
                                   const char* icon,
                                   const char* label,
                                   const char* tooltip,
                                   VaxpBool initial_state,
                                   VaxpToolbarCallback callback,
                                   void* user_data) {
    if (!toolbar) return -1;
    
    if (toolbar->item_count >= toolbar->item_capacity) {
        if (!toolbar_grow(toolbar).ok) return -1;
    }
    
    VaxpToolbarItem* item = &toolbar->items[toolbar->item_count];
    memset(item, 0, sizeof(VaxpToolbarItem));
    
    item->type = VAXP_TOOLBAR_ITEM_TOGGLE;
    item->icon = str_dup(icon);
    item->label = str_dup(label);
    item->tooltip = str_dup(tooltip);
    item->enabled = VAXP_TRUE;
    item->toggled = initial_state;
    item->on_click = callback;
    item->user_data = user_data;
    
    VaxpI32 index = (VaxpI32)toolbar->item_count;
    toolbar->item_count++;
    vaxp_widget_invalidate(&toolbar->base);
    
    return index;
}

VaxpI32 vaxp_toolbar_add_separator(VaxpToolbar* toolbar) {
    if (!toolbar) return -1;
    
    if (toolbar->item_count >= toolbar->item_capacity) {
        if (!toolbar_grow(toolbar).ok) return -1;
    }
    
    VaxpToolbarItem* item = &toolbar->items[toolbar->item_count];
    memset(item, 0, sizeof(VaxpToolbarItem));
    
    item->type = VAXP_TOOLBAR_ITEM_SEPARATOR;
    item->enabled = VAXP_TRUE;
    
    VaxpI32 index = (VaxpI32)toolbar->item_count;
    toolbar->item_count++;
    vaxp_widget_invalidate(&toolbar->base);
    
    return index;
}

VaxpI32 vaxp_toolbar_add_spacer(VaxpToolbar* toolbar) {
    if (!toolbar) return -1;
    
    if (toolbar->item_count >= toolbar->item_capacity) {
        if (!toolbar_grow(toolbar).ok) return -1;
    }
    
    VaxpToolbarItem* item = &toolbar->items[toolbar->item_count];
    memset(item, 0, sizeof(VaxpToolbarItem));
    
    item->type = VAXP_TOOLBAR_ITEM_SPACER;
    item->enabled = VAXP_TRUE;
    
    VaxpI32 index = (VaxpI32)toolbar->item_count;
    toolbar->item_count++;
    vaxp_widget_invalidate(&toolbar->base);
    
    return index;
}

VaxpI32 vaxp_toolbar_add_widget(VaxpToolbar* toolbar, VaxpWidget* widget) {
    if (!toolbar || !widget) return -1;
    
    if (toolbar->item_count >= toolbar->item_capacity) {
        if (!toolbar_grow(toolbar).ok) return -1;
    }
    
    VaxpToolbarItem* item = &toolbar->items[toolbar->item_count];
    memset(item, 0, sizeof(VaxpToolbarItem));
    
    item->type = VAXP_TOOLBAR_ITEM_WIDGET;
    item->widget = widget;
    item->enabled = VAXP_TRUE;
    
    VaxpI32 index = (VaxpI32)toolbar->item_count;
    toolbar->item_count++;
    vaxp_widget_invalidate(&toolbar->base);
    
    return index;
}

VaxpResult vaxp_toolbar_remove_item(VaxpToolbar* toolbar, VaxpU32 index) {
    if (!toolbar || index >= toolbar->item_count) {
        return VAXP_ERR_UNIT(VAXP_ERROR_OUT_OF_BOUNDS);
    }
    
    VaxpToolbarItem* item = &toolbar->items[index];
    str_free(item->label);
    str_free(item->icon);
    str_free(item->tooltip);
    
    for (VaxpU32 i = index; i < toolbar->item_count - 1; i++) {
        toolbar->items[i] = toolbar->items[i + 1];
    }
    toolbar->item_count--;
    
    vaxp_widget_invalidate(&toolbar->base);
    return VAXP_OK_UNIT();
}

void vaxp_toolbar_clear(VaxpToolbar* toolbar) {
    if (!toolbar) return;
    
    for (VaxpU32 i = 0; i < toolbar->item_count; i++) {
        str_free(toolbar->items[i].label);
        str_free(toolbar->items[i].icon);
        str_free(toolbar->items[i].tooltip);
    }
    toolbar->item_count = 0;
    
    vaxp_widget_invalidate(&toolbar->base);
}

VaxpU32 vaxp_toolbar_get_count(const VaxpToolbar* toolbar) {
    return toolbar ? toolbar->item_count : 0;
}

/* ============================================================================
 * ITEM STATE
 * ============================================================================ */

void vaxp_toolbar_set_enabled(VaxpToolbar* toolbar, VaxpU32 index, VaxpBool enabled) {
    if (!toolbar || index >= toolbar->item_count) return;
    toolbar->items[index].enabled = enabled;
    vaxp_widget_invalidate(&toolbar->base);
}

VaxpBool vaxp_toolbar_get_toggled(const VaxpToolbar* toolbar, VaxpU32 index) {
    if (!toolbar || index >= toolbar->item_count) return VAXP_FALSE;
    return toolbar->items[index].toggled;
}

void vaxp_toolbar_set_toggled(VaxpToolbar* toolbar, VaxpU32 index, VaxpBool toggled) {
    if (!toolbar || index >= toolbar->item_count) return;
    toolbar->items[index].toggled = toggled;
    vaxp_widget_invalidate(&toolbar->base);
}

void vaxp_toolbar_set_tooltip(VaxpToolbar* toolbar, VaxpU32 index, const char* tooltip) {
    if (!toolbar || index >= toolbar->item_count) return;
    str_free(toolbar->items[index].tooltip);
    toolbar->items[index].tooltip = str_dup(tooltip);
}

/* ============================================================================
 * APPEARANCE
 * ============================================================================ */

void vaxp_toolbar_set_style(VaxpToolbar* toolbar, VaxpToolbarButtonStyle style) {
    if (!toolbar) return;
    toolbar->button_style = style;
    vaxp_widget_invalidate(&toolbar->base);
}

void vaxp_toolbar_set_background(VaxpToolbar* toolbar, VaxpColor color) {
    if (!toolbar) return;
    toolbar->background = color;
    vaxp_widget_invalidate(&toolbar->base);
}

void vaxp_toolbar_set_height(VaxpToolbar* toolbar, VaxpF32 height) {
    if (!toolbar) return;
    toolbar->height = height;
    toolbar->base.layout.min_height = height;
    vaxp_widget_invalidate(&toolbar->base);
}

void vaxp_toolbar_set_button_size(VaxpToolbar* toolbar, VaxpF32 size) {
    if (!toolbar) return;
    toolbar->button_size = size;
    vaxp_widget_invalidate(&toolbar->base);
}

void vaxp_toolbar_set_spacing(VaxpToolbar* toolbar, VaxpF32 spacing) {
    if (!toolbar) return;
    toolbar->spacing = spacing;
    vaxp_widget_invalidate(&toolbar->base);
}

void vaxp_toolbar_set_icon_size(VaxpToolbar* toolbar, VaxpF32 size) {
    if (!toolbar) return;
    toolbar->icon_size = size;
    vaxp_widget_invalidate(&toolbar->base);
}

/* ============================================================================
 * WIDGET CALLBACKS
 * ============================================================================ */

static void toolbar_measure(VaxpWidget* widget, VaxpF32 available_width, 
                             VaxpF32 available_height, VaxpF32* out_width, VaxpF32* out_height) {
    VaxpToolbar* toolbar = (VaxpToolbar*)widget;
    (void)available_height;
    
    VaxpF32 total_width = toolbar->padding_h * 2;
    VaxpU32 spacer_count = 0;
    
    for (VaxpU32 i = 0; i < toolbar->item_count; i++) {
        VaxpToolbarItem* item = &toolbar->items[i];
        
        switch (item->type) {
            case VAXP_TOOLBAR_ITEM_BUTTON:
            case VAXP_TOOLBAR_ITEM_TOGGLE:
                item->width = toolbar->button_size;
                total_width += item->width + toolbar->spacing;
                break;
                
            case VAXP_TOOLBAR_ITEM_SEPARATOR:
                item->width = 8.0f;
                total_width += item->width + toolbar->spacing;
                break;
                
            case VAXP_TOOLBAR_ITEM_SPACER:
                spacer_count++;
                item->width = 0;
                break;
                
            case VAXP_TOOLBAR_ITEM_WIDGET:
                if (item->widget) {
                    item->width = item->widget->layout.preferred_width > 0 ? 
                                  item->widget->layout.preferred_width : 100.0f;
                    total_width += item->width + toolbar->spacing;
                }
                break;
        }
    }
    
    if (spacer_count > 0 && available_width > total_width) {
        VaxpF32 spacer_width = (available_width - total_width) / spacer_count;
        for (VaxpU32 i = 0; i < toolbar->item_count; i++) {
            if (toolbar->items[i].type == VAXP_TOOLBAR_ITEM_SPACER) {
                toolbar->items[i].width = spacer_width;
            }
        }
    }
    
    *out_width = available_width > 0 ? available_width : total_width;
    *out_height = toolbar->height;
}

static void toolbar_layout(VaxpWidget* widget, VaxpRectF bounds) {
    VaxpToolbar* toolbar = (VaxpToolbar*)widget;
    widget->bounds = bounds;
    
    VaxpF32 x = toolbar->padding_h;
    for (VaxpU32 i = 0; i < toolbar->item_count; i++) {
        VaxpToolbarItem* item = &toolbar->items[i];
        item->x = x;
        x += item->width + toolbar->spacing;
    }
}

static void toolbar_draw(VaxpWidget* widget, VaxpCanvas* canvas) {
    VaxpToolbar* toolbar = (VaxpToolbar*)widget;
    VaxpRectF bounds = widget->bounds;
    
    VaxpPaint bg_paint = vaxp_paint_fill(toolbar->background);
    vaxp_canvas_draw_rect(canvas, bounds, &bg_paint);
    
    VaxpPaint border_paint = vaxp_paint_stroke(vaxp_color_rgba(0, 0, 0, 25), 1.0f);
    vaxp_canvas_draw_line(canvas, bounds.x, bounds.y + bounds.height - 0.5f,
                            bounds.x + bounds.width, bounds.y + bounds.height - 0.5f,
                            &border_paint);
    
    VaxpF32 button_y = bounds.y + (bounds.height - toolbar->button_size) / 2;
    
    for (VaxpU32 i = 0; i < toolbar->item_count; i++) {
        VaxpToolbarItem* item = &toolbar->items[i];
        VaxpF32 x = bounds.x + item->x;
        
        switch (item->type) {
            case VAXP_TOOLBAR_ITEM_BUTTON:
            case VAXP_TOOLBAR_ITEM_TOGGLE: {
                VaxpRectF btn_rect = { x, button_y, toolbar->button_size, toolbar->button_size };
                
                VaxpColor bg_color = toolbar->background;
                if (!item->enabled) {
                    /* Disabled */
                } else if ((VaxpI32)i == toolbar->pressed_index) {
                    bg_color = toolbar->button_pressed;
                } else if (item->toggled) {
                    bg_color = toolbar->button_toggled;
                } else if ((VaxpI32)i == toolbar->hover_index) {
                    bg_color = toolbar->button_hover;
                }
                
                if (bg_color.a > 0) {
                    VaxpPaint btn_paint = vaxp_paint_fill(bg_color);
                    vaxp_canvas_draw_rounded_rect(canvas, btn_rect, toolbar->corner_radius, &btn_paint);
                }
                
                VaxpColor icon_color = item->enabled ? toolbar->icon_color : 
                                         vaxp_color_rgba(toolbar->icon_color.r, 
                                                          toolbar->icon_color.g,
                                                          toolbar->icon_color.b, 80);
                VaxpPaint icon_paint = vaxp_paint_fill(icon_color);
                VaxpF32 icon_x = x + toolbar->button_size / 2;
                VaxpF32 icon_y = button_y + toolbar->button_size / 2;
                vaxp_canvas_draw_circle(canvas, icon_x, icon_y, toolbar->icon_size / 2 - 2, &icon_paint);
                break;
            }
            
            case VAXP_TOOLBAR_ITEM_SEPARATOR: {
                VaxpF32 sep_x = x + item->width / 2;
                VaxpPaint sep_paint = vaxp_paint_stroke(toolbar->separator_color, 1.0f);
                vaxp_canvas_draw_line(canvas, sep_x, bounds.y + 6, 
                                        sep_x, bounds.y + bounds.height - 6, &sep_paint);
                break;
            }
            
            case VAXP_TOOLBAR_ITEM_SPACER:
            case VAXP_TOOLBAR_ITEM_WIDGET:
                break;
        }
    }
}

static VaxpI32 toolbar_hit_test(VaxpToolbar* toolbar, VaxpF32 x, VaxpF32 y) {
    VaxpF32 rel_x = x - toolbar->base.bounds.x;
    VaxpF32 button_y = (toolbar->height - toolbar->button_size) / 2;
    VaxpF32 rel_y = y - toolbar->base.bounds.y - button_y;
    
    if (rel_y < 0 || rel_y > toolbar->button_size) return -1;
    
    for (VaxpU32 i = 0; i < toolbar->item_count; i++) {
        VaxpToolbarItem* item = &toolbar->items[i];
        if (item->type != VAXP_TOOLBAR_ITEM_BUTTON && 
            item->type != VAXP_TOOLBAR_ITEM_TOGGLE) continue;
        
        if (rel_x >= item->x && rel_x < item->x + item->width) {
            return (VaxpI32)i;
        }
    }
    return -1;
}

static VaxpBool toolbar_on_event(VaxpWidget* widget, const VaxpEvent* event) {
    VaxpToolbar* toolbar = (VaxpToolbar*)widget;
    
    switch (event->type) {
        case VAXP_EVENT_MOUSE_MOVE: {
            VaxpI32 index = toolbar_hit_test(toolbar, (VaxpF32)event->mouse.x, 
                                               (VaxpF32)event->mouse.y);
            if (index != toolbar->hover_index) {
                toolbar->hover_index = index;
                vaxp_widget_invalidate(widget);
            }
            return index >= 0;
        }
        
        case VAXP_EVENT_MOUSE_LEAVE:
            toolbar->hover_index = -1;
            toolbar->pressed_index = -1;
            vaxp_widget_invalidate(widget);
            return VAXP_TRUE;
            
        case VAXP_EVENT_MOUSE_BUTTON_DOWN: {
            if (event->mouse.button != VAXP_MOUSE_BUTTON_LEFT) return VAXP_FALSE;
            
            VaxpI32 index = toolbar_hit_test(toolbar, (VaxpF32)event->mouse.x,
                                               (VaxpF32)event->mouse.y);
            if (index >= 0 && toolbar->items[index].enabled) {
                toolbar->pressed_index = index;
                vaxp_widget_invalidate(widget);
                return VAXP_TRUE;
            }
            return VAXP_FALSE;
        }
        
        case VAXP_EVENT_MOUSE_BUTTON_UP: {
            if (event->mouse.button != VAXP_MOUSE_BUTTON_LEFT) return VAXP_FALSE;
            
            VaxpI32 pressed = toolbar->pressed_index;
            toolbar->pressed_index = -1;
            
            if (pressed >= 0) {
                VaxpI32 index = toolbar_hit_test(toolbar, (VaxpF32)event->mouse.x,
                                                   (VaxpF32)event->mouse.y);
                if (index == pressed && toolbar->items[index].enabled) {
                    VaxpToolbarItem* item = &toolbar->items[index];
                    
                    if (item->type == VAXP_TOOLBAR_ITEM_TOGGLE) {
                        item->toggled = !item->toggled;
                    }
                    
                    if (item->on_click) {
                        item->on_click(toolbar, (VaxpU32)index, item->user_data);
                    }
                }
                vaxp_widget_invalidate(widget);
                return VAXP_TRUE;
            }
            return VAXP_FALSE;
        }
        
        default:
            return VAXP_FALSE;
    }
}
