/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_tooltip.c - Tooltip widget implementation
 */

#include "venom/widgets/venom_tooltip.h"
#include "venom/core/venom_memory.h"
#include <string.h>

#define DEFAULT_SHOW_DELAY 500
#define DEFAULT_PADDING 8.0f
#define DEFAULT_RADIUS 4.0f

static void tooltip_init(VenomWidget* widget) {
    VenomTooltip* tip = (VenomTooltip*)widget;
    
    tip->child = NULL;
    tip->message = NULL;
    tip->position = VENOM_TOOLTIP_TOP;
    tip->show_delay_ms = DEFAULT_SHOW_DELAY;
    tip->hide_delay_ms = 100;
    tip->showing = VENOM_FALSE;
    tip->popup_x = 0;
    tip->popup_y = 0;
    
    tip->bg_color = (VenomColor){ 50, 50, 50, 230 };
    tip->text_color = (VenomColor){ 255, 255, 255, 255 };
    tip->padding = DEFAULT_PADDING;
    tip->corner_radius = DEFAULT_RADIUS;
}

static void tooltip_destroy(VenomWidget* widget) {
    VenomTooltip* tip = (VenomTooltip*)widget;
    
    if (tip->child) {
        tip->child->parent = NULL;
        venom_unref(tip->child);
        tip->child = NULL;
    }
    
    if (tip->message) {
        venom_free(tip->message, strlen(tip->message) + 1);
        tip->message = NULL;
    }
    
    venom_widget_class.destroy(widget);
}

static void tooltip_measure(VenomWidget* widget, VenomF32 available_width, VenomF32 available_height,
                            VenomF32* out_width, VenomF32* out_height) {
    VenomTooltip* tip = (VenomTooltip*)widget;
    
    if (tip->child) {
        venom_widget_measure(tip->child, available_width, available_height, out_width, out_height);
    } else {
        *out_width = 0;
        *out_height = 0;
    }
}

static void tooltip_layout(VenomWidget* widget, VenomRectF bounds) {
    VenomTooltip* tip = (VenomTooltip*)widget;
    widget->bounds = bounds;
    
    if (tip->child && tip->child->visible) {
        VenomRectF child_bounds = { 0, 0, bounds.width, bounds.height };
        venom_widget_layout(tip->child, child_bounds);
    }
}

static void tooltip_draw(VenomWidget* widget, VenomCanvas* canvas) {
    VenomTooltip* tip = (VenomTooltip*)widget;
    
    /* Draw child first */
    if (tip->child && tip->child->visible) {
        venom_widget_draw(tip->child, canvas);
    }
    
    /* Draw tooltip popup if showing */
    if (tip->showing && tip->message && tip->message[0]) {
        VenomF32 text_width = (VenomF32)strlen(tip->message) * 7;  /* Approx */
        VenomF32 text_height = 16;
        
        VenomF32 popup_w = text_width + tip->padding * 2;
        VenomF32 popup_h = text_height + tip->padding * 2;
        
        /* Calculate position based on setting */
        VenomF32 px = 0, py = 0;
        
        switch (tip->position) {
            case VENOM_TOOLTIP_TOP:
                px = (widget->bounds.width - popup_w) / 2;
                py = -popup_h - 4;
                break;
            case VENOM_TOOLTIP_BOTTOM:
                px = (widget->bounds.width - popup_w) / 2;
                py = widget->bounds.height + 4;
                break;
            case VENOM_TOOLTIP_LEFT:
                px = -popup_w - 4;
                py = (widget->bounds.height - popup_h) / 2;
                break;
            case VENOM_TOOLTIP_RIGHT:
                px = widget->bounds.width + 4;
                py = (widget->bounds.height - popup_h) / 2;
                break;
            case VENOM_TOOLTIP_AUTO:
            default:
                px = (widget->bounds.width - popup_w) / 2;
                py = -popup_h - 4;
                break;
        }
        
        /* Draw popup background */
        VenomRectF popup = { px, py, popup_w, popup_h };
        VenomPaint bg_paint = venom_paint_fill(tip->bg_color);
        venom_canvas_draw_rounded_rect(canvas, popup, tip->corner_radius, &bg_paint);
        
        /* Draw text */
        VenomPaint text_paint = venom_paint_fill(tip->text_color);
        venom_canvas_draw_text(canvas, tip->message, px + tip->padding, py + tip->padding + text_height - 4, NULL, &text_paint);
    }
}

static VenomBool tooltip_on_event(VenomWidget* widget, const VenomEvent* event) {
    VenomTooltip* tip = (VenomTooltip*)widget;
    
    switch (event->type) {
        case VENOM_EVENT_MOUSE_ENTER:
            tip->showing = VENOM_TRUE;
            widget->needs_redraw = VENOM_TRUE;
            break;
            
        case VENOM_EVENT_MOUSE_LEAVE:
            tip->showing = VENOM_FALSE;
            widget->needs_redraw = VENOM_TRUE;
            break;
            
        default:
            break;
    }
    
    /* Pass events to child */
    if (tip->child && tip->child->visible) {
        return venom_widget_dispatch_event(tip->child, event);
    }
    
    return VENOM_FALSE;
}

const VenomWidgetClass venom_tooltip_class = {
    .class_name = "VenomTooltip",
    .instance_size = sizeof(VenomTooltip),
    .parent_class = &venom_widget_class,
    .init = tooltip_init,
    .destroy = tooltip_destroy,
    .measure = tooltip_measure,
    .layout = tooltip_layout,
    .draw = tooltip_draw,
    .on_event = tooltip_on_event,
    .on_state_changed = NULL,
};

VenomResultPtr venom_tooltip_create(void) {
    return venom_widget_create(&venom_tooltip_class);
}

VenomResult venom_tooltip_set_child(VenomTooltip* tip, VenomWidget* child) {
    VENOM_ENSURE_NOT_NULL(tip);
    
    if (tip->child) {
        tip->child->parent = NULL;
        venom_unref(tip->child);
    }
    
    tip->child = child;
    if (child) {
        venom_ref(child);
        child->parent = (VenomWidget*)tip;
    }
    
    venom_widget_invalidate_layout((VenomWidget*)tip);
    return VENOM_OK_UNIT();
}

void venom_tooltip_set_message(VenomTooltip* tip, const char* message) {
    if (!tip) return;
    
    if (tip->message) {
        venom_free(tip->message, strlen(tip->message) + 1);
        tip->message = NULL;
    }
    
    if (message) {
        VenomSize len = strlen(message) + 1;
        tip->message = (char*)venom_alloc(len);
        if (tip->message) {
            memcpy(tip->message, message, len);
        }
    }
}

void venom_tooltip_set_position(VenomTooltip* tip, VenomTooltipPosition pos) {
    if (tip) tip->position = pos;
}

void venom_tooltip_show(VenomTooltip* tip) {
    if (tip) {
        tip->showing = VENOM_TRUE;
        venom_widget_invalidate((VenomWidget*)tip);
    }
}

void venom_tooltip_hide(VenomTooltip* tip) {
    if (tip) {
        tip->showing = VENOM_FALSE;
        venom_widget_invalidate((VenomWidget*)tip);
    }
}

VenomWidget* _venom_tooltip_build(const VenomTooltipConfig* config) {
    VenomResultPtr result = venom_tooltip_create();
    if (!result.ok) return NULL;
    
    VenomTooltip* tip = (VenomTooltip*)result.value;
    
    if (config->child) venom_tooltip_set_child(tip, config->child);
    if (config->message) venom_tooltip_set_message(tip, config->message);
    tip->position = config->position;
    
    return (VenomWidget*)tip;
}
