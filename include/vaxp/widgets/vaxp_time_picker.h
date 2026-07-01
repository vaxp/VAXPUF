/*
 * VAXPUI - TimePicker widget header
 */

#ifndef VAXP_TIME_PICKER_H
#define VAXP_TIME_PICKER_H

#include "vaxp/widgets/vaxp_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VaxpTime {
    VaxpU8 hour;     /* 0-23 */
    VaxpU8 minute;   /* 0-59 */
    VaxpU8 second;   /* 0-59 */
} VaxpTime;

typedef struct VaxpTimePicker VaxpTimePicker;
typedef void (*VaxpTimePickerCallback)(VaxpTimePicker* picker, VaxpTime time, void* data);

struct VaxpTimePicker {
    VaxpWidget base;
    
    VaxpTime selected_time;
    VaxpBool use_24h;
    VaxpBool show_seconds;
    VaxpU8 minute_step;      /* 1, 5, 10, 15, 30 */
    VaxpBool is_open;
    
    VaxpI32 editing_field;   /* 0=hour, 1=min, 2=sec, -1=none */
    
    VaxpColor background_color;
    VaxpColor border_color;
    VaxpColor text_color;
    VaxpF32 height;
    
    VaxpTimePickerCallback on_change;
    void* callback_data;
};

VaxpResultPtr vaxp_time_picker_create(void);
void vaxp_time_picker_set_time(VaxpTimePicker* picker, VaxpTime time);
VaxpTime vaxp_time_picker_get_time(const VaxpTimePicker* picker);
void vaxp_time_picker_set_24h(VaxpTimePicker* picker, VaxpBool use_24h);
void vaxp_time_picker_set_show_seconds(VaxpTimePicker* picker, VaxpBool show);
void vaxp_time_picker_set_minute_step(VaxpTimePicker* picker, VaxpU8 step);
void vaxp_time_picker_set_on_change(VaxpTimePicker* picker, VaxpTimePickerCallback cb, void* data);

extern const VaxpWidgetClass vaxp_time_picker_class;

#define vaxp_time_picker(...) _vaxp_time_picker_build(&(VaxpTimePickerConfig){ __VA_ARGS__ })

typedef struct VaxpTimePickerConfig {
    VaxpTime initial_time;
    VaxpBool use_24h;
    VaxpBool show_seconds;
    VaxpTimePickerCallback on_change;
    void* data;
} VaxpTimePickerConfig;

VaxpWidget* _vaxp_time_picker_build(const VaxpTimePickerConfig* config);

#ifdef __cplusplus
}
#endif

#endif
