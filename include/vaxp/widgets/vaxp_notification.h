/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_notification.h - Notification/Toast widget
 */

#ifndef VAXP_NOTIFICATION_H
#define VAXP_NOTIFICATION_H

#include "vaxp/widgets/vaxp_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum VaxpNotificationType {
    VAXP_NOTIFY_INFO,
    VAXP_NOTIFY_SUCCESS,
    VAXP_NOTIFY_WARNING,
    VAXP_NOTIFY_ERROR,
} VaxpNotificationType;

typedef enum VaxpNotifyPosition {
    VAXP_NOTIFY_TOP_LEFT,
    VAXP_NOTIFY_TOP_CENTER,
    VAXP_NOTIFY_TOP_RIGHT,
    VAXP_NOTIFY_BOTTOM_LEFT,
    VAXP_NOTIFY_BOTTOM_CENTER,
    VAXP_NOTIFY_BOTTOM_RIGHT,
} VaxpNotifyPosition;

typedef struct VaxpNotification VaxpNotification;

typedef void (*VaxpNotifyCallback)(VaxpNotification* notif, void* user_data);

struct VaxpNotification {
    VaxpWidget base;
    
    char* title;
    char* message;
    VaxpNotificationType type;
    
    VaxpBool is_showing;
    VaxpBool dismissible;
    VaxpU32 duration_ms;        /* 0 = no auto-dismiss */
    VaxpF32 progress;           /* For auto-dismiss countdown */
    VaxpNotifyPosition position;
    
    /* Styling */
    VaxpF32 width;
    VaxpF32 corner_radius;
    VaxpF32 padding;
    VaxpColor background_color;
    VaxpColor text_color;
    VaxpColor icon_color;
    
    /* Callbacks */
    VaxpNotifyCallback on_dismiss;
    void* callback_data;
};

VaxpResultPtr vaxp_notification_create(void);
void vaxp_notification_set_title(VaxpNotification* notif, const char* title);
void vaxp_notification_set_message(VaxpNotification* notif, const char* message);
void vaxp_notification_set_type(VaxpNotification* notif, VaxpNotificationType type);
void vaxp_notification_set_duration(VaxpNotification* notif, VaxpU32 ms);
void vaxp_notification_set_position(VaxpNotification* notif, VaxpNotifyPosition pos);
void vaxp_notification_show(VaxpNotification* notif);
void vaxp_notification_dismiss(VaxpNotification* notif);
void vaxp_notification_set_on_dismiss(VaxpNotification* notif, VaxpNotifyCallback callback, void* data);

/* Quick show functions */
void vaxp_notify_info(const char* title, const char* message);
void vaxp_notify_success(const char* title, const char* message);
void vaxp_notify_warning(const char* title, const char* message);
void vaxp_notify_error(const char* title, const char* message);

extern const VaxpWidgetClass vaxp_notification_class;

#define vaxp_notification(...) _vaxp_notification_build(&(VaxpNotificationConfig){ __VA_ARGS__ })

typedef struct VaxpNotificationConfig {
    const char* title;
    const char* message;
    VaxpNotificationType type;
    VaxpU32 duration_ms;
    VaxpBool dismissible;
    VaxpNotifyPosition position;
} VaxpNotificationConfig;

VaxpWidget* _vaxp_notification_build(const VaxpNotificationConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VAXP_NOTIFICATION_H */
