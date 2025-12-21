/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_appbar.c - Application Bar Widget implementation
 */

#include "venom/widgets/venom_appbar.h"
#include "venom/core/venom_memory.h"
#include "venom/graphics/venom_canvas.h"
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

static void appbar_init(VenomWidget* widget);
static void appbar_destroy(VenomWidget* widget);
static void appbar_measure(VenomWidget* widget, VenomF32 aw, VenomF32 ah,
                           VenomF32* ow, VenomF32* oh);
static void appbar_layout(VenomWidget* widget, VenomRectF bounds);
static void appbar_draw(VenomWidget* widget, VenomCanvas* canvas);
static VenomBool appbar_on_event(VenomWidget* widget, const VenomEvent* event);

/* ============================================================================
 * CLASS DEFINITION
 * ============================================================================ */

const VenomWidgetClass venom_appbar_class = {
    .class_name = "VenomAppBar",
    .instance_size = sizeof(VenomAppBar),
    .parent_class = &venom_widget_class,
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

static void appbar_init(VenomWidget* widget) {
    VenomAppBar* bar = (VenomAppBar*)widget;
    
    bar->title = NULL;
    bar->subtitle = NULL;
    bar->leading = NULL;
    bar->actions = NULL;
    bar->action_count = 0;
    bar->background = venom_color_rgb(60, 120, 220);  /* Primary blue */
    bar->title_color = VENOM_COLOR_WHITE;
    bar->subtitle_color = venom_color_rgba(255, 255, 255, 200);
    bar->elevation = VENOM_APPBAR_ELEVATED;
    bar->height = APPBAR_DEFAULT_HEIGHT;
    bar->center_title = VENOM_FALSE;
    bar->on_leading_tap = NULL;
    bar->user_data = NULL;
}

static void appbar_destroy(VenomWidget* widget) {
    VenomAppBar* bar = (VenomAppBar*)widget;
    
    if (bar->title) {
        venom_free(bar->title, strlen(bar->title) + 1);
        bar->title = NULL;
    }
    if (bar->subtitle) {
        venom_free(bar->subtitle, strlen(bar->subtitle) + 1);
        bar->subtitle = NULL;
    }
    if (bar->leading) {
        venom_unref(bar->leading);
        bar->leading = NULL;
    }
    
    /* Free actions */
    for (VenomU32 i = 0; i < bar->action_count; i++) {
        venom_unref(bar->actions[i]);
    }
    if (bar->actions) {
        venom_free(bar->actions, bar->action_count * sizeof(VenomWidget*));
        bar->actions = NULL;
    }
    bar->action_count = 0;
    
    venom_widget_class.destroy(widget);
}

/* ============================================================================
 * LAYOUT
 * ============================================================================ */

static void appbar_measure(VenomWidget* widget, VenomF32 aw, VenomF32 ah,
                           VenomF32* ow, VenomF32* oh) {
    VenomAppBar* bar = (VenomAppBar*)widget;
    (void)ah;
    
    *ow = aw;  /* Full width */
    *oh = bar->height;
}

static void appbar_layout(VenomWidget* widget, VenomRectF bounds) {
    VenomAppBar* bar = (VenomAppBar*)widget;
    widget->bounds = bounds;
    
    VenomF32 x = APPBAR_PADDING;
    VenomF32 center_y = bounds.height / 2.0f;
    
    /* Layout leading widget */
    if (bar->leading) {
        VenomF32 lw, lh;
        venom_widget_measure(bar->leading, APPBAR_ICON_SIZE, APPBAR_ICON_SIZE, &lw, &lh);
        VenomRectF leading_bounds = {
            x, center_y - lh / 2.0f, lw, lh
        };
        venom_widget_layout(bar->leading, leading_bounds);
        x += lw + APPBAR_PADDING;
    }
    
    /* Layout actions from right */
    VenomF32 actions_width = 0;
    for (VenomU32 i = 0; i < bar->action_count; i++) {
        VenomF32 aw, ah;
        venom_widget_measure(bar->actions[i], APPBAR_ICON_SIZE, APPBAR_ICON_SIZE, &aw, &ah);
        actions_width += aw + APPBAR_ACTION_GAP;
    }
    
    VenomF32 action_x = bounds.width - APPBAR_PADDING;
    for (VenomI32 i = (VenomI32)bar->action_count - 1; i >= 0; i--) {
        VenomF32 aw, ah;
        venom_widget_measure(bar->actions[i], APPBAR_ICON_SIZE, APPBAR_ICON_SIZE, &aw, &ah);
        action_x -= aw;
        VenomRectF action_bounds = {
            action_x, center_y - ah / 2.0f, aw, ah
        };
        venom_widget_layout(bar->actions[i], action_bounds);
        action_x -= APPBAR_ACTION_GAP;
    }
}

/* ============================================================================
 * DRAWING
 * ============================================================================ */

static void appbar_draw(VenomWidget* widget, VenomCanvas* canvas) {
    VenomAppBar* bar = (VenomAppBar*)widget;
    VenomRectF bounds = { 0, 0, widget->bounds.width, widget->bounds.height };
    
    /* Draw shadow (if elevated) */
    if (bar->elevation > 0) {
        VenomColor shadow = venom_color_rgba(0, 0, 0, (VenomU8)(bar->elevation * 8));
        for (VenomF32 i = 0; i < bar->elevation; i += 1.0f) {
            VenomRectF shadow_rect = {
                bounds.x, bounds.y + bounds.height + i,
                bounds.width, 1
            };
            VenomPaint sp = venom_paint_fill(shadow);
            shadow.a = (VenomU8)((bar->elevation - i) * 6);
            venom_canvas_draw_rect(canvas, shadow_rect, &sp);
        }
    }
    
    /* Draw background */
    VenomPaint bg = venom_paint_fill(bar->background);
    venom_canvas_draw_rect(canvas, bounds, &bg);
    
    /* Calculate title position */
    VenomF32 title_x = APPBAR_PADDING;
    if (bar->leading) {
        title_x = bar->leading->bounds.x + bar->leading->bounds.width + APPBAR_PADDING;
    }
    
    VenomF32 title_max_width = bounds.width - title_x - APPBAR_PADDING;
    for (VenomU32 i = 0; i < bar->action_count; i++) {
        title_max_width -= bar->actions[i]->bounds.width + APPBAR_ACTION_GAP;
    }
    
    /* Draw title */
    if (bar->title) {
        VenomF32 title_y;
        if (bar->subtitle) {
            title_y = bounds.height / 2.0f - 4;
        } else {
            title_y = bounds.height / 2.0f + APPBAR_TITLE_SIZE / 3.0f;
        }
        
        VenomF32 tx = bar->center_title ? 
            bounds.width / 2.0f : title_x;
        
        VenomPaint tp = venom_paint_fill(bar->title_color);
        venom_canvas_draw_text(canvas, bar->title, tx, title_y, NULL, &tp);
    }
    
    /* Draw subtitle */
    if (bar->subtitle) {
        VenomF32 subtitle_y = bounds.height / 2.0f + APPBAR_SUBTITLE_SIZE + 2;
        VenomF32 tx = bar->center_title ? 
            bounds.width / 2.0f : title_x;
        
        VenomPaint sp = venom_paint_fill(bar->subtitle_color);
        venom_canvas_draw_text(canvas, bar->subtitle, tx, subtitle_y, NULL, &sp);
    }
    
    /* Draw leading widget */
    if (bar->leading) {
        venom_widget_draw(bar->leading, canvas);
    }
    
    /* Draw action widgets */
    for (VenomU32 i = 0; i < bar->action_count; i++) {
        venom_widget_draw(bar->actions[i], canvas);
    }
}

/* ============================================================================
 * EVENTS
 * ============================================================================ */

static VenomBool appbar_on_event(VenomWidget* widget, const VenomEvent* event) {
    VenomAppBar* bar = (VenomAppBar*)widget;
    
    /* Forward to leading */
    if (bar->leading && venom_widget_dispatch_event(bar->leading, event)) {
        return VENOM_TRUE;
    }
    
    /* Forward to actions */
    for (VenomU32 i = 0; i < bar->action_count; i++) {
        if (venom_widget_dispatch_event(bar->actions[i], event)) {
            return VENOM_TRUE;
        }
    }
    
    /* Handle leading tap */
    if (event->type == VENOM_EVENT_MOUSE_BUTTON_DOWN && bar->leading && bar->on_leading_tap) {
        if (event->mouse.x >= bar->leading->bounds.x &&
            event->mouse.x <= bar->leading->bounds.x + bar->leading->bounds.width &&
            event->mouse.y >= bar->leading->bounds.y &&
            event->mouse.y <= bar->leading->bounds.y + bar->leading->bounds.height) {
            bar->on_leading_tap(bar, bar->user_data);
            return VENOM_TRUE;
        }
    }
    
    return VENOM_FALSE;
}

/* ============================================================================
 * PUBLIC API
 * ============================================================================ */

VenomResultPtr venom_appbar_create(const char* title) {
    VenomResultPtr result = venom_widget_create(&venom_appbar_class);
    if (!result.ok) return result;
    
    VenomAppBar* bar = (VenomAppBar*)result.value;
    
    if (title) {
        venom_appbar_set_title(bar, title);
    }
    
    return result;
}

void venom_appbar_set_title(VenomAppBar* bar, const char* title) {
    if (!bar) return;
    
    if (bar->title) {
        venom_free(bar->title, strlen(bar->title) + 1);
        bar->title = NULL;
    }
    
    if (title) {
        VenomSize len = strlen(title) + 1;
        bar->title = venom_alloc(len);
        if (bar->title) {
            memcpy(bar->title, title, len);
        }
    }
    
    venom_widget_invalidate((VenomWidget*)bar);
}

void venom_appbar_set_subtitle(VenomAppBar* bar, const char* subtitle) {
    if (!bar) return;
    
    if (bar->subtitle) {
        venom_free(bar->subtitle, strlen(bar->subtitle) + 1);
        bar->subtitle = NULL;
    }
    
    if (subtitle) {
        VenomSize len = strlen(subtitle) + 1;
        bar->subtitle = venom_alloc(len);
        if (bar->subtitle) {
            memcpy(bar->subtitle, subtitle, len);
        }
    }
    
    venom_widget_invalidate((VenomWidget*)bar);
}

void venom_appbar_set_leading(VenomAppBar* bar, VenomWidget* leading) {
    if (!bar) return;
    
    if (bar->leading) {
        venom_unref(bar->leading);
    }
    
    bar->leading = leading ? venom_ref(leading) : NULL;
    venom_widget_invalidate_layout((VenomWidget*)bar);
}

void venom_appbar_add_action(VenomAppBar* bar, VenomWidget* action) {
    if (!bar || !action) return;
    
    /* Grow array */
    VenomU32 new_count = bar->action_count + 1;
    VenomWidget** new_arr = venom_realloc(bar->actions,
        bar->action_count * sizeof(VenomWidget*),
        new_count * sizeof(VenomWidget*));
    
    if (!new_arr) return;
    
    bar->actions = new_arr;
    bar->actions[bar->action_count] = venom_ref(action);
    bar->action_count = new_count;
    
    venom_widget_invalidate_layout((VenomWidget*)bar);
}

void venom_appbar_clear_actions(VenomAppBar* bar) {
    if (!bar) return;
    
    for (VenomU32 i = 0; i < bar->action_count; i++) {
        venom_unref(bar->actions[i]);
    }
    
    if (bar->actions) {
        venom_free(bar->actions, bar->action_count * sizeof(VenomWidget*));
        bar->actions = NULL;
    }
    bar->action_count = 0;
    
    venom_widget_invalidate_layout((VenomWidget*)bar);
}

void venom_appbar_set_background(VenomAppBar* bar, VenomColor color) {
    if (!bar) return;
    bar->background = color;
    venom_widget_invalidate((VenomWidget*)bar);
}

void venom_appbar_set_elevation(VenomAppBar* bar, VenomF32 elevation) {
    if (!bar) return;
    bar->elevation = elevation;
    venom_widget_invalidate((VenomWidget*)bar);
}

void venom_appbar_set_center_title(VenomAppBar* bar, VenomBool center) {
    if (!bar) return;
    bar->center_title = center;
    venom_widget_invalidate((VenomWidget*)bar);
}

void venom_appbar_set_height(VenomAppBar* bar, VenomF32 height) {
    if (!bar) return;
    bar->height = height;
    venom_widget_invalidate_layout((VenomWidget*)bar);
}

void venom_appbar_set_on_leading_tap(VenomAppBar* bar, 
                                      void (*callback)(VenomAppBar*, void*),
                                      void* user_data) {
    if (!bar) return;
    bar->on_leading_tap = callback;
    bar->user_data = user_data;
}
