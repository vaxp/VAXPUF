/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_snackbar.c - Snackbar implementation
 */

#include "vaxp/widgets/vaxp_snackbar.h"
#include "vaxp/core/vaxp_memory.h"
#include <string.h>

#define DEFAULT_DURATION 4000
#define DEFAULT_MAX_WIDTH 400.0f

static void snackbar_init(VaxpWidget* widget) {
    VaxpSnackbar* bar = (VaxpSnackbar*)widget;
    
    bar->message = NULL;
    bar->action_label = NULL;
    bar->on_action = NULL;
    bar->action_data = NULL;
    
    bar->is_showing = VAXP_FALSE;
    bar->duration_ms = DEFAULT_DURATION;
    
    bar->background_color = (VaxpColor){ 50, 50, 50, 240 };
    bar->text_color = (VaxpColor){ 255, 255, 255, 255 };
    bar->action_color = (VaxpColor){ 130, 177, 255, 255 };
    bar->corner_radius = 4.0f;
    bar->max_width = DEFAULT_MAX_WIDTH;
}

static void snackbar_destroy(VaxpWidget* widget) {
    VaxpSnackbar* bar = (VaxpSnackbar*)widget;
    
    if (bar->message) {
        vaxp_free(bar->message, strlen(bar->message) + 1);
    }
    if (bar->action_label) {
        vaxp_free(bar->action_label, strlen(bar->action_label) + 1);
    }
    
    vaxp_widget_class.destroy(widget);
}

static void snackbar_measure(VaxpWidget* widget, VaxpF32 available_width, VaxpF32 available_height,
                             VaxpF32* out_width, VaxpF32* out_height) {
    VaxpSnackbar* bar = (VaxpSnackbar*)widget;
    (void)available_width; (void)available_height;
    
    VaxpF32 text_w = bar->message ? (VaxpF32)strlen(bar->message) * 8 : 0;
    VaxpF32 action_w = bar->action_label ? (VaxpF32)strlen(bar->action_label) * 9 + 24 : 0;
    
    *out_width = text_w + action_w + 32;
    if (*out_width > bar->max_width) *out_width = bar->max_width;
    
    *out_height = 48.0f;
}

static void snackbar_draw(VaxpWidget* widget, VaxpCanvas* canvas) {
    VaxpSnackbar* bar = (VaxpSnackbar*)widget;
    
    if (!bar->is_showing) return;
    
    VaxpF32 w = widget->bounds.width;
    VaxpF32 h = widget->bounds.height;
    
    /* Draw background */
    VaxpRectF bg = { 0, 0, w, h };
    VaxpPaint bg_paint = vaxp_paint_fill(bar->background_color);
    vaxp_canvas_draw_rounded_rect(canvas, bg, bar->corner_radius, &bg_paint);
    
    /* Draw message */
    if (bar->message) {
        VaxpPaint text_paint = vaxp_paint_fill(bar->text_color);
        vaxp_canvas_draw_text(canvas, bar->message, 16, h / 2 + 5, NULL, &text_paint);
    }
    
    /* Draw action button */
    if (bar->action_label) {
        VaxpPaint action_paint = vaxp_paint_fill(bar->action_color);
        VaxpF32 action_w = (VaxpF32)strlen(bar->action_label) * 9;
        vaxp_canvas_draw_text(canvas, bar->action_label, w - 16 - action_w, h / 2 + 5, NULL, &action_paint);
    }
}

static VaxpBool snackbar_on_event(VaxpWidget* widget, const VaxpEvent* event) {
    VaxpSnackbar* bar = (VaxpSnackbar*)widget;
    
    if (!bar->is_showing) return VAXP_FALSE;
    
    if (event->type == VAXP_EVENT_MOUSE_BUTTON_DOWN && bar->action_label) {
        VaxpF32 action_w = (VaxpF32)strlen(bar->action_label) * 9 + 32;
        if (event->mouse.x > widget->bounds.width - action_w) {
            if (bar->on_action) {
                bar->on_action(bar, bar->action_data);
            }
            vaxp_snackbar_hide(bar);
            return VAXP_TRUE;
        }
    }
    
    return VAXP_FALSE;
}

const VaxpWidgetClass vaxp_snackbar_class = {
    .class_name = "VaxpSnackbar",
    .instance_size = sizeof(VaxpSnackbar),
    .parent_class = &vaxp_widget_class,
    .init = snackbar_init,
    .destroy = snackbar_destroy,
    .measure = snackbar_measure,
    .layout = NULL,
    .draw = snackbar_draw,
    .on_event = snackbar_on_event,
    .on_state_changed = NULL,
};

VaxpResultPtr vaxp_snackbar_create(void) {
    return vaxp_widget_create(&vaxp_snackbar_class);
}

static void set_string(char** dest, const char* src) {
    if (*dest) {
        vaxp_free(*dest, strlen(*dest) + 1);
        *dest = NULL;
    }
    if (src) {
        VaxpSize len = strlen(src) + 1;
        *dest = (char*)vaxp_alloc(len);
        if (*dest) memcpy(*dest, src, len);
    }
}

void vaxp_snackbar_set_message(VaxpSnackbar* bar, const char* message) {
    if (bar) set_string(&bar->message, message);
}

void vaxp_snackbar_set_action(VaxpSnackbar* bar, const char* label, VaxpSnackbarAction action, void* data) {
    if (bar) {
        set_string(&bar->action_label, label);
        bar->on_action = action;
        bar->action_data = data;
    }
}

void vaxp_snackbar_show(VaxpSnackbar* bar) {
    if (bar) {
        bar->is_showing = VAXP_TRUE;
        vaxp_widget_invalidate((VaxpWidget*)bar);
    }
}

void vaxp_snackbar_hide(VaxpSnackbar* bar) {
    if (bar) {
        bar->is_showing = VAXP_FALSE;
        vaxp_widget_invalidate((VaxpWidget*)bar);
    }
}

VaxpWidget* _vaxp_snackbar_build(const VaxpSnackbarConfig* config) {
    VaxpResultPtr result = vaxp_snackbar_create();
    if (!result.ok) return NULL;
    
    VaxpSnackbar* bar = (VaxpSnackbar*)result.value;
    
    if (config->message) vaxp_snackbar_set_message(bar, config->message);
    if (config->action) vaxp_snackbar_set_action(bar, config->action, config->on_action, config->data);
    if (config->duration > 0) bar->duration_ms = config->duration;
    
    return (VaxpWidget*)bar;
}
