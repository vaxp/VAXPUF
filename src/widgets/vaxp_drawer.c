/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_drawer.c - Side Drawer Navigation implementation
 */

#include "vaxp/widgets/vaxp_drawer.h"
#include "vaxp/core/vaxp_memory.h"
#include "vaxp/graphics/vaxp_canvas.h"
#include <string.h>

/* Dimensions */
#define DRAWER_DEFAULT_WIDTH 280.0f
#define DRAWER_HEADER_HEIGHT 160.0f
#define DRAWER_ITEM_HEIGHT 48.0f
#define DRAWER_ITEM_PADDING 16.0f
#define DRAWER_DIVIDER_HEIGHT 1.0f
#define DRAWER_ICON_SIZE 24.0f

/* ============================================================================
 * FORWARD DECLARATIONS
 * ============================================================================ */

static void drawer_init(VaxpWidget* widget);
static void drawer_destroy(VaxpWidget* widget);
static void drawer_measure(VaxpWidget* widget, VaxpF32 aw, VaxpF32 ah,
                            VaxpF32* ow, VaxpF32* oh);
static void drawer_layout(VaxpWidget* widget, VaxpRectF bounds);
static void drawer_draw(VaxpWidget* widget, VaxpCanvas* canvas);
static VaxpBool drawer_on_event(VaxpWidget* widget, const VaxpEvent* event);

/* ============================================================================
 * CLASS DEFINITION
 * ============================================================================ */

const VaxpWidgetClass vaxp_drawer_class = {
    .class_name = "VaxpDrawer",
    .instance_size = sizeof(VaxpDrawer),
    .parent_class = &vaxp_widget_class,
    .init = drawer_init,
    .destroy = drawer_destroy,
    .measure = drawer_measure,
    .layout = drawer_layout,
    .draw = drawer_draw,
    .on_event = drawer_on_event,
    .on_state_changed = NULL,
};

/* ============================================================================
 * LIFECYCLE
 * ============================================================================ */

static void drawer_init(VaxpWidget* widget) {
    VaxpDrawer* drawer = (VaxpDrawer*)widget;
    
    drawer->header = NULL;
    drawer->items = NULL;
    drawer->item_count = 0;
    drawer->selected_index = -1;
    drawer->background = VAXP_COLOR_WHITE;
    drawer->selected_bg = vaxp_color_rgb(232, 240, 254);
    drawer->item_color = vaxp_color_rgb(60, 60, 70);
    drawer->selected_color = vaxp_color_rgb(60, 120, 220);
    drawer->width = DRAWER_DEFAULT_WIDTH;
    drawer->position = VAXP_DRAWER_LEFT;
    drawer->is_open = VAXP_FALSE;
    drawer->open_progress = 0.0f;
    drawer->on_item_tap = NULL;
    drawer->user_data = NULL;
}

static void drawer_destroy(VaxpWidget* widget) {
    VaxpDrawer* drawer = (VaxpDrawer*)widget;
    
    if (drawer->header) {
        vaxp_unref(drawer->header);
        drawer->header = NULL;
    }
    
    if (drawer->items) {
        vaxp_free(drawer->items, drawer->item_count * sizeof(VaxpDrawerItem));
        drawer->items = NULL;
    }
    drawer->item_count = 0;
    
    vaxp_widget_class.destroy(widget);
}

/* ============================================================================
 * LAYOUT
 * ============================================================================ */

static void drawer_measure(VaxpWidget* widget, VaxpF32 aw, VaxpF32 ah,
                            VaxpF32* ow, VaxpF32* oh) {
    VaxpDrawer* drawer = (VaxpDrawer*)widget;
    (void)aw;
    
    *ow = drawer->width;
    *oh = ah;
}

static void drawer_layout(VaxpWidget* widget, VaxpRectF bounds) {
    VaxpDrawer* drawer = (VaxpDrawer*)widget;
    widget->bounds = bounds;
    
    /* Layout header */
    if (drawer->header) {
        VaxpRectF header_bounds = {0, 0, drawer->width, DRAWER_HEADER_HEIGHT};
        vaxp_widget_layout(drawer->header, header_bounds);
    }
}

/* ============================================================================
 * DRAWING
 * ============================================================================ */

static void drawer_draw(VaxpWidget* widget, VaxpCanvas* canvas) {
    VaxpDrawer* drawer = (VaxpDrawer*)widget;
    VaxpRectF bounds = widget->bounds;
    
    /* Animate open/close */
    VaxpF32 target = drawer->is_open ? 1.0f : 0.0f;
    drawer->open_progress += (target - drawer->open_progress) * 0.15f;
    
    if (drawer->open_progress < 0.01f && !drawer->is_open) {
        return;  /* Fully closed, don't draw */
    }
    
    /* Draw scrim (overlay) */
    if (drawer->open_progress > 0) {
        VaxpColor scrim = vaxp_color_rgba(0, 0, 0, (VaxpU8)(drawer->open_progress * 100));
        VaxpPaint sp = vaxp_paint_fill(scrim);
        VaxpRectF full = {0, 0, bounds.width, bounds.height};
        vaxp_canvas_draw_rect(canvas, full, &sp);
    }
    
    /* Calculate drawer position */
    VaxpF32 offset = (1.0f - drawer->open_progress) * drawer->width;
    VaxpF32 drawer_x = drawer->position == VAXP_DRAWER_LEFT ? 
                         -offset : bounds.width - drawer->width + offset;
    
    /* Draw drawer background */
    VaxpRectF drawer_rect = {drawer_x, 0, drawer->width, bounds.height};
    VaxpPaint bg = vaxp_paint_fill(drawer->background);
    vaxp_canvas_draw_rect(canvas, drawer_rect, &bg);
    
    /* Draw shadow */
    for (VaxpF32 i = 0; i < 8; i++) {
        VaxpF32 sx = drawer->position == VAXP_DRAWER_LEFT ? 
                      drawer_x + drawer->width + i : drawer_x - i - 1;
        VaxpColor shadow = vaxp_color_rgba(0, 0, 0, (VaxpU8)((8 - i) * 4));
        VaxpRectF s = {sx, 0, 1, bounds.height};
        VaxpPaint shp = vaxp_paint_fill(shadow);
        vaxp_canvas_draw_rect(canvas, s, &shp);
    }
    
    /* Draw header */
    if (drawer->header) {
        vaxp_widget_draw(drawer->header, canvas);
    }
    
    /* Draw items */
    VaxpF32 y = drawer->header ? DRAWER_HEADER_HEIGHT : 8;
    
    for (VaxpU32 i = 0; i < drawer->item_count; i++) {
        VaxpDrawerItem* item = &drawer->items[i];
        
        if (item->is_divider) {
            /* Draw divider */
            VaxpRectF div_rect = {
                drawer_x + DRAWER_ITEM_PADDING,
                y + 8,
                drawer->width - DRAWER_ITEM_PADDING * 2,
                DRAWER_DIVIDER_HEIGHT
            };
            VaxpPaint dp = vaxp_paint_fill(vaxp_color_rgb(220, 220, 225));
            vaxp_canvas_draw_rect(canvas, div_rect, &dp);
            y += 17;
            continue;
        }
        
        VaxpBool is_selected = ((VaxpI32)i == drawer->selected_index);
        
        /* Draw selected background */
        if (is_selected) {
            VaxpRectF item_bg = {
                drawer_x + 8, y,
                drawer->width - 16, DRAWER_ITEM_HEIGHT
            };
            VaxpPaint sbg = vaxp_paint_fill(drawer->selected_bg);
            vaxp_canvas_draw_rounded_rect(canvas, item_bg, 8, &sbg);
        }
        
        /* Draw icon placeholder */
        VaxpF32 icon_x = drawer_x + DRAWER_ITEM_PADDING + 12;
        VaxpF32 icon_y = y + DRAWER_ITEM_HEIGHT / 2.0f;
        VaxpColor icon_color = is_selected ? drawer->selected_color : drawer->item_color;
        VaxpPaint icp = vaxp_paint_fill(icon_color);
        vaxp_canvas_draw_circle(canvas, icon_x, icon_y, 10, &icp);
        
        /* Draw label */
        if (item->label) {
            VaxpF32 label_x = drawer_x + DRAWER_ITEM_PADDING + 40;
            VaxpF32 label_y = y + DRAWER_ITEM_HEIGHT / 2.0f + 5;
            VaxpColor label_color = is_selected ? drawer->selected_color : drawer->item_color;
            VaxpPaint lp = vaxp_paint_fill(label_color);
            vaxp_canvas_draw_text(canvas, item->label, label_x, label_y, NULL, &lp);
        }
        
        y += DRAWER_ITEM_HEIGHT;
    }
    
    /* Request redraw for animation */
    if (drawer->open_progress != target) {
        widget->needs_redraw = VAXP_TRUE;
    }
}

/* ============================================================================
 * EVENTS
 * ============================================================================ */

static VaxpBool drawer_on_event(VaxpWidget* widget, const VaxpEvent* event) {
    VaxpDrawer* drawer = (VaxpDrawer*)widget;
    
    if (!drawer->is_open) return VAXP_FALSE;
    
    if (event->type == VAXP_EVENT_MOUSE_BUTTON_DOWN) {
        VaxpF32 mx = event->mouse.x;
        
        /* Check if click is on scrim (close drawer) */
        VaxpF32 drawer_x = drawer->position == VAXP_DRAWER_LEFT ? 0 : 
                            widget->bounds.width - drawer->width;
        VaxpF32 drawer_end = drawer_x + drawer->width;
        
        if ((drawer->position == VAXP_DRAWER_LEFT && mx > drawer_end) ||
            (drawer->position == VAXP_DRAWER_RIGHT && mx < drawer_x)) {
            vaxp_drawer_close(drawer);
            return VAXP_TRUE;
        }
        
        /* Check if click is on item */
        VaxpF32 y = drawer->header ? DRAWER_HEADER_HEIGHT : 8;
        
        for (VaxpU32 i = 0; i < drawer->item_count; i++) {
            if (drawer->items[i].is_divider) {
                y += 17;
                continue;
            }
            
            if (event->mouse.y >= y && event->mouse.y < y + DRAWER_ITEM_HEIGHT) {
                drawer->selected_index = (VaxpI32)i;
                
                if (drawer->on_item_tap) {
                    drawer->on_item_tap(drawer, i, drawer->user_data);
                }
                if (drawer->items[i].on_tap) {
                    drawer->items[i].on_tap(i, drawer->items[i].user_data);
                }
                
                vaxp_widget_invalidate(widget);
                return VAXP_TRUE;
            }
            
            y += DRAWER_ITEM_HEIGHT;
        }
    }
    
    return VAXP_FALSE;
}

/* ============================================================================
 * PUBLIC API
 * ============================================================================ */

VaxpResultPtr vaxp_drawer_create(void) {
    return vaxp_widget_create(&vaxp_drawer_class);
}

void vaxp_drawer_set_header(VaxpDrawer* drawer, VaxpWidget* header) {
    if (!drawer) return;
    
    if (drawer->header) {
        vaxp_unref(drawer->header);
    }
    drawer->header = header ? vaxp_ref(header) : NULL;
    vaxp_widget_invalidate_layout((VaxpWidget*)drawer);
}

void vaxp_drawer_set_items(VaxpDrawer* drawer, const VaxpDrawerItem* items, VaxpU32 count) {
    if (!drawer) return;
    
    if (drawer->items) {
        vaxp_free(drawer->items, drawer->item_count * sizeof(VaxpDrawerItem));
    }
    
    if (items && count > 0) {
        drawer->items = vaxp_alloc(count * sizeof(VaxpDrawerItem));
        if (drawer->items) {
            memcpy(drawer->items, items, count * sizeof(VaxpDrawerItem));
            drawer->item_count = count;
        }
    } else {
        drawer->items = NULL;
        drawer->item_count = 0;
    }
    
    vaxp_widget_invalidate((VaxpWidget*)drawer);
}

void vaxp_drawer_add_item(VaxpDrawer* drawer, const VaxpDrawerItem* item) {
    if (!drawer || !item) return;
    
    VaxpU32 new_count = drawer->item_count + 1;
    VaxpDrawerItem* new_items = vaxp_realloc(drawer->items,
        drawer->item_count * sizeof(VaxpDrawerItem),
        new_count * sizeof(VaxpDrawerItem));
    
    if (!new_items) return;
    
    drawer->items = new_items;
    memcpy(&drawer->items[drawer->item_count], item, sizeof(VaxpDrawerItem));
    drawer->item_count = new_count;
    
    vaxp_widget_invalidate((VaxpWidget*)drawer);
}

void vaxp_drawer_add_divider(VaxpDrawer* drawer) {
    VaxpDrawerItem divider = {
        .icon = NULL,
        .label = NULL,
        .is_divider = VAXP_TRUE,
        .selected = VAXP_FALSE,
        .on_tap = NULL,
        .user_data = NULL
    };
    vaxp_drawer_add_item(drawer, &divider);
}

void vaxp_drawer_set_selected(VaxpDrawer* drawer, VaxpI32 index) {
    if (!drawer) return;
    drawer->selected_index = index;
    vaxp_widget_invalidate((VaxpWidget*)drawer);
}

void vaxp_drawer_open(VaxpDrawer* drawer) {
    if (!drawer) return;
    drawer->is_open = VAXP_TRUE;
    vaxp_widget_invalidate((VaxpWidget*)drawer);
}

void vaxp_drawer_close(VaxpDrawer* drawer) {
    if (!drawer) return;
    drawer->is_open = VAXP_FALSE;
    vaxp_widget_invalidate((VaxpWidget*)drawer);
}

void vaxp_drawer_toggle(VaxpDrawer* drawer) {
    if (!drawer) return;
    drawer->is_open = !drawer->is_open;
    vaxp_widget_invalidate((VaxpWidget*)drawer);
}

VaxpBool vaxp_drawer_is_open(VaxpDrawer* drawer) {
    return drawer ? drawer->is_open : VAXP_FALSE;
}

void vaxp_drawer_set_width(VaxpDrawer* drawer, VaxpF32 width) {
    if (!drawer) return;
    drawer->width = width;
    vaxp_widget_invalidate_layout((VaxpWidget*)drawer);
}

void vaxp_drawer_set_position(VaxpDrawer* drawer, VaxpDrawerPosition position) {
    if (!drawer) return;
    drawer->position = position;
    vaxp_widget_invalidate((VaxpWidget*)drawer);
}

void vaxp_drawer_set_colors(VaxpDrawer* drawer,
                              VaxpColor background,
                              VaxpColor item_color,
                              VaxpColor selected_bg,
                              VaxpColor selected_color) {
    if (!drawer) return;
    drawer->background = background;
    drawer->item_color = item_color;
    drawer->selected_bg = selected_bg;
    drawer->selected_color = selected_color;
    vaxp_widget_invalidate((VaxpWidget*)drawer);
}

void vaxp_drawer_set_on_item_tap(VaxpDrawer* drawer,
                                   void (*callback)(VaxpDrawer*, VaxpU32, void*),
                                   void* user_data) {
    if (!drawer) return;
    drawer->on_item_tap = callback;
    drawer->user_data = user_data;
}
