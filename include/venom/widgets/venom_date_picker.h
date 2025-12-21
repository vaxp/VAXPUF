/*
 * VENOMUI - DatePicker widget header
 */

#ifndef VENOM_DATE_PICKER_H
#define VENOM_DATE_PICKER_H

#include "venom/widgets/venom_widget.h"
#include "venom/widgets/venom_calendar.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VenomDatePicker VenomDatePicker;
typedef void (*VenomDatePickerCallback)(VenomDatePicker* picker, VenomDate date, void* data);

struct VenomDatePicker {
    VenomWidget base;
    
    VenomDate selected_date;
    VenomDate min_date;
    VenomDate max_date;
    VenomBool has_min;
    VenomBool has_max;
    
    char* placeholder;
    char* format;          /* e.g. "YYYY-MM-DD" */
    VenomBool is_open;
    
    VenomColor background_color;
    VenomColor border_color;
    VenomColor text_color;
    VenomF32 height;
    VenomF32 corner_radius;
    
    VenomDatePickerCallback on_change;
    void* callback_data;
};

VenomResultPtr venom_date_picker_create(void);
void venom_date_picker_set_date(VenomDatePicker* picker, VenomDate date);
VenomDate venom_date_picker_get_date(const VenomDatePicker* picker);
void venom_date_picker_set_placeholder(VenomDatePicker* picker, const char* ph);
void venom_date_picker_set_format(VenomDatePicker* picker, const char* fmt);
void venom_date_picker_set_min_date(VenomDatePicker* picker, VenomDate date);
void venom_date_picker_set_max_date(VenomDatePicker* picker, VenomDate date);
void venom_date_picker_open(VenomDatePicker* picker);
void venom_date_picker_close(VenomDatePicker* picker);
void venom_date_picker_set_on_change(VenomDatePicker* picker, VenomDatePickerCallback cb, void* data);

extern const VenomWidgetClass venom_date_picker_class;

#define venom_date_picker(...) _venom_date_picker_build(&(VenomDatePickerConfig){ __VA_ARGS__ })

typedef struct VenomDatePickerConfig {
    VenomDate initial_date;
    const char* placeholder;
    const char* format;
    VenomDatePickerCallback on_change;
    void* data;
} VenomDatePickerConfig;

VenomWidget* _venom_date_picker_build(const VenomDatePickerConfig* config);

#ifdef __cplusplus
}
#endif

#endif
