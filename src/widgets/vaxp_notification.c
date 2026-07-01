/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_notification.c - Notification/Toast implementation
 */

#include "vaxp/widgets/vaxp_notification.h"
#include "vaxp/core/vaxp_memory.h"
#include <string.h>

#define DEFAULT_WIDTH 320.0f
#define DEFAULT_DURATION 4000
#define DEFAULT_PADDING 16.0f
#define DEFAULT_RADIUS 8.0f

static VaxpColor get_type_color(VaxpNotificationType type) {
    switch (type) {
        case VAXP_NOTIFY_INFO:    return (VaxpColor){ 33, 150, 243, 255 };    /* Blue */
        case VAXP_NOTIFY_SUCCESS: return (VaxpColor){ 76, 175, 80, 255 };     /* Green */
        case VAXP_NOTIFY_WARNING: return (VaxpColor){ 255, 193, 7, 255 };     /* Amber */
        case VAXP_NOTIFY_ERROR:   return (VaxpColor){ 244, 67, 54, 255 };     /* Red */
        default:                   return (VaxpColor){ 97, 97, 97, 255 };
    }
}

static const char* get_type_icon(VaxpNotificationType type) {
    switch (type) {
        case VAXP_NOTIFY_INFO:    return "ℹ";
        case VAXP_NOTIFY_SUCCESS: return "✓";
        case VAXP_NOTIFY_WARNING: return "⚠";
        case VAXP_NOTIFY_ERROR:   return "✕";
        default:                   return "•";
    }
}

static void notification_init(VaxpWidget* widget) {
    VaxpNotification* notif = (VaxpNotification*)widget;
    
    notif->title = NULL;
    notif->message = NULL;
    notif->type = VAXP_NOTIFY_INFO;
    
    notif->is_showing = VAXP_FALSE;
    notif->dismissible = VAXP_TRUE;
    notif->duration_ms = DEFAULT_DURATION;
    notif->progress = 0;
    notif->position = VAXP_NOTIFY_TOP_RIGHT;
    
    notif->width = DEFAULT_WIDTH;
    notif->corner_radius = DEFAULT_RADIUS;
    notif->padding = DEFAULT_PADDING;
    notif->background_color = (VaxpColor){ 50, 50, 50, 240 };
    notif->text_color = (VaxpColor){ 255, 255, 255, 255 };
    notif->icon_color = get_type_color(notif->type);
    
    notif->on_dismiss = NULL;
    notif->callback_data = NULL;
}

static void notification_destroy(VaxpWidget* widget) {
    VaxpNotification* notif = (VaxpNotification*)widget;
    
    if (notif->title) {
        vaxp_free(notif->title, strlen(notif->title) + 1);
        notif->title = NULL;
    }
    
    if (notif->message) {
        vaxp_free(notif->message, strlen(notif->message) + 1);
        notif->message = NULL;
    }
    
    vaxp_widget_class.destroy(widget);
}

static void notification_measure(VaxpWidget* widget, VaxpF32 available_width, VaxpF32 available_height,
                                  VaxpF32* out_width, VaxpF32* out_height) {
    VaxpNotification* notif = (VaxpNotification*)widget;
    (void)available_width; (void)available_height;
    
    *out_width = notif->width;
    
    VaxpF32 h = notif->padding * 2;
    if (notif->title) h += 24;
    if (notif->message) h += 20;
    if (notif->title && notif->message) h += 8;  /* Spacing */
    
    *out_height = h;
}

static void notification_draw(VaxpWidget* widget, VaxpCanvas* canvas) {
    VaxpNotification* notif = (VaxpNotification*)widget;
    
    if (!notif->is_showing) return;
    
    VaxpF32 w = widget->bounds.width;
    VaxpF32 h = widget->bounds.height;
    
    /* Draw shadow */
    VaxpRectF shadow = { 3, 3, w, h };
    VaxpPaint shadow_paint = vaxp_paint_fill((VaxpColor){ 0, 0, 0, 60 });
    vaxp_canvas_draw_rounded_rect(canvas, shadow, notif->corner_radius, &shadow_paint);
    
    /* Draw background */
    VaxpRectF bg = { 0, 0, w, h };
    VaxpPaint bg_paint = vaxp_paint_fill(notif->background_color);
    vaxp_canvas_draw_rounded_rect(canvas, bg, notif->corner_radius, &bg_paint);
    
    /* Draw type indicator bar on left */
    VaxpColor type_color = get_type_color(notif->type);
    VaxpRectF indicator = { 0, 0, 4, h };
    VaxpPaint ind_paint = vaxp_paint_fill(type_color);
    
    /* Clip to rounded corner on left */
    vaxp_canvas_save(canvas);
    vaxp_canvas_clip_rounded_rect(canvas, bg, notif->corner_radius);
    vaxp_canvas_draw_rect(canvas, indicator, &ind_paint);
    vaxp_canvas_restore(canvas);
    
    /* Draw icon */
    VaxpF32 icon_x = notif->padding;
    VaxpF32 icon_y = notif->padding + 18;
    VaxpPaint icon_paint = vaxp_paint_fill(type_color);
    vaxp_canvas_draw_text(canvas, get_type_icon(notif->type), icon_x, icon_y, NULL, &icon_paint);
    
    /* Draw title */
    VaxpF32 text_x = icon_x + 28;
    VaxpF32 text_y = notif->padding + 18;
    
    if (notif->title) {
        VaxpPaint title_paint = vaxp_paint_fill(notif->text_color);
        vaxp_canvas_draw_text(canvas, notif->title, text_x, text_y, NULL, &title_paint);
        text_y += 24;
    }
    
    /* Draw message */
    if (notif->message) {
        VaxpColor msg_color = notif->text_color;
        msg_color.a = 200;
        VaxpPaint msg_paint = vaxp_paint_fill(msg_color);
        vaxp_canvas_draw_text(canvas, notif->message, text_x, text_y, NULL, &msg_paint);
    }
    
    /* Draw dismiss button if dismissible */
    if (notif->dismissible) {
        VaxpPaint close_paint = vaxp_paint_fill((VaxpColor){ 255, 255, 255, 150 });
        vaxp_canvas_draw_text(canvas, "✕", w - notif->padding - 10, notif->padding + 18, NULL, &close_paint);
    }
    
    /* Draw progress bar for countdown */
    if (notif->duration_ms > 0 && notif->progress > 0) {
        VaxpF32 prog_w = w * (1.0f - notif->progress);
        VaxpRectF prog = { 0, h - 3, prog_w, 3 };
        VaxpPaint prog_paint = vaxp_paint_fill(type_color);
        
        vaxp_canvas_save(canvas);
        vaxp_canvas_clip_rounded_rect(canvas, bg, notif->corner_radius);
        vaxp_canvas_draw_rect(canvas, prog, &prog_paint);
        vaxp_canvas_restore(canvas);
    }
}

static VaxpBool notification_on_event(VaxpWidget* widget, const VaxpEvent* event) {
    VaxpNotification* notif = (VaxpNotification*)widget;
    
    if (!notif->is_showing) return VAXP_FALSE;
    
    if (event->type == VAXP_EVENT_MOUSE_BUTTON_DOWN) {
        if (notif->dismissible) {
            vaxp_notification_dismiss(notif);
            return VAXP_TRUE;
        }
    }
    
    return VAXP_FALSE;
}

const VaxpWidgetClass vaxp_notification_class = {
    .class_name = "VaxpNotification",
    .instance_size = sizeof(VaxpNotification),
    .parent_class = &vaxp_widget_class,
    .init = notification_init,
    .destroy = notification_destroy,
    .measure = notification_measure,
    .layout = NULL,
    .draw = notification_draw,
    .on_event = notification_on_event,
    .on_state_changed = NULL,
};

VaxpResultPtr vaxp_notification_create(void) {
    return vaxp_widget_create(&vaxp_notification_class);
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

void vaxp_notification_set_title(VaxpNotification* notif, const char* title) {
    if (notif) set_string(&notif->title, title);
}

void vaxp_notification_set_message(VaxpNotification* notif, const char* message) {
    if (notif) set_string(&notif->message, message);
}

void vaxp_notification_set_type(VaxpNotification* notif, VaxpNotificationType type) {
    if (notif) {
        notif->type = type;
        notif->icon_color = get_type_color(type);
    }
}

void vaxp_notification_set_duration(VaxpNotification* notif, VaxpU32 ms) {
    if (notif) notif->duration_ms = ms;
}

void vaxp_notification_set_position(VaxpNotification* notif, VaxpNotifyPosition pos) {
    if (notif) notif->position = pos;
}

void vaxp_notification_show(VaxpNotification* notif) {
    if (notif) {
        notif->is_showing = VAXP_TRUE;
        notif->progress = 0;
        vaxp_widget_invalidate((VaxpWidget*)notif);
    }
}

void vaxp_notification_dismiss(VaxpNotification* notif) {
    if (notif) {
        notif->is_showing = VAXP_FALSE;
        
        if (notif->on_dismiss) {
            notif->on_dismiss(notif, notif->callback_data);
        }
        
        vaxp_widget_invalidate((VaxpWidget*)notif);
    }
}

void vaxp_notification_set_on_dismiss(VaxpNotification* notif, VaxpNotifyCallback callback, void* data) {
    if (notif) {
        notif->on_dismiss = callback;
        notif->callback_data = data;
    }
}

VaxpWidget* _vaxp_notification_build(const VaxpNotificationConfig* config) {
    VaxpResultPtr result = vaxp_notification_create();
    if (!result.ok) return NULL;
    
    VaxpNotification* notif = (VaxpNotification*)result.value;
    
    if (config->title) vaxp_notification_set_title(notif, config->title);
    if (config->message) vaxp_notification_set_message(notif, config->message);
    notif->type = config->type;
    notif->icon_color = get_type_color(config->type);
    if (config->duration_ms > 0) notif->duration_ms = config->duration_ms;
    notif->dismissible = config->dismissible;
    notif->position = config->position;
    
    return (VaxpWidget*)notif;
}
