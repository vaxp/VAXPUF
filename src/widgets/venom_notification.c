/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_notification.c - Notification/Toast implementation
 */

#include "venom/widgets/venom_notification.h"
#include "venom/core/venom_memory.h"
#include <string.h>

#define DEFAULT_WIDTH 320.0f
#define DEFAULT_DURATION 4000
#define DEFAULT_PADDING 16.0f
#define DEFAULT_RADIUS 8.0f

static VenomColor get_type_color(VenomNotificationType type) {
    switch (type) {
        case VENOM_NOTIFY_INFO:    return (VenomColor){ 33, 150, 243, 255 };    /* Blue */
        case VENOM_NOTIFY_SUCCESS: return (VenomColor){ 76, 175, 80, 255 };     /* Green */
        case VENOM_NOTIFY_WARNING: return (VenomColor){ 255, 193, 7, 255 };     /* Amber */
        case VENOM_NOTIFY_ERROR:   return (VenomColor){ 244, 67, 54, 255 };     /* Red */
        default:                   return (VenomColor){ 97, 97, 97, 255 };
    }
}

static const char* get_type_icon(VenomNotificationType type) {
    switch (type) {
        case VENOM_NOTIFY_INFO:    return "ℹ";
        case VENOM_NOTIFY_SUCCESS: return "✓";
        case VENOM_NOTIFY_WARNING: return "⚠";
        case VENOM_NOTIFY_ERROR:   return "✕";
        default:                   return "•";
    }
}

static void notification_init(VenomWidget* widget) {
    VenomNotification* notif = (VenomNotification*)widget;
    
    notif->title = NULL;
    notif->message = NULL;
    notif->type = VENOM_NOTIFY_INFO;
    
    notif->is_showing = VENOM_FALSE;
    notif->dismissible = VENOM_TRUE;
    notif->duration_ms = DEFAULT_DURATION;
    notif->progress = 0;
    notif->position = VENOM_NOTIFY_TOP_RIGHT;
    
    notif->width = DEFAULT_WIDTH;
    notif->corner_radius = DEFAULT_RADIUS;
    notif->padding = DEFAULT_PADDING;
    notif->background_color = (VenomColor){ 50, 50, 50, 240 };
    notif->text_color = (VenomColor){ 255, 255, 255, 255 };
    notif->icon_color = get_type_color(notif->type);
    
    notif->on_dismiss = NULL;
    notif->callback_data = NULL;
}

static void notification_destroy(VenomWidget* widget) {
    VenomNotification* notif = (VenomNotification*)widget;
    
    if (notif->title) {
        venom_free(notif->title, strlen(notif->title) + 1);
        notif->title = NULL;
    }
    
    if (notif->message) {
        venom_free(notif->message, strlen(notif->message) + 1);
        notif->message = NULL;
    }
    
    venom_widget_class.destroy(widget);
}

static void notification_measure(VenomWidget* widget, VenomF32 available_width, VenomF32 available_height,
                                  VenomF32* out_width, VenomF32* out_height) {
    VenomNotification* notif = (VenomNotification*)widget;
    (void)available_width; (void)available_height;
    
    *out_width = notif->width;
    
    VenomF32 h = notif->padding * 2;
    if (notif->title) h += 24;
    if (notif->message) h += 20;
    if (notif->title && notif->message) h += 8;  /* Spacing */
    
    *out_height = h;
}

static void notification_draw(VenomWidget* widget, VenomCanvas* canvas) {
    VenomNotification* notif = (VenomNotification*)widget;
    
    if (!notif->is_showing) return;
    
    VenomF32 w = widget->bounds.width;
    VenomF32 h = widget->bounds.height;
    
    /* Draw shadow */
    VenomRectF shadow = { 3, 3, w, h };
    VenomPaint shadow_paint = venom_paint_fill((VenomColor){ 0, 0, 0, 60 });
    venom_canvas_draw_rounded_rect(canvas, shadow, notif->corner_radius, &shadow_paint);
    
    /* Draw background */
    VenomRectF bg = { 0, 0, w, h };
    VenomPaint bg_paint = venom_paint_fill(notif->background_color);
    venom_canvas_draw_rounded_rect(canvas, bg, notif->corner_radius, &bg_paint);
    
    /* Draw type indicator bar on left */
    VenomColor type_color = get_type_color(notif->type);
    VenomRectF indicator = { 0, 0, 4, h };
    VenomPaint ind_paint = venom_paint_fill(type_color);
    
    /* Clip to rounded corner on left */
    venom_canvas_save(canvas);
    venom_canvas_clip_rounded_rect(canvas, bg, notif->corner_radius);
    venom_canvas_draw_rect(canvas, indicator, &ind_paint);
    venom_canvas_restore(canvas);
    
    /* Draw icon */
    VenomF32 icon_x = notif->padding;
    VenomF32 icon_y = notif->padding + 18;
    VenomPaint icon_paint = venom_paint_fill(type_color);
    venom_canvas_draw_text(canvas, get_type_icon(notif->type), icon_x, icon_y, NULL, &icon_paint);
    
    /* Draw title */
    VenomF32 text_x = icon_x + 28;
    VenomF32 text_y = notif->padding + 18;
    
    if (notif->title) {
        VenomPaint title_paint = venom_paint_fill(notif->text_color);
        venom_canvas_draw_text(canvas, notif->title, text_x, text_y, NULL, &title_paint);
        text_y += 24;
    }
    
    /* Draw message */
    if (notif->message) {
        VenomColor msg_color = notif->text_color;
        msg_color.a = 200;
        VenomPaint msg_paint = venom_paint_fill(msg_color);
        venom_canvas_draw_text(canvas, notif->message, text_x, text_y, NULL, &msg_paint);
    }
    
    /* Draw dismiss button if dismissible */
    if (notif->dismissible) {
        VenomPaint close_paint = venom_paint_fill((VenomColor){ 255, 255, 255, 150 });
        venom_canvas_draw_text(canvas, "✕", w - notif->padding - 10, notif->padding + 18, NULL, &close_paint);
    }
    
    /* Draw progress bar for countdown */
    if (notif->duration_ms > 0 && notif->progress > 0) {
        VenomF32 prog_w = w * (1.0f - notif->progress);
        VenomRectF prog = { 0, h - 3, prog_w, 3 };
        VenomPaint prog_paint = venom_paint_fill(type_color);
        
        venom_canvas_save(canvas);
        venom_canvas_clip_rounded_rect(canvas, bg, notif->corner_radius);
        venom_canvas_draw_rect(canvas, prog, &prog_paint);
        venom_canvas_restore(canvas);
    }
}

static VenomBool notification_on_event(VenomWidget* widget, const VenomEvent* event) {
    VenomNotification* notif = (VenomNotification*)widget;
    
    if (!notif->is_showing) return VENOM_FALSE;
    
    if (event->type == VENOM_EVENT_MOUSE_BUTTON_DOWN) {
        if (notif->dismissible) {
            venom_notification_dismiss(notif);
            return VENOM_TRUE;
        }
    }
    
    return VENOM_FALSE;
}

const VenomWidgetClass venom_notification_class = {
    .class_name = "VenomNotification",
    .instance_size = sizeof(VenomNotification),
    .parent_class = &venom_widget_class,
    .init = notification_init,
    .destroy = notification_destroy,
    .measure = notification_measure,
    .layout = NULL,
    .draw = notification_draw,
    .on_event = notification_on_event,
    .on_state_changed = NULL,
};

VenomResultPtr venom_notification_create(void) {
    return venom_widget_create(&venom_notification_class);
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

void venom_notification_set_title(VenomNotification* notif, const char* title) {
    if (notif) set_string(&notif->title, title);
}

void venom_notification_set_message(VenomNotification* notif, const char* message) {
    if (notif) set_string(&notif->message, message);
}

void venom_notification_set_type(VenomNotification* notif, VenomNotificationType type) {
    if (notif) {
        notif->type = type;
        notif->icon_color = get_type_color(type);
    }
}

void venom_notification_set_duration(VenomNotification* notif, VenomU32 ms) {
    if (notif) notif->duration_ms = ms;
}

void venom_notification_set_position(VenomNotification* notif, VenomNotifyPosition pos) {
    if (notif) notif->position = pos;
}

void venom_notification_show(VenomNotification* notif) {
    if (notif) {
        notif->is_showing = VENOM_TRUE;
        notif->progress = 0;
        venom_widget_invalidate((VenomWidget*)notif);
    }
}

void venom_notification_dismiss(VenomNotification* notif) {
    if (notif) {
        notif->is_showing = VENOM_FALSE;
        
        if (notif->on_dismiss) {
            notif->on_dismiss(notif, notif->callback_data);
        }
        
        venom_widget_invalidate((VenomWidget*)notif);
    }
}

void venom_notification_set_on_dismiss(VenomNotification* notif, VenomNotifyCallback callback, void* data) {
    if (notif) {
        notif->on_dismiss = callback;
        notif->callback_data = data;
    }
}

VenomWidget* _venom_notification_build(const VenomNotificationConfig* config) {
    VenomResultPtr result = venom_notification_create();
    if (!result.ok) return NULL;
    
    VenomNotification* notif = (VenomNotification*)result.value;
    
    if (config->title) venom_notification_set_title(notif, config->title);
    if (config->message) venom_notification_set_message(notif, config->message);
    notif->type = config->type;
    notif->icon_color = get_type_color(config->type);
    if (config->duration_ms > 0) notif->duration_ms = config->duration_ms;
    notif->dismissible = config->dismissible;
    notif->position = config->position;
    
    return (VenomWidget*)notif;
}
