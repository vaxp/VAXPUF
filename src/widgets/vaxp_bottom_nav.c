/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_bottom_nav.c - Bottom Navigation Widget implementation
 */

#include "vaxp/widgets/vaxp_bottom_nav.h"
#include "vaxp/core/vaxp_memory.h"
#include "vaxp/graphics/vaxp_canvas.h"
#include <string.h>

/* Dimensions */
#define BOTTOM_NAV_HEIGHT 56.0f
#define BOTTOM_NAV_ICON_SIZE 24.0f
#define BOTTOM_NAV_LABEL_SIZE 12.0f
#define BOTTOM_NAV_INDICATOR_HEIGHT 3.0f

/* ============================================================================
 * FORWARD DECLARATIONS
 * ============================================================================ */

static void bottom_nav_init(VaxpWidget* widget);
static void bottom_nav_destroy(VaxpWidget* widget);
static void bottom_nav_measure(VaxpWidget* widget, VaxpF32 aw, VaxpF32 ah,
                                VaxpF32* ow, VaxpF32* oh);
static void bottom_nav_layout(VaxpWidget* widget, VaxpRectF bounds);
static void bottom_nav_draw(VaxpWidget* widget, VaxpCanvas* canvas);
static VaxpBool bottom_nav_on_event(VaxpWidget* widget, const VaxpEvent* event);

/* ============================================================================
 * CLASS DEFINITION
 * ============================================================================ */

const VaxpWidgetClass vaxp_bottom_nav_class = {
    .class_name = "VaxpBottomNav",
    .instance_size = sizeof(VaxpBottomNav),
    .parent_class = &vaxp_widget_class,
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

static void bottom_nav_init(VaxpWidget* widget) {
    VaxpBottomNav* nav = (VaxpBottomNav*)widget;
    
    nav->items = NULL;
    nav->item_count = 0;
    nav->selected = 0;
    nav->background = VAXP_COLOR_WHITE;
    nav->active_color = vaxp_color_rgb(60, 120, 220);
    nav->inactive_color = vaxp_color_rgb(120, 120, 130);
    nav->style = VAXP_NAV_FIXED;
    nav->height = BOTTOM_NAV_HEIGHT;
    nav->show_labels = VAXP_TRUE;
    nav->elevation = 8.0f;
    nav->on_change = NULL;
    nav->user_data = NULL;
    nav->indicator_x = 0;
    nav->indicator_target = 0;
}

static void bottom_nav_destroy(VaxpWidget* widget) {
    VaxpBottomNav* nav = (VaxpBottomNav*)widget;
    
    if (nav->items) {
        vaxp_free(nav->items, nav->item_count * sizeof(VaxpNavItem));
        nav->items = NULL;
    }
    nav->item_count = 0;
    
    vaxp_widget_class.destroy(widget);
}

/* ============================================================================
 * LAYOUT
 * ============================================================================ */

static void bottom_nav_measure(VaxpWidget* widget, VaxpF32 aw, VaxpF32 ah,
                                VaxpF32* ow, VaxpF32* oh) {
    VaxpBottomNav* nav = (VaxpBottomNav*)widget;
    (void)ah;
    
    *ow = aw;
    *oh = nav->height;
}

static void bottom_nav_layout(VaxpWidget* widget, VaxpRectF bounds) {
    widget->bounds = bounds;
}

/* ============================================================================
 * DRAWING
 * ============================================================================ */

static void bottom_nav_draw(VaxpWidget* widget, VaxpCanvas* canvas) {
    VaxpBottomNav* nav = (VaxpBottomNav*)widget;
    VaxpRectF bounds = { 0, 0, widget->bounds.width, widget->bounds.height };
    
    if (nav->item_count == 0) return;
    
    /* Draw shadow (top) */
    if (nav->elevation > 0) {
        for (VaxpF32 i = 0; i < nav->elevation; i += 1.0f) {
            VaxpColor shadow = vaxp_color_rgba(0, 0, 0, (VaxpU8)((nav->elevation - i) * 4));
            VaxpRectF shadow_rect = {
                bounds.x, bounds.y - i - 1,
                bounds.width, 1
            };
            VaxpPaint sp = vaxp_paint_fill(shadow);
            vaxp_canvas_draw_rect(canvas, shadow_rect, &sp);
        }
    }
    
    /* Draw background */
    VaxpPaint bg = vaxp_paint_fill(nav->background);
    vaxp_canvas_draw_rect(canvas, bounds, &bg);
    
    /* Calculate item width */
    VaxpF32 item_width = bounds.width / (VaxpF32)nav->item_count;
    
    /* Animate indicator */
    nav->indicator_target = nav->selected * item_width + item_width / 2.0f;
    nav->indicator_x += (nav->indicator_target - nav->indicator_x) * 0.2f;
    
    /* Draw indicator */
    VaxpRectF indicator = {
        bounds.x + nav->indicator_x - item_width / 3.0f,
        bounds.y,
        item_width * 2.0f / 3.0f,
        BOTTOM_NAV_INDICATOR_HEIGHT
    };
    VaxpPaint ip = vaxp_paint_fill(nav->active_color);
    vaxp_canvas_draw_rounded_rect(canvas, indicator, 2, &ip);
    
    /* Draw items */
    for (VaxpU32 i = 0; i < nav->item_count; i++) {
        VaxpF32 item_x = bounds.x + i * item_width;
        VaxpF32 center_x = item_x + item_width / 2.0f;
        VaxpBool is_selected = ((VaxpI32)i == nav->selected);
        VaxpBool is_enabled = nav->items[i].enabled;
        
        VaxpColor color = is_selected ? nav->active_color : 
                           (is_enabled ? nav->inactive_color : 
                            vaxp_color_rgba(120, 120, 130, 100));
        
        /* Draw icon placeholder (circle) */
        VaxpF32 icon_y = bounds.y + (nav->show_labels ? 14 : bounds.height / 2.0f);
        VaxpPaint icp = vaxp_paint_fill(color);
        vaxp_canvas_draw_circle(canvas, center_x, icon_y, 
                                  is_selected ? 14 : 12, &icp);
        
        /* Draw label */
        if (nav->show_labels || (nav->style == VAXP_NAV_SHIFTING && is_selected)) {
            VaxpF32 label_y = bounds.y + bounds.height - 8;
            VaxpPaint lp = vaxp_paint_fill(color);
            
            if (nav->items[i].label) {
                vaxp_canvas_draw_text(canvas, nav->items[i].label, 
                                       center_x, label_y, NULL, &lp);
            }
        }
    }
    
    /* Request redraw for animation */
    if (nav->indicator_x != nav->indicator_target) {
        widget->needs_redraw = VAXP_TRUE;
    }
}

/* ============================================================================
 * EVENTS
 * ============================================================================ */

static VaxpBool bottom_nav_on_event(VaxpWidget* widget, const VaxpEvent* event) {
    VaxpBottomNav* nav = (VaxpBottomNav*)widget;
    
    if (event->type == VAXP_EVENT_MOUSE_BUTTON_DOWN && nav->item_count > 0) {
        VaxpF32 item_width = widget->bounds.width / (VaxpF32)nav->item_count;
        VaxpF32 local_x = event->mouse.x - widget->bounds.x;
        
        VaxpI32 clicked = (VaxpI32)(local_x / item_width);
        if (clicked >= 0 && clicked < (VaxpI32)nav->item_count) {
            if (nav->items[clicked].enabled && clicked != nav->selected) {
                nav->selected = clicked;
                
                if (nav->on_change) {
                    nav->on_change(clicked, nav->user_data);
                }
                
                vaxp_widget_invalidate(widget);
                return VAXP_TRUE;
            }
        }
    }
    
    return VAXP_FALSE;
}

/* ============================================================================
 * PUBLIC API
 * ============================================================================ */

VaxpResultPtr vaxp_bottom_nav_create(const VaxpNavItem* items, VaxpU32 count) {
    if (!items || count == 0) {
        return VAXP_ERR_PTR(VAXP_ERROR_INVALID_STATE);
    }
    
    VaxpResultPtr result = vaxp_widget_create(&vaxp_bottom_nav_class);
    if (!result.ok) return result;
    
    VaxpBottomNav* nav = (VaxpBottomNav*)result.value;
    
    /* Copy items */
    nav->items = vaxp_alloc(count * sizeof(VaxpNavItem));
    if (!nav->items) {
        vaxp_unref(nav);
        return VAXP_ERR_PTR(VAXP_ERROR_OUT_OF_MEMORY);
    }
    
    memcpy(nav->items, items, count * sizeof(VaxpNavItem));
    nav->item_count = count;
    
    /* Enable all by default */
    for (VaxpU32 i = 0; i < count; i++) {
        nav->items[i].enabled = VAXP_TRUE;
    }
    
    return result;
}

void vaxp_bottom_nav_set_selected(VaxpBottomNav* nav, VaxpI32 index) {
    if (!nav || index < 0 || (VaxpU32)index >= nav->item_count) return;
    nav->selected = index;
    vaxp_widget_invalidate((VaxpWidget*)nav);
}

VaxpI32 vaxp_bottom_nav_get_selected(VaxpBottomNav* nav) {
    return nav ? nav->selected : -1;
}

void vaxp_bottom_nav_set_on_change(VaxpBottomNav* nav, 
                                     VaxpNavCallback callback, 
                                     void* user_data) {
    if (!nav) return;
    nav->on_change = callback;
    nav->user_data = user_data;
}

void vaxp_bottom_nav_set_style(VaxpBottomNav* nav, VaxpNavStyle style) {
    if (!nav) return;
    nav->style = style;
    vaxp_widget_invalidate((VaxpWidget*)nav);
}

void vaxp_bottom_nav_set_colors(VaxpBottomNav* nav,
                                  VaxpColor background,
                                  VaxpColor active,
                                  VaxpColor inactive) {
    if (!nav) return;
    nav->background = background;
    nav->active_color = active;
    nav->inactive_color = inactive;
    vaxp_widget_invalidate((VaxpWidget*)nav);
}

void vaxp_bottom_nav_set_show_labels(VaxpBottomNav* nav, VaxpBool show) {
    if (!nav) return;
    nav->show_labels = show;
    vaxp_widget_invalidate((VaxpWidget*)nav);
}

void vaxp_bottom_nav_set_elevation(VaxpBottomNav* nav, VaxpF32 elevation) {
    if (!nav) return;
    nav->elevation = elevation;
    vaxp_widget_invalidate((VaxpWidget*)nav);
}

void vaxp_bottom_nav_set_item_enabled(VaxpBottomNav* nav, VaxpU32 index, VaxpBool enabled) {
    if (!nav || index >= nav->item_count) return;
    nav->items[index].enabled = enabled;
    vaxp_widget_invalidate((VaxpWidget*)nav);
}
