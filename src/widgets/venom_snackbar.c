/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_snackbar.c - Snackbar implementation
 */

#include "venom/widgets/venom_snackbar.h"
#include "venom/core/venom_memory.h"
#include <string.h>

#define DEFAULT_DURATION 4000
#define DEFAULT_MAX_WIDTH 400.0f

static void snackbar_init(VenomWidget* widget) {
    VenomSnackbar* bar = (VenomSnackbar*)widget;
    
    bar->message = NULL;
    bar->action_label = NULL;
    bar->on_action = NULL;
    bar->action_data = NULL;
    
    bar->is_showing = VENOM_FALSE;
    bar->duration_ms = DEFAULT_DURATION;
    
    bar->background_color = (VenomColor){ 50, 50, 50, 240 };
    bar->text_color = (VenomColor){ 255, 255, 255, 255 };
    bar->action_color = (VenomColor){ 130, 177, 255, 255 };
    bar->corner_radius = 4.0f;
    bar->max_width = DEFAULT_MAX_WIDTH;
}

static void snackbar_destroy(VenomWidget* widget) {
    VenomSnackbar* bar = (VenomSnackbar*)widget;
    
    if (bar->message) {
        venom_free(bar->message, strlen(bar->message) + 1);
    }
    if (bar->action_label) {
        venom_free(bar->action_label, strlen(bar->action_label) + 1);
    }
    
    venom_widget_class.destroy(widget);
}

static void snackbar_measure(VenomWidget* widget, VenomF32 available_width, VenomF32 available_height,
                             VenomF32* out_width, VenomF32* out_height) {
    VenomSnackbar* bar = (VenomSnackbar*)widget;
    (void)available_width; (void)available_height;
    
    VenomF32 text_w = bar->message ? (VenomF32)strlen(bar->message) * 8 : 0;
    VenomF32 action_w = bar->action_label ? (VenomF32)strlen(bar->action_label) * 9 + 24 : 0;
    
    *out_width = text_w + action_w + 32;
    if (*out_width > bar->max_width) *out_width = bar->max_width;
    
    *out_height = 48.0f;
}

static void snackbar_draw(VenomWidget* widget, VenomCanvas* canvas) {
    VenomSnackbar* bar = (VenomSnackbar*)widget;
    
    if (!bar->is_showing) return;
    
    VenomF32 w = widget->bounds.width;
    VenomF32 h = widget->bounds.height;
    
    /* Draw background */
    VenomRectF bg = { 0, 0, w, h };
    VenomPaint bg_paint = venom_paint_fill(bar->background_color);
    venom_canvas_draw_rounded_rect(canvas, bg, bar->corner_radius, &bg_paint);
    
    /* Draw message */
    if (bar->message) {
        VenomPaint text_paint = venom_paint_fill(bar->text_color);
        venom_canvas_draw_text(canvas, bar->message, 16, h / 2 + 5, NULL, &text_paint);
    }
    
    /* Draw action button */
    if (bar->action_label) {
        VenomPaint action_paint = venom_paint_fill(bar->action_color);
        VenomF32 action_w = (VenomF32)strlen(bar->action_label) * 9;
        venom_canvas_draw_text(canvas, bar->action_label, w - 16 - action_w, h / 2 + 5, NULL, &action_paint);
    }
}

static VenomBool snackbar_on_event(VenomWidget* widget, const VenomEvent* event) {
    VenomSnackbar* bar = (VenomSnackbar*)widget;
    
    if (!bar->is_showing) return VENOM_FALSE;
    
    if (event->type == VENOM_EVENT_MOUSE_BUTTON_DOWN && bar->action_label) {
        VenomF32 action_w = (VenomF32)strlen(bar->action_label) * 9 + 32;
        if (event->mouse.x > widget->bounds.width - action_w) {
            if (bar->on_action) {
                bar->on_action(bar, bar->action_data);
            }
            venom_snackbar_hide(bar);
            return VENOM_TRUE;
        }
    }
    
    return VENOM_FALSE;
}

const VenomWidgetClass venom_snackbar_class = {
    .class_name = "VenomSnackbar",
    .instance_size = sizeof(VenomSnackbar),
    .parent_class = &venom_widget_class,
    .init = snackbar_init,
    .destroy = snackbar_destroy,
    .measure = snackbar_measure,
    .layout = NULL,
    .draw = snackbar_draw,
    .on_event = snackbar_on_event,
    .on_state_changed = NULL,
};

VenomResultPtr venom_snackbar_create(void) {
    return venom_widget_create(&venom_snackbar_class);
}

static void set_string(char** dest, const char* src) {
    if (*dest) {
        venom_free(*dest, strlen(*dest) + 1);
        *dest = NULL;
    }
    if (src) {
        VenomSize len = strlen(src) + 1;
        *dest = (char*)venom_alloc(len);
        if (*dest) memcpy(*dest, src, len);
    }
}

void venom_snackbar_set_message(VenomSnackbar* bar, const char* message) {
    if (bar) set_string(&bar->message, message);
}

void venom_snackbar_set_action(VenomSnackbar* bar, const char* label, VenomSnackbarAction action, void* data) {
    if (bar) {
        set_string(&bar->action_label, label);
        bar->on_action = action;
        bar->action_data = data;
    }
}

void venom_snackbar_show(VenomSnackbar* bar) {
    if (bar) {
        bar->is_showing = VENOM_TRUE;
        venom_widget_invalidate((VenomWidget*)bar);
    }
}

void venom_snackbar_hide(VenomSnackbar* bar) {
    if (bar) {
        bar->is_showing = VENOM_FALSE;
        venom_widget_invalidate((VenomWidget*)bar);
    }
}

VenomWidget* _venom_snackbar_build(const VenomSnackbarConfig* config) {
    VenomResultPtr result = venom_snackbar_create();
    if (!result.ok) return NULL;
    
    VenomSnackbar* bar = (VenomSnackbar*)result.value;
    
    if (config->message) venom_snackbar_set_message(bar, config->message);
    if (config->action) venom_snackbar_set_action(bar, config->action, config->on_action, config->data);
    if (config->duration > 0) bar->duration_ms = config->duration;
    
    return (VenomWidget*)bar;
}
