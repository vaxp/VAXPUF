/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_tooltip.c - Tooltip widget implementation
 */

#include "vaxp/widgets/vaxp_tooltip.h"
#include "vaxp/core/vaxp_memory.h"
#include <string.h>

#define DEFAULT_SHOW_DELAY 500
#define DEFAULT_PADDING 8.0f
#define DEFAULT_RADIUS 4.0f

static void tooltip_init(VaxpWidget* widget) {
    VaxpTooltip* tip = (VaxpTooltip*)widget;
    
    tip->child = NULL;
    tip->message = NULL;
    tip->position = VAXP_TOOLTIP_TOP;
    tip->show_delay_ms = DEFAULT_SHOW_DELAY;
    tip->hide_delay_ms = 100;
    tip->showing = VAXP_FALSE;
    tip->popup_x = 0;
    tip->popup_y = 0;
    
    tip->bg_color = (VaxpColor){ 50, 50, 50, 230 };
    tip->text_color = (VaxpColor){ 255, 255, 255, 255 };
    tip->padding = DEFAULT_PADDING;
    tip->corner_radius = DEFAULT_RADIUS;
}

static void tooltip_destroy(VaxpWidget* widget) {
    VaxpTooltip* tip = (VaxpTooltip*)widget;
    
    if (tip->child) {
        tip->child->parent = NULL;
        vaxp_unref(tip->child);
        tip->child = NULL;
    }
    
    if (tip->message) {
        vaxp_free(tip->message, strlen(tip->message) + 1);
        tip->message = NULL;
    }
    
    vaxp_widget_class.destroy(widget);
}

static void tooltip_measure(VaxpWidget* widget, VaxpF32 available_width, VaxpF32 available_height,
                            VaxpF32* out_width, VaxpF32* out_height) {
    VaxpTooltip* tip = (VaxpTooltip*)widget;
    
    if (tip->child) {
        vaxp_widget_measure(tip->child, available_width, available_height, out_width, out_height);
    } else {
        *out_width = 0;
        *out_height = 0;
    }
}

static void tooltip_layout(VaxpWidget* widget, VaxpRectF bounds) {
    VaxpTooltip* tip = (VaxpTooltip*)widget;
    widget->bounds = bounds;
    
    if (tip->child && tip->child->visible) {
        VaxpRectF child_bounds = { 0, 0, bounds.width, bounds.height };
        vaxp_widget_layout(tip->child, child_bounds);
    }
}

static void tooltip_draw(VaxpWidget* widget, VaxpCanvas* canvas) {
    VaxpTooltip* tip = (VaxpTooltip*)widget;
    
    /* Draw child first */
    if (tip->child && tip->child->visible) {
        vaxp_widget_draw(tip->child, canvas);
    }
    
    /* Draw tooltip popup if showing */
    if (tip->showing && tip->message && tip->message[0]) {
        VaxpF32 text_width = (VaxpF32)strlen(tip->message) * 7;  /* Approx */
        VaxpF32 text_height = 16;
        
        VaxpF32 popup_w = text_width + tip->padding * 2;
        VaxpF32 popup_h = text_height + tip->padding * 2;
        
        /* Calculate position based on setting */
        VaxpF32 px = 0, py = 0;
        
        switch (tip->position) {
            case VAXP_TOOLTIP_TOP:
                px = (widget->bounds.width - popup_w) / 2;
                py = -popup_h - 4;
                break;
            case VAXP_TOOLTIP_BOTTOM:
                px = (widget->bounds.width - popup_w) / 2;
                py = widget->bounds.height + 4;
                break;
            case VAXP_TOOLTIP_LEFT:
                px = -popup_w - 4;
                py = (widget->bounds.height - popup_h) / 2;
                break;
            case VAXP_TOOLTIP_RIGHT:
                px = widget->bounds.width + 4;
                py = (widget->bounds.height - popup_h) / 2;
                break;
            case VAXP_TOOLTIP_AUTO:
            default:
                px = (widget->bounds.width - popup_w) / 2;
                py = -popup_h - 4;
                break;
        }
        
        /* Draw popup background */
        VaxpRectF popup = { px, py, popup_w, popup_h };
        VaxpPaint bg_paint = vaxp_paint_fill(tip->bg_color);
        vaxp_canvas_draw_rounded_rect(canvas, popup, tip->corner_radius, &bg_paint);
        
        /* Draw text */
        VaxpPaint text_paint = vaxp_paint_fill(tip->text_color);
        vaxp_canvas_draw_text(canvas, tip->message, px + tip->padding, py + tip->padding + text_height - 4, NULL, &text_paint);
    }
}

static VaxpBool tooltip_on_event(VaxpWidget* widget, const VaxpEvent* event) {
    VaxpTooltip* tip = (VaxpTooltip*)widget;
    
    switch (event->type) {
        case VAXP_EVENT_MOUSE_ENTER:
            tip->showing = VAXP_TRUE;
            widget->needs_redraw = VAXP_TRUE;
            break;
            
        case VAXP_EVENT_MOUSE_LEAVE:
            tip->showing = VAXP_FALSE;
            widget->needs_redraw = VAXP_TRUE;
            break;
            
        default:
            break;
    }
    
    /* Pass events to child */
    if (tip->child && tip->child->visible) {
        return vaxp_widget_dispatch_event(tip->child, event);
    }
    
    return VAXP_FALSE;
}

const VaxpWidgetClass vaxp_tooltip_class = {
    .class_name = "VaxpTooltip",
    .instance_size = sizeof(VaxpTooltip),
    .parent_class = &vaxp_widget_class,
    .init = tooltip_init,
    .destroy = tooltip_destroy,
    .measure = tooltip_measure,
    .layout = tooltip_layout,
    .draw = tooltip_draw,
    .on_event = tooltip_on_event,
    .on_state_changed = NULL,
};

VaxpResultPtr vaxp_tooltip_create(void) {
    return vaxp_widget_create(&vaxp_tooltip_class);
}

VaxpResult vaxp_tooltip_set_child(VaxpTooltip* tip, VaxpWidget* child) {
    VAXP_ENSURE_NOT_NULL(tip);
    
    if (tip->child) {
        tip->child->parent = NULL;
        vaxp_unref(tip->child);
    }
    
    tip->child = child;
    if (child) {
        vaxp_ref(child);
        child->parent = (VaxpWidget*)tip;
    }
    
    vaxp_widget_invalidate_layout((VaxpWidget*)tip);
    return VAXP_OK_UNIT();
}

void vaxp_tooltip_set_message(VaxpTooltip* tip, const char* message) {
    if (!tip) return;
    
    if (tip->message) {
        vaxp_free(tip->message, strlen(tip->message) + 1);
        tip->message = NULL;
    }
    
    if (message) {
        VaxpSize len = strlen(message) + 1;
        tip->message = (char*)vaxp_alloc(len);
        if (tip->message) {
            memcpy(tip->message, message, len);
        }
    }
}

void vaxp_tooltip_set_position(VaxpTooltip* tip, VaxpTooltipPosition pos) {
    if (tip) tip->position = pos;
}

void vaxp_tooltip_show(VaxpTooltip* tip) {
    if (tip) {
        tip->showing = VAXP_TRUE;
        vaxp_widget_invalidate((VaxpWidget*)tip);
    }
}

void vaxp_tooltip_hide(VaxpTooltip* tip) {
    if (tip) {
        tip->showing = VAXP_FALSE;
        vaxp_widget_invalidate((VaxpWidget*)tip);
    }
}

VaxpWidget* _vaxp_tooltip_build(const VaxpTooltipConfig* config) {
    VaxpResultPtr result = vaxp_tooltip_create();
    if (!result.ok) return NULL;
    
    VaxpTooltip* tip = (VaxpTooltip*)result.value;
    
    if (config->child) vaxp_tooltip_set_child(tip, config->child);
    if (config->message) vaxp_tooltip_set_message(tip, config->message);
    tip->position = config->position;
    
    return (VaxpWidget*)tip;
}
