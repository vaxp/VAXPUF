/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_badge.c - Badge widget implementation
 */

#include "vaxp/widgets/vaxp_badge.h"
#include "vaxp/core/vaxp_memory.h"
#include <string.h>
#include <stdio.h>

#define DEFAULT_SIZE 20.0f
#define DOT_SIZE 10.0f

static void badge_init(VaxpWidget* widget) {
    VaxpBadge* badge = (VaxpBadge*)widget;
    
    badge->child = NULL;
    badge->text = NULL;
    badge->count = 0;
    badge->show_zero = VAXP_FALSE;
    badge->dot_only = VAXP_FALSE;
    
    badge->bg_color = (VaxpColor){ 244, 67, 54, 255 };  /* Red */
    badge->text_color = (VaxpColor){ 255, 255, 255, 255 };
    badge->size = DEFAULT_SIZE;
    badge->offset_x = 0;
    badge->offset_y = 0;
}

static void badge_destroy(VaxpWidget* widget) {
    VaxpBadge* badge = (VaxpBadge*)widget;
    
    if (badge->child) {
        badge->child->parent = NULL;
        vaxp_unref(badge->child);
        badge->child = NULL;
    }
    
    if (badge->text) {
        vaxp_free(badge->text, strlen(badge->text) + 1);
        badge->text = NULL;
    }
    
    vaxp_widget_class.destroy(widget);
}

static void badge_measure(VaxpWidget* widget, VaxpF32 available_width, VaxpF32 available_height,
                          VaxpF32* out_width, VaxpF32* out_height) {
    VaxpBadge* badge = (VaxpBadge*)widget;
    
    if (badge->child) {
        vaxp_widget_measure(badge->child, available_width, available_height, out_width, out_height);
    } else {
        /* Show badge circle even without child */
        *out_width = badge->size;
        *out_height = badge->size;
    }
}

static void badge_layout(VaxpWidget* widget, VaxpRectF bounds) {
    VaxpBadge* badge = (VaxpBadge*)widget;
    widget->bounds = bounds;
    
    if (badge->child && badge->child->visible) {
        VaxpRectF child_bounds = { 0, 0, bounds.width, bounds.height };
        vaxp_widget_layout(badge->child, child_bounds);
    }
}

static void badge_draw(VaxpWidget* widget, VaxpCanvas* canvas) {
    VaxpBadge* badge = (VaxpBadge*)widget;
    
    /* Draw child first */
    if (badge->child && badge->child->visible) {
        vaxp_widget_draw(badge->child, canvas);
    }
    
    /* Determine if badge should be shown */
    VaxpBool show_badge = VAXP_TRUE;
    if (!badge->dot_only && badge->count == 0 && !badge->show_zero && !badge->text) {
        show_badge = VAXP_FALSE;
    }
    
    if (!show_badge) return;
    
    /* Prepare badge text */
    char buf[16];
    const char* display_text = NULL;
    
    if (!badge->dot_only) {
        if (badge->text) {
            display_text = badge->text;
        } else if (badge->count > 99) {
            snprintf(buf, sizeof(buf), "99+");
            display_text = buf;
        } else {
            snprintf(buf, sizeof(buf), "%d", badge->count);
            display_text = buf;
        }
    }
    
    /* Calculate badge size and position */
    VaxpF32 badge_w, badge_h;
    
    if (badge->dot_only) {
        badge_w = badge_h = DOT_SIZE;
    } else {
        VaxpF32 text_width = display_text ? (VaxpF32)strlen(display_text) * 8 : 0;
        badge_w = text_width + 10;
        if (badge_w < badge->size) badge_w = badge->size;
        badge_h = badge->size;
    }
    
    /* Position at top-right corner or center if no child */
    VaxpF32 bx, by;
    if (badge->child) {
        bx = widget->bounds.width - badge_w / 2 + badge->offset_x;
        by = -badge_h / 2 + badge->offset_y;
    } else {
        bx = (widget->bounds.width - badge_w) / 2;
        by = (widget->bounds.height - badge_h) / 2;
    }
    
    /* Draw badge circle/pill */
    VaxpPaint bg_paint = vaxp_paint_fill(badge->bg_color);
    
    if (badge->dot_only || badge_w == badge_h) {
        /* Circle */
        vaxp_canvas_draw_circle(canvas, bx + badge_w / 2, by + badge_h / 2, badge_w / 2, &bg_paint);
    } else {
        /* Pill shape */
        VaxpRectF pill = { bx, by, badge_w, badge_h };
        vaxp_canvas_draw_rounded_rect(canvas, pill, badge_h / 2, &bg_paint);
    }
    
    /* Draw text */
    if (!badge->dot_only && display_text) {
        VaxpPaint text_paint = vaxp_paint_fill(badge->text_color);
        VaxpF32 tx = bx + (badge_w - (VaxpF32)strlen(display_text) * 7) / 2;
        VaxpF32 ty = by + badge_h / 2 + 4;
        vaxp_canvas_draw_text(canvas, display_text, tx, ty, NULL, &text_paint);
    }
}

static VaxpBool badge_on_event(VaxpWidget* widget, const VaxpEvent* event) {
    VaxpBadge* badge = (VaxpBadge*)widget;
    
    if (badge->child && badge->child->visible) {
        return vaxp_widget_dispatch_event(badge->child, event);
    }
    
    return VAXP_FALSE;
}

const VaxpWidgetClass vaxp_badge_class = {
    .class_name = "VaxpBadge",
    .instance_size = sizeof(VaxpBadge),
    .parent_class = &vaxp_widget_class,
    .init = badge_init,
    .destroy = badge_destroy,
    .measure = badge_measure,
    .layout = badge_layout,
    .draw = badge_draw,
    .on_event = badge_on_event,
    .on_state_changed = NULL,
};

VaxpResultPtr vaxp_badge_create(void) {
    return vaxp_widget_create(&vaxp_badge_class);
}

VaxpResult vaxp_badge_set_child(VaxpBadge* badge, VaxpWidget* child) {
    VAXP_ENSURE_NOT_NULL(badge);
    
    if (badge->child) {
        badge->child->parent = NULL;
        vaxp_unref(badge->child);
    }
    
    badge->child = child;
    if (child) {
        vaxp_ref(child);
        child->parent = (VaxpWidget*)badge;
    }
    
    vaxp_widget_invalidate_layout((VaxpWidget*)badge);
    return VAXP_OK_UNIT();
}

void vaxp_badge_set_count(VaxpBadge* badge, int count) {
    if (badge) {
        badge->count = count;
        vaxp_widget_invalidate((VaxpWidget*)badge);
    }
}

void vaxp_badge_set_text(VaxpBadge* badge, const char* text) {
    if (!badge) return;
    
    if (badge->text) {
        vaxp_free(badge->text, strlen(badge->text) + 1);
        badge->text = NULL;
    }
    
    if (text) {
        VaxpSize len = strlen(text) + 1;
        badge->text = (char*)vaxp_alloc(len);
        if (badge->text) {
            memcpy(badge->text, text, len);
        }
    }
    
    vaxp_widget_invalidate((VaxpWidget*)badge);
}

void vaxp_badge_set_dot(VaxpBadge* badge, VaxpBool dot_only) {
    if (badge) {
        badge->dot_only = dot_only;
        vaxp_widget_invalidate((VaxpWidget*)badge);
    }
}

void vaxp_badge_set_visible(VaxpBadge* badge, VaxpBool visible) {
    if (badge) {
        ((VaxpWidget*)badge)->visible = visible;
        vaxp_widget_invalidate((VaxpWidget*)badge);
    }
}

VaxpWidget* _vaxp_badge_build(const VaxpBadgeConfig* config) {
    VaxpResultPtr result = vaxp_badge_create();
    if (!result.ok) return NULL;
    
    VaxpBadge* badge = (VaxpBadge*)result.value;
    
    if (config->child) vaxp_badge_set_child(badge, config->child);
    if (config->count > 0) badge->count = config->count;
    if (config->text) vaxp_badge_set_text(badge, config->text);
    badge->dot_only = config->dot_only;
    if (config->color.a > 0) badge->bg_color = config->color;
    
    return (VaxpWidget*)badge;
}
