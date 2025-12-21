/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_bottom_nav.c - Bottom Navigation Widget implementation
 */

#include "venom/widgets/venom_bottom_nav.h"
#include "venom/core/venom_memory.h"
#include "venom/graphics/venom_canvas.h"
#include <string.h>

/* Dimensions */
#define BOTTOM_NAV_HEIGHT 56.0f
#define BOTTOM_NAV_ICON_SIZE 24.0f
#define BOTTOM_NAV_LABEL_SIZE 12.0f
#define BOTTOM_NAV_INDICATOR_HEIGHT 3.0f

/* ============================================================================
 * FORWARD DECLARATIONS
 * ============================================================================ */

static void bottom_nav_init(VenomWidget* widget);
static void bottom_nav_destroy(VenomWidget* widget);
static void bottom_nav_measure(VenomWidget* widget, VenomF32 aw, VenomF32 ah,
                                VenomF32* ow, VenomF32* oh);
static void bottom_nav_layout(VenomWidget* widget, VenomRectF bounds);
static void bottom_nav_draw(VenomWidget* widget, VenomCanvas* canvas);
static VenomBool bottom_nav_on_event(VenomWidget* widget, const VenomEvent* event);

/* ============================================================================
 * CLASS DEFINITION
 * ============================================================================ */

const VenomWidgetClass venom_bottom_nav_class = {
    .class_name = "VenomBottomNav",
    .instance_size = sizeof(VenomBottomNav),
    .parent_class = &venom_widget_class,
    .init = bottom_nav_init,
    .destroy = bottom_nav_destroy,
    .measure = bottom_nav_measure,
    .layout = bottom_nav_layout,
    .draw = bottom_nav_draw,
    .on_event = bottom_nav_on_event,
    .on_state_changed = NULL,
};

/* ============================================================================
 * LIFECYCLE
 * ============================================================================ */

static void bottom_nav_init(VenomWidget* widget) {
    VenomBottomNav* nav = (VenomBottomNav*)widget;
    
    nav->items = NULL;
    nav->item_count = 0;
    nav->selected = 0;
    nav->background = VENOM_COLOR_WHITE;
    nav->active_color = venom_color_rgb(60, 120, 220);
    nav->inactive_color = venom_color_rgb(120, 120, 130);
    nav->style = VENOM_NAV_FIXED;
    nav->height = BOTTOM_NAV_HEIGHT;
    nav->show_labels = VENOM_TRUE;
    nav->elevation = 8.0f;
    nav->on_change = NULL;
    nav->user_data = NULL;
    nav->indicator_x = 0;
    nav->indicator_target = 0;
}

static void bottom_nav_destroy(VenomWidget* widget) {
    VenomBottomNav* nav = (VenomBottomNav*)widget;
    
    if (nav->items) {
        venom_free(nav->items, nav->item_count * sizeof(VenomNavItem));
        nav->items = NULL;
    }
    nav->item_count = 0;
    
    venom_widget_class.destroy(widget);
}

/* ============================================================================
 * LAYOUT
 * ============================================================================ */

static void bottom_nav_measure(VenomWidget* widget, VenomF32 aw, VenomF32 ah,
                                VenomF32* ow, VenomF32* oh) {
    VenomBottomNav* nav = (VenomBottomNav*)widget;
    (void)ah;
    
    *ow = aw;
    *oh = nav->height;
}

static void bottom_nav_layout(VenomWidget* widget, VenomRectF bounds) {
    widget->bounds = bounds;
}

/* ============================================================================
 * DRAWING
 * ============================================================================ */

static void bottom_nav_draw(VenomWidget* widget, VenomCanvas* canvas) {
    VenomBottomNav* nav = (VenomBottomNav*)widget;
    VenomRectF bounds = { 0, 0, widget->bounds.width, widget->bounds.height };
    
    if (nav->item_count == 0) return;
    
    /* Draw shadow (top) */
    if (nav->elevation > 0) {
        for (VenomF32 i = 0; i < nav->elevation; i += 1.0f) {
            VenomColor shadow = venom_color_rgba(0, 0, 0, (VenomU8)((nav->elevation - i) * 4));
            VenomRectF shadow_rect = {
                bounds.x, bounds.y - i - 1,
                bounds.width, 1
            };
            VenomPaint sp = venom_paint_fill(shadow);
            venom_canvas_draw_rect(canvas, shadow_rect, &sp);
        }
    }
    
    /* Draw background */
    VenomPaint bg = venom_paint_fill(nav->background);
    venom_canvas_draw_rect(canvas, bounds, &bg);
    
    /* Calculate item width */
    VenomF32 item_width = bounds.width / (VenomF32)nav->item_count;
    
    /* Animate indicator */
    nav->indicator_target = nav->selected * item_width + item_width / 2.0f;
    nav->indicator_x += (nav->indicator_target - nav->indicator_x) * 0.2f;
    
    /* Draw indicator */
    VenomRectF indicator = {
        bounds.x + nav->indicator_x - item_width / 3.0f,
        bounds.y,
        item_width * 2.0f / 3.0f,
        BOTTOM_NAV_INDICATOR_HEIGHT
    };
    VenomPaint ip = venom_paint_fill(nav->active_color);
    venom_canvas_draw_rounded_rect(canvas, indicator, 2, &ip);
    
    /* Draw items */
    for (VenomU32 i = 0; i < nav->item_count; i++) {
        VenomF32 item_x = bounds.x + i * item_width;
        VenomF32 center_x = item_x + item_width / 2.0f;
        VenomBool is_selected = ((VenomI32)i == nav->selected);
        VenomBool is_enabled = nav->items[i].enabled;
        
        VenomColor color = is_selected ? nav->active_color : 
                           (is_enabled ? nav->inactive_color : 
                            venom_color_rgba(120, 120, 130, 100));
        
        /* Draw icon placeholder (circle) */
        VenomF32 icon_y = bounds.y + (nav->show_labels ? 14 : bounds.height / 2.0f);
        VenomPaint icp = venom_paint_fill(color);
        venom_canvas_draw_circle(canvas, center_x, icon_y, 
                                  is_selected ? 14 : 12, &icp);
        
        /* Draw label */
        if (nav->show_labels || (nav->style == VENOM_NAV_SHIFTING && is_selected)) {
            VenomF32 label_y = bounds.y + bounds.height - 8;
            VenomPaint lp = venom_paint_fill(color);
            
            if (nav->items[i].label) {
                venom_canvas_draw_text(canvas, nav->items[i].label, 
                                       center_x, label_y, NULL, &lp);
            }
        }
    }
    
    /* Request redraw for animation */
    if (nav->indicator_x != nav->indicator_target) {
        widget->needs_redraw = VENOM_TRUE;
    }
}

/* ============================================================================
 * EVENTS
 * ============================================================================ */

static VenomBool bottom_nav_on_event(VenomWidget* widget, const VenomEvent* event) {
    VenomBottomNav* nav = (VenomBottomNav*)widget;
    
    if (event->type == VENOM_EVENT_MOUSE_BUTTON_DOWN && nav->item_count > 0) {
        VenomF32 item_width = widget->bounds.width / (VenomF32)nav->item_count;
        VenomF32 local_x = event->mouse.x - widget->bounds.x;
        
        VenomI32 clicked = (VenomI32)(local_x / item_width);
        if (clicked >= 0 && clicked < (VenomI32)nav->item_count) {
            if (nav->items[clicked].enabled && clicked != nav->selected) {
                nav->selected = clicked;
                
                if (nav->on_change) {
                    nav->on_change(clicked, nav->user_data);
                }
                
                venom_widget_invalidate(widget);
                return VENOM_TRUE;
            }
        }
    }
    
    return VENOM_FALSE;
}

/* ============================================================================
 * PUBLIC API
 * ============================================================================ */

VenomResultPtr venom_bottom_nav_create(const VenomNavItem* items, VenomU32 count) {
    if (!items || count == 0) {
        return VENOM_ERR_PTR(VENOM_ERROR_INVALID_STATE);
    }
    
    VenomResultPtr result = venom_widget_create(&venom_bottom_nav_class);
    if (!result.ok) return result;
    
    VenomBottomNav* nav = (VenomBottomNav*)result.value;
    
    /* Copy items */
    nav->items = venom_alloc(count * sizeof(VenomNavItem));
    if (!nav->items) {
        venom_unref(nav);
        return VENOM_ERR_PTR(VENOM_ERROR_OUT_OF_MEMORY);
    }
    
    memcpy(nav->items, items, count * sizeof(VenomNavItem));
    nav->item_count = count;
    
    /* Enable all by default */
    for (VenomU32 i = 0; i < count; i++) {
        nav->items[i].enabled = VENOM_TRUE;
    }
    
    return result;
}

void venom_bottom_nav_set_selected(VenomBottomNav* nav, VenomI32 index) {
    if (!nav || index < 0 || (VenomU32)index >= nav->item_count) return;
    nav->selected = index;
    venom_widget_invalidate((VenomWidget*)nav);
}

VenomI32 venom_bottom_nav_get_selected(VenomBottomNav* nav) {
    return nav ? nav->selected : -1;
}

void venom_bottom_nav_set_on_change(VenomBottomNav* nav, 
                                     VenomNavCallback callback, 
                                     void* user_data) {
    if (!nav) return;
    nav->on_change = callback;
    nav->user_data = user_data;
}

void venom_bottom_nav_set_style(VenomBottomNav* nav, VenomNavStyle style) {
    if (!nav) return;
    nav->style = style;
    venom_widget_invalidate((VenomWidget*)nav);
}

void venom_bottom_nav_set_colors(VenomBottomNav* nav,
                                  VenomColor background,
                                  VenomColor active,
                                  VenomColor inactive) {
    if (!nav) return;
    nav->background = background;
    nav->active_color = active;
    nav->inactive_color = inactive;
    venom_widget_invalidate((VenomWidget*)nav);
}

void venom_bottom_nav_set_show_labels(VenomBottomNav* nav, VenomBool show) {
    if (!nav) return;
    nav->show_labels = show;
    venom_widget_invalidate((VenomWidget*)nav);
}

void venom_bottom_nav_set_elevation(VenomBottomNav* nav, VenomF32 elevation) {
    if (!nav) return;
    nav->elevation = elevation;
    venom_widget_invalidate((VenomWidget*)nav);
}

void venom_bottom_nav_set_item_enabled(VenomBottomNav* nav, VenomU32 index, VenomBool enabled) {
    if (!nav || index >= nav->item_count) return;
    nav->items[index].enabled = enabled;
    venom_widget_invalidate((VenomWidget*)nav);
}
