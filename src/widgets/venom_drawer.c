/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_drawer.c - Side Drawer Navigation implementation
 */

#include "venom/widgets/venom_drawer.h"
#include "venom/core/venom_memory.h"
#include "venom/graphics/venom_canvas.h"
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

static void drawer_init(VenomWidget* widget);
static void drawer_destroy(VenomWidget* widget);
static void drawer_measure(VenomWidget* widget, VenomF32 aw, VenomF32 ah,
                            VenomF32* ow, VenomF32* oh);
static void drawer_layout(VenomWidget* widget, VenomRectF bounds);
static void drawer_draw(VenomWidget* widget, VenomCanvas* canvas);
static VenomBool drawer_on_event(VenomWidget* widget, const VenomEvent* event);

/* ============================================================================
 * CLASS DEFINITION
 * ============================================================================ */

const VenomWidgetClass venom_drawer_class = {
    .class_name = "VenomDrawer",
    .instance_size = sizeof(VenomDrawer),
    .parent_class = &venom_widget_class,
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

static void drawer_init(VenomWidget* widget) {
    VenomDrawer* drawer = (VenomDrawer*)widget;
    
    drawer->header = NULL;
    drawer->items = NULL;
    drawer->item_count = 0;
    drawer->selected_index = -1;
    drawer->background = VENOM_COLOR_WHITE;
    drawer->selected_bg = venom_color_rgb(232, 240, 254);
    drawer->item_color = venom_color_rgb(60, 60, 70);
    drawer->selected_color = venom_color_rgb(60, 120, 220);
    drawer->width = DRAWER_DEFAULT_WIDTH;
    drawer->position = VENOM_DRAWER_LEFT;
    drawer->is_open = VENOM_FALSE;
    drawer->open_progress = 0.0f;
    drawer->on_item_tap = NULL;
    drawer->user_data = NULL;
}

static void drawer_destroy(VenomWidget* widget) {
    VenomDrawer* drawer = (VenomDrawer*)widget;
    
    if (drawer->header) {
        venom_unref(drawer->header);
        drawer->header = NULL;
    }
    
    if (drawer->items) {
        venom_free(drawer->items, drawer->item_count * sizeof(VenomDrawerItem));
        drawer->items = NULL;
    }
    drawer->item_count = 0;
    
    venom_widget_class.destroy(widget);
}

/* ============================================================================
 * LAYOUT
 * ============================================================================ */

static void drawer_measure(VenomWidget* widget, VenomF32 aw, VenomF32 ah,
                            VenomF32* ow, VenomF32* oh) {
    VenomDrawer* drawer = (VenomDrawer*)widget;
    (void)aw;
    
    *ow = drawer->width;
    *oh = ah;
}

static void drawer_layout(VenomWidget* widget, VenomRectF bounds) {
    VenomDrawer* drawer = (VenomDrawer*)widget;
    widget->bounds = bounds;
    
    /* Layout header */
    if (drawer->header) {
        VenomRectF header_bounds = {0, 0, drawer->width, DRAWER_HEADER_HEIGHT};
        venom_widget_layout(drawer->header, header_bounds);
    }
}

/* ============================================================================
 * DRAWING
 * ============================================================================ */

static void drawer_draw(VenomWidget* widget, VenomCanvas* canvas) {
    VenomDrawer* drawer = (VenomDrawer*)widget;
    VenomRectF bounds = widget->bounds;
    
    /* Animate open/close */
    VenomF32 target = drawer->is_open ? 1.0f : 0.0f;
    drawer->open_progress += (target - drawer->open_progress) * 0.15f;
    
    if (drawer->open_progress < 0.01f && !drawer->is_open) {
        return;  /* Fully closed, don't draw */
    }
    
    /* Draw scrim (overlay) */
    if (drawer->open_progress > 0) {
        VenomColor scrim = venom_color_rgba(0, 0, 0, (VenomU8)(drawer->open_progress * 100));
        VenomPaint sp = venom_paint_fill(scrim);
        VenomRectF full = {0, 0, bounds.width, bounds.height};
        venom_canvas_draw_rect(canvas, full, &sp);
    }
    
    /* Calculate drawer position */
    VenomF32 offset = (1.0f - drawer->open_progress) * drawer->width;
    VenomF32 drawer_x = drawer->position == VENOM_DRAWER_LEFT ? 
                         -offset : bounds.width - drawer->width + offset;
    
    /* Draw drawer background */
    VenomRectF drawer_rect = {drawer_x, 0, drawer->width, bounds.height};
    VenomPaint bg = venom_paint_fill(drawer->background);
    venom_canvas_draw_rect(canvas, drawer_rect, &bg);
    
    /* Draw shadow */
    for (VenomF32 i = 0; i < 8; i++) {
        VenomF32 sx = drawer->position == VENOM_DRAWER_LEFT ? 
                      drawer_x + drawer->width + i : drawer_x - i - 1;
        VenomColor shadow = venom_color_rgba(0, 0, 0, (VenomU8)((8 - i) * 4));
        VenomRectF s = {sx, 0, 1, bounds.height};
        VenomPaint shp = venom_paint_fill(shadow);
        venom_canvas_draw_rect(canvas, s, &shp);
    }
    
    /* Draw header */
    if (drawer->header) {
        venom_widget_draw(drawer->header, canvas);
    }
    
    /* Draw items */
    VenomF32 y = drawer->header ? DRAWER_HEADER_HEIGHT : 8;
    
    for (VenomU32 i = 0; i < drawer->item_count; i++) {
        VenomDrawerItem* item = &drawer->items[i];
        
        if (item->is_divider) {
            /* Draw divider */
            VenomRectF div_rect = {
                drawer_x + DRAWER_ITEM_PADDING,
                y + 8,
                drawer->width - DRAWER_ITEM_PADDING * 2,
                DRAWER_DIVIDER_HEIGHT
            };
            VenomPaint dp = venom_paint_fill(venom_color_rgb(220, 220, 225));
            venom_canvas_draw_rect(canvas, div_rect, &dp);
            y += 17;
            continue;
        }
        
        VenomBool is_selected = ((VenomI32)i == drawer->selected_index);
        
        /* Draw selected background */
        if (is_selected) {
            VenomRectF item_bg = {
                drawer_x + 8, y,
                drawer->width - 16, DRAWER_ITEM_HEIGHT
            };
            VenomPaint sbg = venom_paint_fill(drawer->selected_bg);
            venom_canvas_draw_rounded_rect(canvas, item_bg, 8, &sbg);
        }
        
        /* Draw icon placeholder */
        VenomF32 icon_x = drawer_x + DRAWER_ITEM_PADDING + 12;
        VenomF32 icon_y = y + DRAWER_ITEM_HEIGHT / 2.0f;
        VenomColor icon_color = is_selected ? drawer->selected_color : drawer->item_color;
        VenomPaint icp = venom_paint_fill(icon_color);
        venom_canvas_draw_circle(canvas, icon_x, icon_y, 10, &icp);
        
        /* Draw label */
        if (item->label) {
            VenomF32 label_x = drawer_x + DRAWER_ITEM_PADDING + 40;
            VenomF32 label_y = y + DRAWER_ITEM_HEIGHT / 2.0f + 5;
            VenomColor label_color = is_selected ? drawer->selected_color : drawer->item_color;
            VenomPaint lp = venom_paint_fill(label_color);
            venom_canvas_draw_text(canvas, item->label, label_x, label_y, NULL, &lp);
        }
        
        y += DRAWER_ITEM_HEIGHT;
    }
    
    /* Request redraw for animation */
    if (drawer->open_progress != target) {
        widget->needs_redraw = VENOM_TRUE;
    }
}

/* ============================================================================
 * EVENTS
 * ============================================================================ */

static VenomBool drawer_on_event(VenomWidget* widget, const VenomEvent* event) {
    VenomDrawer* drawer = (VenomDrawer*)widget;
    
    if (!drawer->is_open) return VENOM_FALSE;
    
    if (event->type == VENOM_EVENT_MOUSE_BUTTON_DOWN) {
        VenomF32 mx = event->mouse.x;
        
        /* Check if click is on scrim (close drawer) */
        VenomF32 drawer_x = drawer->position == VENOM_DRAWER_LEFT ? 0 : 
                            widget->bounds.width - drawer->width;
        VenomF32 drawer_end = drawer_x + drawer->width;
        
        if ((drawer->position == VENOM_DRAWER_LEFT && mx > drawer_end) ||
            (drawer->position == VENOM_DRAWER_RIGHT && mx < drawer_x)) {
            venom_drawer_close(drawer);
            return VENOM_TRUE;
        }
        
        /* Check if click is on item */
        VenomF32 y = drawer->header ? DRAWER_HEADER_HEIGHT : 8;
        
        for (VenomU32 i = 0; i < drawer->item_count; i++) {
            if (drawer->items[i].is_divider) {
                y += 17;
                continue;
            }
            
            if (event->mouse.y >= y && event->mouse.y < y + DRAWER_ITEM_HEIGHT) {
                drawer->selected_index = (VenomI32)i;
                
                if (drawer->on_item_tap) {
                    drawer->on_item_tap(drawer, i, drawer->user_data);
                }
                if (drawer->items[i].on_tap) {
                    drawer->items[i].on_tap(i, drawer->items[i].user_data);
                }
                
                venom_widget_invalidate(widget);
                return VENOM_TRUE;
            }
            
            y += DRAWER_ITEM_HEIGHT;
        }
    }
    
    return VENOM_FALSE;
}

/* ============================================================================
 * PUBLIC API
 * ============================================================================ */

VenomResultPtr venom_drawer_create(void) {
    return venom_widget_create(&venom_drawer_class);
}

void venom_drawer_set_header(VenomDrawer* drawer, VenomWidget* header) {
    if (!drawer) return;
    
    if (drawer->header) {
        venom_unref(drawer->header);
    }
    drawer->header = header ? venom_ref(header) : NULL;
    venom_widget_invalidate_layout((VenomWidget*)drawer);
}

void venom_drawer_set_items(VenomDrawer* drawer, const VenomDrawerItem* items, VenomU32 count) {
    if (!drawer) return;
    
    if (drawer->items) {
        venom_free(drawer->items, drawer->item_count * sizeof(VenomDrawerItem));
    }
    
    if (items && count > 0) {
        drawer->items = venom_alloc(count * sizeof(VenomDrawerItem));
        if (drawer->items) {
            memcpy(drawer->items, items, count * sizeof(VenomDrawerItem));
            drawer->item_count = count;
        }
    } else {
        drawer->items = NULL;
        drawer->item_count = 0;
    }
    
    venom_widget_invalidate((VenomWidget*)drawer);
}

void venom_drawer_add_item(VenomDrawer* drawer, const VenomDrawerItem* item) {
    if (!drawer || !item) return;
    
    VenomU32 new_count = drawer->item_count + 1;
    VenomDrawerItem* new_items = venom_realloc(drawer->items,
        drawer->item_count * sizeof(VenomDrawerItem),
        new_count * sizeof(VenomDrawerItem));
    
    if (!new_items) return;
    
    drawer->items = new_items;
    memcpy(&drawer->items[drawer->item_count], item, sizeof(VenomDrawerItem));
    drawer->item_count = new_count;
    
    venom_widget_invalidate((VenomWidget*)drawer);
}

void venom_drawer_add_divider(VenomDrawer* drawer) {
    VenomDrawerItem divider = {
        .icon = NULL,
        .label = NULL,
        .is_divider = VENOM_TRUE,
        .selected = VENOM_FALSE,
        .on_tap = NULL,
        .user_data = NULL
    };
    venom_drawer_add_item(drawer, &divider);
}

void venom_drawer_set_selected(VenomDrawer* drawer, VenomI32 index) {
    if (!drawer) return;
    drawer->selected_index = index;
    venom_widget_invalidate((VenomWidget*)drawer);
}

void venom_drawer_open(VenomDrawer* drawer) {
    if (!drawer) return;
    drawer->is_open = VENOM_TRUE;
    venom_widget_invalidate((VenomWidget*)drawer);
}

void venom_drawer_close(VenomDrawer* drawer) {
    if (!drawer) return;
    drawer->is_open = VENOM_FALSE;
    venom_widget_invalidate((VenomWidget*)drawer);
}

void venom_drawer_toggle(VenomDrawer* drawer) {
    if (!drawer) return;
    drawer->is_open = !drawer->is_open;
    venom_widget_invalidate((VenomWidget*)drawer);
}

VenomBool venom_drawer_is_open(VenomDrawer* drawer) {
    return drawer ? drawer->is_open : VENOM_FALSE;
}

void venom_drawer_set_width(VenomDrawer* drawer, VenomF32 width) {
    if (!drawer) return;
    drawer->width = width;
    venom_widget_invalidate_layout((VenomWidget*)drawer);
}

void venom_drawer_set_position(VenomDrawer* drawer, VenomDrawerPosition position) {
    if (!drawer) return;
    drawer->position = position;
    venom_widget_invalidate((VenomWidget*)drawer);
}

void venom_drawer_set_colors(VenomDrawer* drawer,
                              VenomColor background,
                              VenomColor item_color,
                              VenomColor selected_bg,
                              VenomColor selected_color) {
    if (!drawer) return;
    drawer->background = background;
    drawer->item_color = item_color;
    drawer->selected_bg = selected_bg;
    drawer->selected_color = selected_color;
    venom_widget_invalidate((VenomWidget*)drawer);
}

void venom_drawer_set_on_item_tap(VenomDrawer* drawer,
                                   void (*callback)(VenomDrawer*, VenomU32, void*),
                                   void* user_data) {
    if (!drawer) return;
    drawer->on_item_tap = callback;
    drawer->user_data = user_data;
}
