/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_badge.c - Badge widget implementation
 */

#include "venom/widgets/venom_badge.h"
#include "venom/core/venom_memory.h"
#include <string.h>
#include <stdio.h>

#define DEFAULT_SIZE 20.0f
#define DOT_SIZE 10.0f

static void badge_init(VenomWidget* widget) {
    VenomBadge* badge = (VenomBadge*)widget;
    
    badge->child = NULL;
    badge->text = NULL;
    badge->count = 0;
    badge->show_zero = VENOM_FALSE;
    badge->dot_only = VENOM_FALSE;
    
    badge->bg_color = (VenomColor){ 244, 67, 54, 255 };  /* Red */
    badge->text_color = (VenomColor){ 255, 255, 255, 255 };
    badge->size = DEFAULT_SIZE;
    badge->offset_x = 0;
    badge->offset_y = 0;
}

static void badge_destroy(VenomWidget* widget) {
    VenomBadge* badge = (VenomBadge*)widget;
    
    if (badge->child) {
        badge->child->parent = NULL;
        venom_unref(badge->child);
        badge->child = NULL;
    }
    
    if (badge->text) {
        venom_free(badge->text, strlen(badge->text) + 1);
        badge->text = NULL;
    }
    
    venom_widget_class.destroy(widget);
}

static void badge_measure(VenomWidget* widget, VenomF32 available_width, VenomF32 available_height,
                          VenomF32* out_width, VenomF32* out_height) {
    VenomBadge* badge = (VenomBadge*)widget;
    
    if (badge->child) {
        venom_widget_measure(badge->child, available_width, available_height, out_width, out_height);
    } else {
        *out_width = 0;
        *out_height = 0;
    }
}

static void badge_layout(VenomWidget* widget, VenomRectF bounds) {
    VenomBadge* badge = (VenomBadge*)widget;
    widget->bounds = bounds;
    
    if (badge->child && badge->child->visible) {
        VenomRectF child_bounds = { 0, 0, bounds.width, bounds.height };
        venom_widget_layout(badge->child, child_bounds);
    }
}

static void badge_draw(VenomWidget* widget, VenomCanvas* canvas) {
    VenomBadge* badge = (VenomBadge*)widget;
    
    /* Draw child first */
    if (badge->child && badge->child->visible) {
        venom_widget_draw(badge->child, canvas);
    }
    
    /* Determine if badge should be shown */
    VenomBool show_badge = VENOM_TRUE;
    if (!badge->dot_only && badge->count == 0 && !badge->show_zero && !badge->text) {
        show_badge = VENOM_FALSE;
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
    VenomF32 badge_w, badge_h;
    
    if (badge->dot_only) {
        badge_w = badge_h = DOT_SIZE;
    } else {
        VenomF32 text_width = display_text ? (VenomF32)strlen(display_text) * 8 : 0;
        badge_w = text_width + 10;
        if (badge_w < badge->size) badge_w = badge->size;
        badge_h = badge->size;
    }
    
    /* Position at top-right corner */
    VenomF32 bx = widget->bounds.width - badge_w / 2 + badge->offset_x;
    VenomF32 by = -badge_h / 2 + badge->offset_y;
    
    /* Draw badge circle/pill */
    VenomPaint bg_paint = venom_paint_fill(badge->bg_color);
    
    if (badge->dot_only || badge_w == badge_h) {
        /* Circle */
        venom_canvas_draw_circle(canvas, bx + badge_w / 2, by + badge_h / 2, badge_w / 2, &bg_paint);
    } else {
        /* Pill shape */
        VenomRectF pill = { bx, by, badge_w, badge_h };
        venom_canvas_draw_rounded_rect(canvas, pill, badge_h / 2, &bg_paint);
    }
    
    /* Draw text */
    if (!badge->dot_only && display_text) {
        VenomPaint text_paint = venom_paint_fill(badge->text_color);
        VenomF32 tx = bx + (badge_w - (VenomF32)strlen(display_text) * 7) / 2;
        VenomF32 ty = by + badge_h / 2 + 4;
        venom_canvas_draw_text(canvas, display_text, tx, ty, NULL, &text_paint);
    }
}

static VenomBool badge_on_event(VenomWidget* widget, const VenomEvent* event) {
    VenomBadge* badge = (VenomBadge*)widget;
    
    if (badge->child && badge->child->visible) {
        return venom_widget_dispatch_event(badge->child, event);
    }
    
    return VENOM_FALSE;
}

const VenomWidgetClass venom_badge_class = {
    .class_name = "VenomBadge",
    .instance_size = sizeof(VenomBadge),
    .parent_class = &venom_widget_class,
    .init = badge_init,
    .destroy = badge_destroy,
    .measure = badge_measure,
    .layout = badge_layout,
    .draw = badge_draw,
    .on_event = badge_on_event,
    .on_state_changed = NULL,
};

VenomResultPtr venom_badge_create(void) {
    return venom_widget_create(&venom_badge_class);
}

VenomResult venom_badge_set_child(VenomBadge* badge, VenomWidget* child) {
    VENOM_ENSURE_NOT_NULL(badge);
    
    if (badge->child) {
        badge->child->parent = NULL;
        venom_unref(badge->child);
    }
    
    badge->child = child;
    if (child) {
        venom_ref(child);
        child->parent = (VenomWidget*)badge;
    }
    
    venom_widget_invalidate_layout((VenomWidget*)badge);
    return VENOM_OK_UNIT();
}

void venom_badge_set_count(VenomBadge* badge, int count) {
    if (badge) {
        badge->count = count;
        venom_widget_invalidate((VenomWidget*)badge);
    }
}

void venom_badge_set_text(VenomBadge* badge, const char* text) {
    if (!badge) return;
    
    if (badge->text) {
        venom_free(badge->text, strlen(badge->text) + 1);
        badge->text = NULL;
    }
    
    if (text) {
        VenomSize len = strlen(text) + 1;
        badge->text = (char*)venom_alloc(len);
        if (badge->text) {
            memcpy(badge->text, text, len);
        }
    }
    
    venom_widget_invalidate((VenomWidget*)badge);
}

void venom_badge_set_dot(VenomBadge* badge, VenomBool dot_only) {
    if (badge) {
        badge->dot_only = dot_only;
        venom_widget_invalidate((VenomWidget*)badge);
    }
}

void venom_badge_set_visible(VenomBadge* badge, VenomBool visible) {
    if (badge) {
        ((VenomWidget*)badge)->visible = visible;
        venom_widget_invalidate((VenomWidget*)badge);
    }
}

VenomWidget* _venom_badge_build(const VenomBadgeConfig* config) {
    VenomResultPtr result = venom_badge_create();
    if (!result.ok) return NULL;
    
    VenomBadge* badge = (VenomBadge*)result.value;
    
    if (config->child) venom_badge_set_child(badge, config->child);
    if (config->count > 0) badge->count = config->count;
    if (config->text) venom_badge_set_text(badge, config->text);
    badge->dot_only = config->dot_only;
    if (config->color.a > 0) badge->bg_color = config->color;
    
    return (VenomWidget*)badge;
}
