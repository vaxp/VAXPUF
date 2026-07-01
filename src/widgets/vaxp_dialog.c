/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_dialog.c - Modal dialog implementation
 */

#include "vaxp/widgets/vaxp_dialog.h"
#include "vaxp/core/vaxp_memory.h"
#include <string.h>

#define DEFAULT_MIN_WIDTH 300.0f
#define DEFAULT_MAX_WIDTH 600.0f
#define DEFAULT_PADDING 24.0f
#define DEFAULT_RADIUS 8.0f

static void dialog_init(VaxpWidget* widget) {
    VaxpDialog* dialog = (VaxpDialog*)widget;
    
    dialog->title = NULL;
    dialog->content = NULL;
    dialog->actions = NULL;
    
    dialog->is_open = VAXP_FALSE;
    dialog->modal = VAXP_TRUE;
    dialog->dismissible = VAXP_TRUE;
    
    dialog->min_width = DEFAULT_MIN_WIDTH;
    dialog->max_width = DEFAULT_MAX_WIDTH;
    dialog->min_height = 0;
    
    dialog->backdrop_color = (VaxpColor){ 0, 0, 0, 128 };
    dialog->background_color = (VaxpColor){ 255, 255, 255, 255 };
    dialog->corner_radius = DEFAULT_RADIUS;
    dialog->padding = DEFAULT_PADDING;
    dialog->title_padding = 16.0f;
    
    dialog->on_close = NULL;
    dialog->callback_data = NULL;
    dialog->result = VAXP_DIALOG_CANCEL;
}

static void dialog_destroy(VaxpWidget* widget) {
    VaxpDialog* dialog = (VaxpDialog*)widget;
    
    if (dialog->title) {
        vaxp_free(dialog->title, strlen(dialog->title) + 1);
        dialog->title = NULL;
    }
    
    if (dialog->content) {
        dialog->content->parent = NULL;
        vaxp_unref(dialog->content);
        dialog->content = NULL;
    }
    
    if (dialog->actions) {
        dialog->actions->parent = NULL;
        vaxp_unref(dialog->actions);
        dialog->actions = NULL;
    }
    
    vaxp_widget_class.destroy(widget);
}

static void dialog_measure(VaxpWidget* widget, VaxpF32 available_width, VaxpF32 available_height,
                           VaxpF32* out_width, VaxpF32* out_height) {
    /* Dialog takes full screen to draw backdrop */
    *out_width = available_width;
    *out_height = available_height;
}

static void dialog_layout(VaxpWidget* widget, VaxpRectF bounds) {
    VaxpDialog* dialog = (VaxpDialog*)widget;
    widget->bounds = bounds;
    
    if (!dialog->is_open) return;
    
    /* Calculate content size */
    VaxpF32 content_w = 0, content_h = 0;
    VaxpF32 title_h = dialog->title ? 30.0f : 0;
    VaxpF32 actions_h = 0;
    
    if (dialog->content) {
        vaxp_widget_measure(dialog->content, 
                             dialog->max_width - dialog->padding * 2,
                             bounds.height - dialog->padding * 2 - title_h,
                             &content_w, &content_h);
    }
    
    if (dialog->actions) {
        VaxpF32 actions_w;
        vaxp_widget_measure(dialog->actions, dialog->max_width - dialog->padding * 2, 50,
                             &actions_w, &actions_h);
    }
    
    /* Calculate dialog box size */
    VaxpF32 dialog_w = content_w + dialog->padding * 2;
    if (dialog_w < dialog->min_width) dialog_w = dialog->min_width;
    if (dialog_w > dialog->max_width) dialog_w = dialog->max_width;
    
    VaxpF32 dialog_h = title_h + content_h + actions_h + dialog->padding * 2;
    if (dialog_h < dialog->min_height) dialog_h = dialog->min_height;
    
    /* Center dialog */
    VaxpF32 dx = (bounds.width - dialog_w) / 2;
    VaxpF32 dy = (bounds.height - dialog_h) / 2;
    
    /* Layout content */
    VaxpF32 cy = dialog->padding + title_h;
    
    if (dialog->content) {
        VaxpRectF content_bounds = {
            dx + dialog->padding, dy + cy,
            dialog_w - dialog->padding * 2,
            content_h
        };
        vaxp_widget_layout(dialog->content, content_bounds);
        cy += content_h;
    }
    
    if (dialog->actions) {
        VaxpRectF actions_bounds = {
            dx + dialog->padding, dy + dialog_h - dialog->padding - actions_h,
            dialog_w - dialog->padding * 2,
            actions_h
        };
        vaxp_widget_layout(dialog->actions, actions_bounds);
    }
}

static void dialog_draw(VaxpWidget* widget, VaxpCanvas* canvas) {
    VaxpDialog* dialog = (VaxpDialog*)widget;
    
    if (!dialog->is_open) return;
    
    VaxpF32 w = widget->bounds.width;
    VaxpF32 h = widget->bounds.height;
    
    /* Draw backdrop */
    if (dialog->modal) {
        VaxpRectF backdrop = { 0, 0, w, h };
        VaxpPaint backdrop_paint = vaxp_paint_fill(dialog->backdrop_color);
        vaxp_canvas_draw_rect(canvas, backdrop, &backdrop_paint);
    }
    
    /* Calculate dialog box */
    VaxpF32 content_w = 0, content_h = 0;
    VaxpF32 title_h = dialog->title ? 30.0f : 0;
    VaxpF32 actions_h = dialog->actions ? 50.0f : 0;
    
    if (dialog->content) {
        vaxp_widget_measure(dialog->content, dialog->max_width, h, &content_w, &content_h);
    }
    
    VaxpF32 dialog_w = content_w + dialog->padding * 2;
    if (dialog_w < dialog->min_width) dialog_w = dialog->min_width;
    if (dialog_w > dialog->max_width) dialog_w = dialog->max_width;
    
    VaxpF32 dialog_h = title_h + content_h + actions_h + dialog->padding * 2;
    
    VaxpF32 dx = (w - dialog_w) / 2;
    VaxpF32 dy = (h - dialog_h) / 2;
    
    /* Draw dialog box shadow */
    VaxpRectF shadow = { dx + 4, dy + 4, dialog_w, dialog_h };
    VaxpPaint shadow_paint = vaxp_paint_fill((VaxpColor){ 0, 0, 0, 50 });
    vaxp_canvas_draw_rounded_rect(canvas, shadow, dialog->corner_radius, &shadow_paint);
    
    /* Draw dialog box */
    VaxpRectF box = { dx, dy, dialog_w, dialog_h };
    VaxpPaint box_paint = vaxp_paint_fill(dialog->background_color);
    vaxp_canvas_draw_rounded_rect(canvas, box, dialog->corner_radius, &box_paint);
    
    /* Draw title */
    if (dialog->title) {
        VaxpPaint title_paint = vaxp_paint_fill((VaxpColor){ 33, 33, 33, 255 });
        vaxp_canvas_draw_text(canvas, dialog->title, 
                               dx + dialog->padding, 
                               dy + dialog->title_padding + 16, 
                               NULL, &title_paint);
    }
    
    /* Draw content */
    if (dialog->content && dialog->content->visible) {
        vaxp_canvas_save(canvas);
        vaxp_canvas_translate(canvas, dialog->content->bounds.x, dialog->content->bounds.y);
        vaxp_widget_draw(dialog->content, canvas);
        vaxp_canvas_restore(canvas);
    }
    
    /* Draw actions */
    if (dialog->actions && dialog->actions->visible) {
        vaxp_canvas_save(canvas);
        vaxp_canvas_translate(canvas, dialog->actions->bounds.x, dialog->actions->bounds.y);
        vaxp_widget_draw(dialog->actions, canvas);
        vaxp_canvas_restore(canvas);
    }
}

static VaxpBool dialog_on_event(VaxpWidget* widget, const VaxpEvent* event) {
    VaxpDialog* dialog = (VaxpDialog*)widget;
    
    if (!dialog->is_open) return VAXP_FALSE;
    
    /* Handle ESC to close */
    if (event->type == VAXP_EVENT_KEY_DOWN && event->key.key == VAXP_KEY_ESCAPE) {
        if (dialog->dismissible) {
            vaxp_dialog_close(dialog, VAXP_DIALOG_CANCEL);
            return VAXP_TRUE;
        }
    }
    
    /* Pass events to actions */
    if (dialog->actions && dialog->actions->visible) {
        if (vaxp_widget_dispatch_event(dialog->actions, event)) {
            return VAXP_TRUE;
        }
    }
    
    /* Pass events to content */
    if (dialog->content && dialog->content->visible) {
        if (vaxp_widget_dispatch_event(dialog->content, event)) {
            return VAXP_TRUE;
        }
    }
    
    /* Click outside to dismiss (only for dismissible dialogs) */
    if (dialog->dismissible && dialog->modal && 
        event->type == VAXP_EVENT_MOUSE_BUTTON_DOWN) {
        /* Check if click is outside dialog box - simplified check */
        vaxp_dialog_close(dialog, VAXP_DIALOG_CANCEL);
        return VAXP_TRUE;
    }
    
    /* Modal dialogs consume all events */
    return dialog->modal;
}

const VaxpWidgetClass vaxp_dialog_class = {
    .class_name = "VaxpDialog",
    .instance_size = sizeof(VaxpDialog),
    .parent_class = &vaxp_widget_class,
    .init = dialog_init,
    .destroy = dialog_destroy,
    .measure = dialog_measure,
    .layout = dialog_layout,
    .draw = dialog_draw,
    .on_event = dialog_on_event,
    .on_state_changed = NULL,
};

VaxpResultPtr vaxp_dialog_create(void) {
    return vaxp_widget_create(&vaxp_dialog_class);
}

void vaxp_dialog_set_title(VaxpDialog* dialog, const char* title) {
    if (!dialog) return;
    
    if (dialog->title) {
        vaxp_free(dialog->title, strlen(dialog->title) + 1);
        dialog->title = NULL;
    }
    
    if (title) {
        VaxpSize len = strlen(title) + 1;
        dialog->title = (char*)vaxp_alloc(len);
        if (dialog->title) {
            memcpy(dialog->title, title, len);
        }
    }
}

VaxpResult vaxp_dialog_set_content(VaxpDialog* dialog, VaxpWidget* content) {
    VAXP_ENSURE_NOT_NULL(dialog);
    
    if (dialog->content) {
        dialog->content->parent = NULL;
        vaxp_unref(dialog->content);
    }
    
    dialog->content = content;
    if (content) {
        vaxp_ref(content);
        content->parent = (VaxpWidget*)dialog;
    }
    
    return VAXP_OK_UNIT();
}

VaxpResult vaxp_dialog_add_action(VaxpDialog* dialog, VaxpWidget* button, int result) {
    VAXP_ENSURE_NOT_NULL(dialog);
    VAXP_ENSURE_NOT_NULL(button);
    (void)result;  /* TODO: Store result and trigger on button click */
    
    /* In a full implementation, we'd create a container for actions if needed */
    /* For now, just set the button as actions */
    if (dialog->actions) {
        dialog->actions->parent = NULL;
        vaxp_unref(dialog->actions);
    }
    
    dialog->actions = button;
    vaxp_ref(button);
    button->parent = (VaxpWidget*)dialog;
    
    return VAXP_OK_UNIT();
}

void vaxp_dialog_open(VaxpDialog* dialog) {
    if (dialog) {
        dialog->is_open = VAXP_TRUE;
        vaxp_widget_invalidate((VaxpWidget*)dialog);
    }
}

void vaxp_dialog_close(VaxpDialog* dialog, int result) {
    if (dialog) {
        dialog->is_open = VAXP_FALSE;
        dialog->result = result;
        
        if (dialog->on_close) {
            dialog->on_close(dialog, result, dialog->callback_data);
        }
        
        vaxp_widget_invalidate((VaxpWidget*)dialog);
    }
}

VaxpBool vaxp_dialog_is_open(const VaxpDialog* dialog) {
    return dialog ? dialog->is_open : VAXP_FALSE;
}

void vaxp_dialog_set_on_close(VaxpDialog* dialog, VaxpDialogCallback callback, void* data) {
    if (dialog) {
        dialog->on_close = callback;
        dialog->callback_data = data;
    }
}

VaxpWidget* _vaxp_dialog_build(const VaxpDialogConfig* config) {
    VaxpResultPtr result = vaxp_dialog_create();
    if (!result.ok) return NULL;
    
    VaxpDialog* dialog = (VaxpDialog*)result.value;
    
    if (config->title) vaxp_dialog_set_title(dialog, config->title);
    if (config->content) vaxp_dialog_set_content(dialog, config->content);
    dialog->modal = config->modal;
    dialog->on_close = config->on_close;
    dialog->callback_data = config->data;
    
    return (VaxpWidget*)dialog;
}
