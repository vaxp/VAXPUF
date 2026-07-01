/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_appbar.c - Application Bar Widget implementation
 */

#include "vaxp/widgets/vaxp_appbar.h"
#include "vaxp/core/vaxp_memory.h"
#include "vaxp/graphics/vaxp_canvas.h"
#include <string.h>
#include <stdio.h>

/* Default dimensions */
#define APPBAR_DEFAULT_HEIGHT 56.0f
#define APPBAR_PADDING 16.0f
#define APPBAR_TITLE_SIZE 20.0f
#define APPBAR_SUBTITLE_SIZE 14.0f
#define APPBAR_ICON_SIZE 24.0f
#define APPBAR_ACTION_GAP 8.0f

/* ============================================================================
 * FORWARD DECLARATIONS
 * ============================================================================ */

static void appbar_init(VaxpWidget* widget);
static void appbar_destroy(VaxpWidget* widget);
static void appbar_measure(VaxpWidget* widget, VaxpF32 aw, VaxpF32 ah,
                           VaxpF32* ow, VaxpF32* oh);
static void appbar_layout(VaxpWidget* widget, VaxpRectF bounds);
static void appbar_draw(VaxpWidget* widget, VaxpCanvas* canvas);
static VaxpBool appbar_on_event(VaxpWidget* widget, const VaxpEvent* event);

/* ============================================================================
 * CLASS DEFINITION
 * ============================================================================ */

const VaxpWidgetClass vaxp_appbar_class = {
    .class_name = "VaxpAppBar",
    .instance_size = sizeof(VaxpAppBar),
    .parent_class = &vaxp_widget_class,
    .init = appbar_init,
    .destroy = appbar_destroy,
    .measure = appbar_measure,
    .layout = appbar_layout,
    .draw = appbar_draw,
    .on_event = appbar_on_event,
    .on_state_changed = NULL,
};

/* ============================================================================
 * LIFECYCLE
 * ============================================================================ */

static void appbar_init(VaxpWidget* widget) {
    VaxpAppBar* bar = (VaxpAppBar*)widget;
    
    bar->title = NULL;
    bar->subtitle = NULL;
    bar->leading = NULL;
    bar->actions = NULL;
    bar->action_count = 0;
    bar->background = vaxp_color_rgb(60, 120, 220);  /* Primary blue */
    bar->title_color = VAXP_COLOR_WHITE;
    bar->subtitle_color = vaxp_color_rgba(255, 255, 255, 200);
    bar->elevation = VAXP_APPBAR_ELEVATED;
    bar->height = APPBAR_DEFAULT_HEIGHT;
    bar->center_title = VAXP_FALSE;
    bar->on_leading_tap = NULL;
    bar->user_data = NULL;
}

static void appbar_destroy(VaxpWidget* widget) {
    VaxpAppBar* bar = (VaxpAppBar*)widget;
    
    if (bar->title) {
        vaxp_free(bar->title, strlen(bar->title) + 1);
        bar->title = NULL;
    }
    if (bar->subtitle) {
        vaxp_free(bar->subtitle, strlen(bar->subtitle) + 1);
        bar->subtitle = NULL;
    }
    if (bar->leading) {
        vaxp_unref(bar->leading);
        bar->leading = NULL;
    }
    
    /* Free actions */
    for (VaxpU32 i = 0; i < bar->action_count; i++) {
        vaxp_unref(bar->actions[i]);
    }
    if (bar->actions) {
        vaxp_free(bar->actions, bar->action_count * sizeof(VaxpWidget*));
        bar->actions = NULL;
    }
    bar->action_count = 0;
    
    vaxp_widget_class.destroy(widget);
}

/* ============================================================================
 * LAYOUT
 * ============================================================================ */

static void appbar_measure(VaxpWidget* widget, VaxpF32 aw, VaxpF32 ah,
                           VaxpF32* ow, VaxpF32* oh) {
    VaxpAppBar* bar = (VaxpAppBar*)widget;
    (void)ah;
    
    *ow = aw;  /* Full width */
    *oh = bar->height;
}

static void appbar_layout(VaxpWidget* widget, VaxpRectF bounds) {
    VaxpAppBar* bar = (VaxpAppBar*)widget;
    widget->bounds = bounds;
    
    VaxpF32 x = APPBAR_PADDING;
    VaxpF32 center_y = bounds.height / 2.0f;
    
    /* Layout leading widget */
    if (bar->leading) {
        VaxpF32 lw, lh;
        vaxp_widget_measure(bar->leading, APPBAR_ICON_SIZE, APPBAR_ICON_SIZE, &lw, &lh);
        VaxpRectF leading_bounds = {
            x, center_y - lh / 2.0f, lw, lh
        };
        vaxp_widget_layout(bar->leading, leading_bounds);
        x += lw + APPBAR_PADDING;
    }
    
    /* Layout actions from right */
    VaxpF32 actions_width = 0;
    for (VaxpU32 i = 0; i < bar->action_count; i++) {
        VaxpF32 aw, ah;
        vaxp_widget_measure(bar->actions[i], APPBAR_ICON_SIZE, APPBAR_ICON_SIZE, &aw, &ah);
        actions_width += aw + APPBAR_ACTION_GAP;
    }
    
    VaxpF32 action_x = bounds.width - APPBAR_PADDING;
    for (VaxpI32 i = (VaxpI32)bar->action_count - 1; i >= 0; i--) {
        VaxpF32 aw, ah;
        vaxp_widget_measure(bar->actions[i], APPBAR_ICON_SIZE, APPBAR_ICON_SIZE, &aw, &ah);
        action_x -= aw;
        VaxpRectF action_bounds = {
            action_x, center_y - ah / 2.0f, aw, ah
        };
        vaxp_widget_layout(bar->actions[i], action_bounds);
        action_x -= APPBAR_ACTION_GAP;
    }
}

/* ============================================================================
 * DRAWING
 * ============================================================================ */

static void appbar_draw(VaxpWidget* widget, VaxpCanvas* canvas) {
    VaxpAppBar* bar = (VaxpAppBar*)widget;
    VaxpRectF bounds = { 0, 0, widget->bounds.width, widget->bounds.height };
    
    /* Draw shadow (if elevated) */
    if (bar->elevation > 0) {
        VaxpColor shadow = vaxp_color_rgba(0, 0, 0, (VaxpU8)(bar->elevation * 8));
        for (VaxpF32 i = 0; i < bar->elevation; i += 1.0f) {
            VaxpRectF shadow_rect = {
                bounds.x, bounds.y + bounds.height + i,
                bounds.width, 1
            };
            VaxpPaint sp = vaxp_paint_fill(shadow);
            shadow.a = (VaxpU8)((bar->elevation - i) * 6);
            vaxp_canvas_draw_rect(canvas, shadow_rect, &sp);
        }
    }
    
    /* Draw background */
    VaxpPaint bg = vaxp_paint_fill(bar->background);
    vaxp_canvas_draw_rect(canvas, bounds, &bg);
    
    /* Calculate title position */
    VaxpF32 title_x = APPBAR_PADDING;
    if (bar->leading) {
        title_x = bar->leading->bounds.x + bar->leading->bounds.width + APPBAR_PADDING;
    }
    
    VaxpF32 title_max_width = bounds.width - title_x - APPBAR_PADDING;
    for (VaxpU32 i = 0; i < bar->action_count; i++) {
        title_max_width -= bar->actions[i]->bounds.width + APPBAR_ACTION_GAP;
    }
    
    /* Draw title */
    if (bar->title) {
        VaxpF32 title_y;
        if (bar->subtitle) {
            title_y = bounds.height / 2.0f - 4;
        } else {
            title_y = bounds.height / 2.0f + APPBAR_TITLE_SIZE / 3.0f;
        }
        
        VaxpF32 tx = bar->center_title ? 
            bounds.width / 2.0f : title_x;
        
        VaxpPaint tp = vaxp_paint_fill(bar->title_color);
        vaxp_canvas_draw_text(canvas, bar->title, tx, title_y, NULL, &tp);
    }
    
    /* Draw subtitle */
    if (bar->subtitle) {
        VaxpF32 subtitle_y = bounds.height / 2.0f + APPBAR_SUBTITLE_SIZE + 2;
        VaxpF32 tx = bar->center_title ? 
            bounds.width / 2.0f : title_x;
        
        VaxpPaint sp = vaxp_paint_fill(bar->subtitle_color);
        vaxp_canvas_draw_text(canvas, bar->subtitle, tx, subtitle_y, NULL, &sp);
    }
    
    /* Draw leading widget */
    if (bar->leading) {
        vaxp_widget_draw(bar->leading, canvas);
    }
    
    /* Draw action widgets */
    for (VaxpU32 i = 0; i < bar->action_count; i++) {
        vaxp_widget_draw(bar->actions[i], canvas);
    }
}

/* ============================================================================
 * EVENTS
 * ============================================================================ */

static VaxpBool appbar_on_event(VaxpWidget* widget, const VaxpEvent* event) {
    VaxpAppBar* bar = (VaxpAppBar*)widget;
    
    /* Forward to leading */
    if (bar->leading && vaxp_widget_dispatch_event(bar->leading, event)) {
        return VAXP_TRUE;
    }
    
    /* Forward to actions */
    for (VaxpU32 i = 0; i < bar->action_count; i++) {
        if (vaxp_widget_dispatch_event(bar->actions[i], event)) {
            return VAXP_TRUE;
        }
    }
    
    /* Handle leading tap */
    if (event->type == VAXP_EVENT_MOUSE_BUTTON_DOWN && bar->leading && bar->on_leading_tap) {
        if (event->mouse.x >= bar->leading->bounds.x &&
            event->mouse.x <= bar->leading->bounds.x + bar->leading->bounds.width &&
            event->mouse.y >= bar->leading->bounds.y &&
            event->mouse.y <= bar->leading->bounds.y + bar->leading->bounds.height) {
            bar->on_leading_tap(bar, bar->user_data);
            return VAXP_TRUE;
        }
    }
    
    return VAXP_FALSE;
}

/* ============================================================================
 * PUBLIC API
 * ============================================================================ */

VaxpResultPtr vaxp_appbar_create(const char* title) {
    VaxpResultPtr result = vaxp_widget_create(&vaxp_appbar_class);
    if (!result.ok) return result;
    
    VaxpAppBar* bar = (VaxpAppBar*)result.value;
    
    if (title) {
        vaxp_appbar_set_title(bar, title);
    }
    
    return result;
}

void vaxp_appbar_set_title(VaxpAppBar* bar, const char* title) {
    if (!bar) return;
    
    if (bar->title) {
        vaxp_free(bar->title, strlen(bar->title) + 1);
        bar->title = NULL;
    }
    
    if (title) {
        VaxpSize len = strlen(title) + 1;
        bar->title = vaxp_alloc(len);
        if (bar->title) {
            memcpy(bar->title, title, len);
        }
    }
    
    vaxp_widget_invalidate((VaxpWidget*)bar);
}

void vaxp_appbar_set_subtitle(VaxpAppBar* bar, const char* subtitle) {
    if (!bar) return;
    
    if (bar->subtitle) {
        vaxp_free(bar->subtitle, strlen(bar->subtitle) + 1);
        bar->subtitle = NULL;
    }
    
    if (subtitle) {
        VaxpSize len = strlen(subtitle) + 1;
        bar->subtitle = vaxp_alloc(len);
        if (bar->subtitle) {
            memcpy(bar->subtitle, subtitle, len);
        }
    }
    
    vaxp_widget_invalidate((VaxpWidget*)bar);
}

void vaxp_appbar_set_leading(VaxpAppBar* bar, VaxpWidget* leading) {
    if (!bar) return;
    
    if (bar->leading) {
        vaxp_unref(bar->leading);
    }
    
    bar->leading = leading ? vaxp_ref(leading) : NULL;
    vaxp_widget_invalidate_layout((VaxpWidget*)bar);
}

void vaxp_appbar_add_action(VaxpAppBar* bar, VaxpWidget* action) {
    if (!bar || !action) return;
    
    /* Grow array */
    VaxpU32 new_count = bar->action_count + 1;
    VaxpWidget** new_arr = vaxp_realloc(bar->actions,
        bar->action_count * sizeof(VaxpWidget*),
        new_count * sizeof(VaxpWidget*));
    
    if (!new_arr) return;
    
    bar->actions = new_arr;
    bar->actions[bar->action_count] = vaxp_ref(action);
    bar->action_count = new_count;
    
    vaxp_widget_invalidate_layout((VaxpWidget*)bar);
}

void vaxp_appbar_clear_actions(VaxpAppBar* bar) {
    if (!bar) return;
    
    for (VaxpU32 i = 0; i < bar->action_count; i++) {
        vaxp_unref(bar->actions[i]);
    }
    
    if (bar->actions) {
        vaxp_free(bar->actions, bar->action_count * sizeof(VaxpWidget*));
        bar->actions = NULL;
    }
    bar->action_count = 0;
    
    vaxp_widget_invalidate_layout((VaxpWidget*)bar);
}

void vaxp_appbar_set_background(VaxpAppBar* bar, VaxpColor color) {
    if (!bar) return;
    bar->background = color;
    vaxp_widget_invalidate((VaxpWidget*)bar);
}

void vaxp_appbar_set_elevation(VaxpAppBar* bar, VaxpF32 elevation) {
    if (!bar) return;
    bar->elevation = elevation;
    vaxp_widget_invalidate((VaxpWidget*)bar);
}

void vaxp_appbar_set_center_title(VaxpAppBar* bar, VaxpBool center) {
    if (!bar) return;
    bar->center_title = center;
    vaxp_widget_invalidate((VaxpWidget*)bar);
}

void vaxp_appbar_set_height(VaxpAppBar* bar, VaxpF32 height) {
    if (!bar) return;
    bar->height = height;
    vaxp_widget_invalidate_layout((VaxpWidget*)bar);
}

void vaxp_appbar_set_on_leading_tap(VaxpAppBar* bar, 
                                      void (*callback)(VaxpAppBar*, void*),
                                      void* user_data) {
    if (!bar) return;
    bar->on_leading_tap = callback;
    bar->user_data = user_data;
}
