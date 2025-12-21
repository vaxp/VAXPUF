/*
 * VENOMUI - Calendar widget header
 */

#ifndef VENOM_CALENDAR_H
#define VENOM_CALENDAR_H

#include "venom/widgets/venom_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VenomDate {
    VenomI32 year;
    VenomU8 month;    /* 1-12 */
    VenomU8 day;      /* 1-31 */
} VenomDate;

typedef struct VenomCalendar VenomCalendar;
typedef void (*VenomCalendarCallback)(VenomCalendar* cal, VenomDate date, void* data);

struct VenomCalendar {
    VenomWidget base;
    
    VenomDate displayed_month;
    VenomDate selected_date;
    VenomDate min_date;
    VenomDate max_date;
    VenomBool has_min;
    VenomBool has_max;
    
    VenomU8 first_day_of_week;  /* 0=Sunday, 1=Monday */
    VenomBool show_week_numbers;
    
    VenomF32 cell_size;
    VenomI32 hover_day;
    
    VenomColor header_color;
    VenomColor day_color;
    VenomColor today_color;
    VenomColor selected_color;
    VenomColor disabled_color;
    
    VenomCalendarCallback on_select;
    void* callback_data;
};

VenomResultPtr venom_calendar_create(void);
void venom_calendar_select_date(VenomCalendar* cal, VenomDate date);
VenomDate venom_calendar_get_selected(const VenomCalendar* cal);
void venom_calendar_go_to_today(VenomCalendar* cal);
void venom_calendar_prev_month(VenomCalendar* cal);
void venom_calendar_next_month(VenomCalendar* cal);
void venom_calendar_set_on_select(VenomCalendar* cal, VenomCalendarCallback cb, void* data);

VenomDate venom_date_today(void);
VenomBool venom_date_equals(VenomDate a, VenomDate b);
VenomU32 venom_date_days_in_month(VenomI32 year, VenomU8 month);

extern const VenomWidgetClass venom_calendar_class;

#define venom_calendar(...) _venom_calendar_build(&(VenomCalendarConfig){ __VA_ARGS__ })

typedef struct VenomCalendarConfig {
    VenomDate initial_date;
    VenomCalendarCallback on_select;
    void* data;
} VenomCalendarConfig;

VenomWidget* _venom_calendar_build(const VenomCalendarConfig* config);

#ifdef __cplusplus
}
#endif

#endif
