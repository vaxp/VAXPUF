/*
 * VAXPUI - Calendar widget header
 */

#ifndef VAXP_CALENDAR_H
#define VAXP_CALENDAR_H

#include "vaxp/widgets/vaxp_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VaxpDate {
    VaxpI32 year;
    VaxpU8 month;    /* 1-12 */
    VaxpU8 day;      /* 1-31 */
} VaxpDate;

typedef struct VaxpCalendar VaxpCalendar;
typedef void (*VaxpCalendarCallback)(VaxpCalendar* cal, VaxpDate date, void* data);

struct VaxpCalendar {
    VaxpWidget base;
    
    VaxpDate displayed_month;
    VaxpDate selected_date;
    VaxpDate min_date;
    VaxpDate max_date;
    VaxpBool has_min;
    VaxpBool has_max;
    
    VaxpU8 first_day_of_week;  /* 0=Sunday, 1=Monday */
    VaxpBool show_week_numbers;
    
    VaxpF32 cell_size;
    VaxpI32 hover_day;
    
    VaxpColor header_color;
    VaxpColor day_color;
    VaxpColor today_color;
    VaxpColor selected_color;
    VaxpColor disabled_color;
    
    VaxpCalendarCallback on_select;
    void* callback_data;
};

VaxpResultPtr vaxp_calendar_create(void);
void vaxp_calendar_select_date(VaxpCalendar* cal, VaxpDate date);
VaxpDate vaxp_calendar_get_selected(const VaxpCalendar* cal);
void vaxp_calendar_go_to_today(VaxpCalendar* cal);
void vaxp_calendar_prev_month(VaxpCalendar* cal);
void vaxp_calendar_next_month(VaxpCalendar* cal);
void vaxp_calendar_set_on_select(VaxpCalendar* cal, VaxpCalendarCallback cb, void* data);

VaxpDate vaxp_date_today(void);
VaxpBool vaxp_date_equals(VaxpDate a, VaxpDate b);
VaxpU32 vaxp_date_days_in_month(VaxpI32 year, VaxpU8 month);

extern const VaxpWidgetClass vaxp_calendar_class;

#define vaxp_calendar(...) _vaxp_calendar_build(&(VaxpCalendarConfig){ __VA_ARGS__ })

typedef struct VaxpCalendarConfig {
    VaxpDate initial_date;
    VaxpCalendarCallback on_select;
    void* data;
} VaxpCalendarConfig;

VaxpWidget* _vaxp_calendar_build(const VaxpCalendarConfig* config);

#ifdef __cplusplus
}
#endif

#endif
