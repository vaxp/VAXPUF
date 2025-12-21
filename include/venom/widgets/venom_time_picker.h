/*
 * VENOMUI - TimePicker widget header
 */

#ifndef VENOM_TIME_PICKER_H
#define VENOM_TIME_PICKER_H

#include "venom/widgets/venom_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VenomTime {
    VenomU8 hour;     /* 0-23 */
    VenomU8 minute;   /* 0-59 */
    VenomU8 second;   /* 0-59 */
} VenomTime;

typedef struct VenomTimePicker VenomTimePicker;
typedef void (*VenomTimePickerCallback)(VenomTimePicker* picker, VenomTime time, void* data);

struct VenomTimePicker {
    VenomWidget base;
    
    VenomTime selected_time;
    VenomBool use_24h;
    VenomBool show_seconds;
    VenomU8 minute_step;      /* 1, 5, 10, 15, 30 */
    VenomBool is_open;
    
    VenomI32 editing_field;   /* 0=hour, 1=min, 2=sec, -1=none */
    
    VenomColor background_color;
    VenomColor border_color;
    VenomColor text_color;
    VenomF32 height;
    
    VenomTimePickerCallback on_change;
    void* callback_data;
};

VenomResultPtr venom_time_picker_create(void);
void venom_time_picker_set_time(VenomTimePicker* picker, VenomTime time);
VenomTime venom_time_picker_get_time(const VenomTimePicker* picker);
void venom_time_picker_set_24h(VenomTimePicker* picker, VenomBool use_24h);
void venom_time_picker_set_show_seconds(VenomTimePicker* picker, VenomBool show);
void venom_time_picker_set_minute_step(VenomTimePicker* picker, VenomU8 step);
void venom_time_picker_set_on_change(VenomTimePicker* picker, VenomTimePickerCallback cb, void* data);

extern const VenomWidgetClass venom_time_picker_class;

#define venom_time_picker(...) _venom_time_picker_build(&(VenomTimePickerConfig){ __VA_ARGS__ })

typedef struct VenomTimePickerConfig {
    VenomTime initial_time;
    VenomBool use_24h;
    VenomBool show_seconds;
    VenomTimePickerCallback on_change;
    void* data;
} VenomTimePickerConfig;

VenomWidget* _venom_time_picker_build(const VenomTimePickerConfig* config);

#ifdef __cplusplus
}
#endif

#endif
