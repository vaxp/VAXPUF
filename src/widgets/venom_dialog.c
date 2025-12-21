/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_dialog.c - Modal dialog implementation
 */

#include "venom/widgets/venom_dialog.h"
#include "venom/core/venom_memory.h"
#include <string.h>

#define DEFAULT_MIN_WIDTH 300.0f
#define DEFAULT_MAX_WIDTH 600.0f
#define DEFAULT_PADDING 24.0f
#define DEFAULT_RADIUS 8.0f

static void dialog_init(VenomWidget* widget) {
    VenomDialog* dialog = (VenomDialog*)widget;
    
    dialog->title = NULL;
    dialog->content = NULL;
    dialog->actions = NULL;
    
    dialog->is_open = VENOM_FALSE;
    dialog->modal = VENOM_TRUE;
    dialog->dismissible = VENOM_TRUE;
    
    dialog->min_width = DEFAULT_MIN_WIDTH;
    dialog->max_width = DEFAULT_MAX_WIDTH;
    dialog->min_height = 0;
    
    dialog->backdrop_color = (VenomColor){ 0, 0, 0, 128 };
    dialog->background_color = (VenomColor){ 255, 255, 255, 255 };
    dialog->corner_radius = DEFAULT_RADIUS;
    dialog->padding = DEFAULT_PADDING;
    dialog->title_padding = 16.0f;
    
    dialog->on_close = NULL;
    dialog->callback_data = NULL;
    dialog->result = VENOM_DIALOG_CANCEL;
}

static void dialog_destroy(VenomWidget* widget) {
    VenomDialog* dialog = (VenomDialog*)widget;
    
    if (dialog->title) {
        venom_free(dialog->title, strlen(dialog->title) + 1);
        dialog->title = NULL;
    }
    
    if (dialog->content) {
        dialog->content->parent = NULL;
        venom_unref(dialog->content);
        dialog->content = NULL;
    }
    
    if (dialog->actions) {
        dialog->actions->parent = NULL;
        venom_unref(dialog->actions);
        dialog->actions = NULL;
    }
    
    venom_widget_class.destroy(widget);
}

static void dialog_measure(VenomWidget* widget, VenomF32 available_width, VenomF32 available_height,
                           VenomF32* out_width, VenomF32* out_height) {
    /* Dialog takes full screen to draw backdrop */
    *out_width = available_width;
    *out_height = available_height;
}

static void dialog_layout(VenomWidget* widget, VenomRectF bounds) {
    VenomDialog* dialog = (VenomDialog*)widget;
    widget->bounds = bounds;
    
    if (!dialog->is_open) return;
    
    /* Calculate content size */
    VenomF32 content_w = 0, content_h = 0;
    VenomF32 title_h = dialog->title ? 30.0f : 0;
    VenomF32 actions_h = 0;
    
    if (dialog->content) {
        venom_widget_measure(dialog->content, 
                             dialog->max_width - dialog->padding * 2,
                             bounds.height - dialog->padding * 2 - title_h,
                             &content_w, &content_h);
    }
    
    if (dialog->actions) {
        VenomF32 actions_w;
        venom_widget_measure(dialog->actions, dialog->max_width - dialog->padding * 2, 50,
                             &actions_w, &actions_h);
    }
    
    /* Calculate dialog box size */
    VenomF32 dialog_w = content_w + dialog->padding * 2;
    if (dialog_w < dialog->min_width) dialog_w = dialog->min_width;
    if (dialog_w > dialog->max_width) dialog_w = dialog->max_width;
    
    VenomF32 dialog_h = title_h + content_h + actions_h + dialog->padding * 2;
    if (dialog_h < dialog->min_height) dialog_h = dialog->min_height;
    
    /* Center dialog */
    VenomF32 dx = (bounds.width - dialog_w) / 2;
    VenomF32 dy = (bounds.height - dialog_h) / 2;
    
    /* Layout content */
    VenomF32 cy = dialog->padding + title_h;
    
    if (dialog->content) {
        VenomRectF content_bounds = {
            dx + dialog->padding, dy + cy,
            dialog_w - dialog->padding * 2,
            content_h
        };
        venom_widget_layout(dialog->content, content_bounds);
        cy += content_h;
    }
    
    if (dialog->actions) {
        VenomRectF actions_bounds = {
            dx + dialog->padding, dy + dialog_h - dialog->padding - actions_h,
            dialog_w - dialog->padding * 2,
            actions_h
        };
        venom_widget_layout(dialog->actions, actions_bounds);
    }
}

static void dialog_draw(VenomWidget* widget, VenomCanvas* canvas) {
    VenomDialog* dialog = (VenomDialog*)widget;
    
    if (!dialog->is_open) return;
    
    VenomF32 w = widget->bounds.width;
    VenomF32 h = widget->bounds.height;
    
    /* Draw backdrop */
    if (dialog->modal) {
        VenomRectF backdrop = { 0, 0, w, h };
        VenomPaint backdrop_paint = venom_paint_fill(dialog->backdrop_color);
        venom_canvas_draw_rect(canvas, backdrop, &backdrop_paint);
    }
    
    /* Calculate dialog box */
    VenomF32 content_w = 0, content_h = 0;
    VenomF32 title_h = dialog->title ? 30.0f : 0;
    VenomF32 actions_h = dialog->actions ? 50.0f : 0;
    
    if (dialog->content) {
        venom_widget_measure(dialog->content, dialog->max_width, h, &content_w, &content_h);
    }
    
    VenomF32 dialog_w = content_w + dialog->padding * 2;
    if (dialog_w < dialog->min_width) dialog_w = dialog->min_width;
    if (dialog_w > dialog->max_width) dialog_w = dialog->max_width;
    
    VenomF32 dialog_h = title_h + content_h + actions_h + dialog->padding * 2;
    
    VenomF32 dx = (w - dialog_w) / 2;
    VenomF32 dy = (h - dialog_h) / 2;
    
    /* Draw dialog box shadow */
    VenomRectF shadow = { dx + 4, dy + 4, dialog_w, dialog_h };
    VenomPaint shadow_paint = venom_paint_fill((VenomColor){ 0, 0, 0, 50 });
    venom_canvas_draw_rounded_rect(canvas, shadow, dialog->corner_radius, &shadow_paint);
    
    /* Draw dialog box */
    VenomRectF box = { dx, dy, dialog_w, dialog_h };
    VenomPaint box_paint = venom_paint_fill(dialog->background_color);
    venom_canvas_draw_rounded_rect(canvas, box, dialog->corner_radius, &box_paint);
    
    /* Draw title */
    if (dialog->title) {
        VenomPaint title_paint = venom_paint_fill((VenomColor){ 33, 33, 33, 255 });
        venom_canvas_draw_text(canvas, dialog->title, 
                               dx + dialog->padding, 
                               dy + dialog->title_padding + 16, 
                               NULL, &title_paint);
    }
    
    /* Draw content */
    if (dialog->content && dialog->content->visible) {
        venom_canvas_save(canvas);
        venom_canvas_translate(canvas, dialog->content->bounds.x, dialog->content->bounds.y);
        venom_widget_draw(dialog->content, canvas);
        venom_canvas_restore(canvas);
    }
    
    /* Draw actions */
    if (dialog->actions && dialog->actions->visible) {
        venom_canvas_save(canvas);
        venom_canvas_translate(canvas, dialog->actions->bounds.x, dialog->actions->bounds.y);
        venom_widget_draw(dialog->actions, canvas);
        venom_canvas_restore(canvas);
    }
}

static VenomBool dialog_on_event(VenomWidget* widget, const VenomEvent* event) {
    VenomDialog* dialog = (VenomDialog*)widget;
    
    if (!dialog->is_open) return VENOM_FALSE;
    
    /* Handle ESC to close */
    if (event->type == VENOM_EVENT_KEY_DOWN && event->key.key == VENOM_KEY_ESCAPE) {
        if (dialog->dismissible) {
            venom_dialog_close(dialog, VENOM_DIALOG_CANCEL);
            return VENOM_TRUE;
        }
    }
    
    /* Pass events to actions */
    if (dialog->actions && dialog->actions->visible) {
        if (venom_widget_dispatch_event(dialog->actions, event)) {
            return VENOM_TRUE;
        }
    }
    
    /* Pass events to content */
    if (dialog->content && dialog->content->visible) {
        if (venom_widget_dispatch_event(dialog->content, event)) {
            return VENOM_TRUE;
        }
    }
    
    /* Click outside to dismiss (only for dismissible dialogs) */
    if (dialog->dismissible && dialog->modal && 
        event->type == VENOM_EVENT_MOUSE_BUTTON_DOWN) {
        /* Check if click is outside dialog box - simplified check */
        venom_dialog_close(dialog, VENOM_DIALOG_CANCEL);
        return VENOM_TRUE;
    }
    
    /* Modal dialogs consume all events */
    return dialog->modal;
}

const VenomWidgetClass venom_dialog_class = {
    .class_name = "VenomDialog",
    .instance_size = sizeof(VenomDialog),
    .parent_class = &venom_widget_class,
    .init = dialog_init,
    .destroy = dialog_destroy,
    .measure = dialog_measure,
    .layout = dialog_layout,
    .draw = dialog_draw,
    .on_event = dialog_on_event,
    .on_state_changed = NULL,
};

VenomResultPtr venom_dialog_create(void) {
    return venom_widget_create(&venom_dialog_class);
}

void venom_dialog_set_title(VenomDialog* dialog, const char* title) {
    if (!dialog) return;
    
    if (dialog->title) {
        venom_free(dialog->title, strlen(dialog->title) + 1);
        dialog->title = NULL;
    }
    
    if (title) {
        VenomSize len = strlen(title) + 1;
        dialog->title = (char*)venom_alloc(len);
        if (dialog->title) {
            memcpy(dialog->title, title, len);
        }
    }
}

VenomResult venom_dialog_set_content(VenomDialog* dialog, VenomWidget* content) {
    VENOM_ENSURE_NOT_NULL(dialog);
    
    if (dialog->content) {
        dialog->content->parent = NULL;
        venom_unref(dialog->content);
    }
    
    dialog->content = content;
    if (content) {
        venom_ref(content);
        content->parent = (VenomWidget*)dialog;
    }
    
    return VENOM_OK_UNIT();
}

VenomResult venom_dialog_add_action(VenomDialog* dialog, VenomWidget* button, int result) {
    VENOM_ENSURE_NOT_NULL(dialog);
    VENOM_ENSURE_NOT_NULL(button);
    (void)result;  /* TODO: Store result and trigger on button click */
    
    /* In a full implementation, we'd create a container for actions if needed */
    /* For now, just set the button as actions */
    if (dialog->actions) {
        dialog->actions->parent = NULL;
        venom_unref(dialog->actions);
    }
    
    dialog->actions = button;
    venom_ref(button);
    button->parent = (VenomWidget*)dialog;
    
    return VENOM_OK_UNIT();
}

void venom_dialog_open(VenomDialog* dialog) {
    if (dialog) {
        dialog->is_open = VENOM_TRUE;
        venom_widget_invalidate((VenomWidget*)dialog);
    }
}

void venom_dialog_close(VenomDialog* dialog, int result) {
    if (dialog) {
        dialog->is_open = VENOM_FALSE;
        dialog->result = result;
        
        if (dialog->on_close) {
            dialog->on_close(dialog, result, dialog->callback_data);
        }
        
        venom_widget_invalidate((VenomWidget*)dialog);
    }
}

VenomBool venom_dialog_is_open(const VenomDialog* dialog) {
    return dialog ? dialog->is_open : VENOM_FALSE;
}

void venom_dialog_set_on_close(VenomDialog* dialog, VenomDialogCallback callback, void* data) {
    if (dialog) {
        dialog->on_close = callback;
        dialog->callback_data = data;
    }
}

VenomWidget* _venom_dialog_build(const VenomDialogConfig* config) {
    VenomResultPtr result = venom_dialog_create();
    if (!result.ok) return NULL;
    
    VenomDialog* dialog = (VenomDialog*)result.value;
    
    if (config->title) venom_dialog_set_title(dialog, config->title);
    if (config->content) venom_dialog_set_content(dialog, config->content);
    dialog->modal = config->modal;
    dialog->on_close = config->on_close;
    dialog->callback_data = config->data;
    
    return (VenomWidget*)dialog;
}
