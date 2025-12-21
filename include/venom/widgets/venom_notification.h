/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_notification.h - Notification/Toast widget
 */

#ifndef VENOM_NOTIFICATION_H
#define VENOM_NOTIFICATION_H

#include "venom/widgets/venom_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum VenomNotificationType {
    VENOM_NOTIFY_INFO,
    VENOM_NOTIFY_SUCCESS,
    VENOM_NOTIFY_WARNING,
    VENOM_NOTIFY_ERROR,
} VenomNotificationType;

typedef enum VenomNotifyPosition {
    VENOM_NOTIFY_TOP_LEFT,
    VENOM_NOTIFY_TOP_CENTER,
    VENOM_NOTIFY_TOP_RIGHT,
    VENOM_NOTIFY_BOTTOM_LEFT,
    VENOM_NOTIFY_BOTTOM_CENTER,
    VENOM_NOTIFY_BOTTOM_RIGHT,
} VenomNotifyPosition;

typedef struct VenomNotification VenomNotification;

typedef void (*VenomNotifyCallback)(VenomNotification* notif, void* user_data);

struct VenomNotification {
    VenomWidget base;
    
    char* title;
    char* message;
    VenomNotificationType type;
    
    VenomBool is_showing;
    VenomBool dismissible;
    VenomU32 duration_ms;        /* 0 = no auto-dismiss */
    VenomF32 progress;           /* For auto-dismiss countdown */
    VenomNotifyPosition position;
    
    /* Styling */
    VenomF32 width;
    VenomF32 corner_radius;
    VenomF32 padding;
    VenomColor background_color;
    VenomColor text_color;
    VenomColor icon_color;
    
    /* Callbacks */
    VenomNotifyCallback on_dismiss;
    void* callback_data;
};

VenomResultPtr venom_notification_create(void);
void venom_notification_set_title(VenomNotification* notif, const char* title);
void venom_notification_set_message(VenomNotification* notif, const char* message);
void venom_notification_set_type(VenomNotification* notif, VenomNotificationType type);
void venom_notification_set_duration(VenomNotification* notif, VenomU32 ms);
void venom_notification_set_position(VenomNotification* notif, VenomNotifyPosition pos);
void venom_notification_show(VenomNotification* notif);
void venom_notification_dismiss(VenomNotification* notif);
void venom_notification_set_on_dismiss(VenomNotification* notif, VenomNotifyCallback callback, void* data);

/* Quick show functions */
void venom_notify_info(const char* title, const char* message);
void venom_notify_success(const char* title, const char* message);
void venom_notify_warning(const char* title, const char* message);
void venom_notify_error(const char* title, const char* message);

extern const VenomWidgetClass venom_notification_class;

#define venom_notification(...) _venom_notification_build(&(VenomNotificationConfig){ __VA_ARGS__ })

typedef struct VenomNotificationConfig {
    const char* title;
    const char* message;
    VenomNotificationType type;
    VenomU32 duration_ms;
    VenomBool dismissible;
    VenomNotifyPosition position;
} VenomNotificationConfig;

VenomWidget* _venom_notification_build(const VenomNotificationConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VENOM_NOTIFICATION_H */
