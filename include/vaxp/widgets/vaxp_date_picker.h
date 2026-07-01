/*
 * VAXPUI - DatePicker widget header
 */

#ifndef VAXP_DATE_PICKER_H
#define VAXP_DATE_PICKER_H

#include "vaxp/widgets/vaxp_widget.h"
#include "vaxp/widgets/vaxp_calendar.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VaxpDatePicker VaxpDatePicker;
typedef void (*VaxpDatePickerCallback)(VaxpDatePicker* picker, VaxpDate date, void* data);

struct VaxpDatePicker {
    VaxpWidget base;
    
    VaxpDate selected_date;
    VaxpDate min_date;
    VaxpDate max_date;
    VaxpBool has_min;
    VaxpBool has_max;
    
    char* placeholder;
    char* format;          /* e.g. "YYYY-MM-DD" */
    VaxpBool is_open;
    
    VaxpColor background_color;
    VaxpColor border_color;
    VaxpColor text_color;
    VaxpF32 height;
    VaxpF32 corner_radius;
    
    VaxpDatePickerCallback on_change;
    void* callback_data;
};

VaxpResultPtr vaxp_date_picker_create(void);
void vaxp_date_picker_set_date(VaxpDatePicker* picker, VaxpDate date);
VaxpDate vaxp_date_picker_get_date(const VaxpDatePicker* picker);
void vaxp_date_picker_set_placeholder(VaxpDatePicker* picker, const char* ph);
void vaxp_date_picker_set_format(VaxpDatePicker* picker, const char* fmt);
void vaxp_date_picker_set_min_date(VaxpDatePicker* picker, VaxpDate date);
void vaxp_date_picker_set_max_date(VaxpDatePicker* picker, VaxpDate date);
void vaxp_date_picker_open(VaxpDatePicker* picker);
void vaxp_date_picker_close(VaxpDatePicker* picker);
void vaxp_date_picker_set_on_change(VaxpDatePicker* picker, VaxpDatePickerCallback cb, void* data);

extern const VaxpWidgetClass vaxp_date_picker_class;

#define vaxp_date_picker(...) _vaxp_date_picker_build(&(VaxpDatePickerConfig){ __VA_ARGS__ })

typedef struct VaxpDatePickerConfig {
    VaxpDate initial_date;
    const char* placeholder;
    const char* format;
    VaxpDatePickerCallback on_change;
    void* data;
} VaxpDatePickerConfig;

VaxpWidget* _vaxp_date_picker_build(const VaxpDatePickerConfig* config);

#ifdef __cplusplus
}
#endif

#endif
