/*
 * VENOMUI - Calendar widget implementation
 */

#include "venom/widgets/venom_calendar.h"
#include "venom/core/venom_memory.h"
#include <time.h>
#include <stdio.h>
#include <string.h>

#define CELL_SIZE 36.0f
#define HEADER_HEIGHT 44.0f

static const char* DAY_NAMES[] = {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"};
static const char* MONTH_NAMES[] = {"January", "February", "March", "April", "May", "June",
                                    "July", "August", "September", "October", "November", "December"};

VenomDate venom_date_today(void) {
    time_t t = time(NULL);
    struct tm* tm = localtime(&t);
    return (VenomDate){ tm->tm_year + 1900, (VenomU8)(tm->tm_mon + 1), (VenomU8)tm->tm_mday };
}

VenomBool venom_date_equals(VenomDate a, VenomDate b) {
    return a.year == b.year && a.month == b.month && a.day == b.day;
}

VenomU32 venom_date_days_in_month(VenomI32 year, VenomU8 month) {
    static const VenomU8 days[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    if (month == 2 && ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0)) return 29;
    return days[month - 1];
}

static VenomU8 first_day_of_month(VenomI32 year, VenomU8 month) {
    struct tm tm = {0};
    tm.tm_year = year - 1900;
    tm.tm_mon = month - 1;
    tm.tm_mday = 1;
    mktime(&tm);
    return (VenomU8)tm.tm_wday;
}

static void calendar_init(VenomWidget* widget) {
    VenomCalendar* cal = (VenomCalendar*)widget;
    
    cal->displayed_month = venom_date_today();
    cal->selected_date = cal->displayed_month;
    cal->has_min = cal->has_max = VENOM_FALSE;
    cal->first_day_of_week = 0;
    cal->show_week_numbers = VENOM_FALSE;
    cal->cell_size = CELL_SIZE;
    cal->hover_day = -1;
    
    cal->header_color = (VenomColor){ 63, 81, 181, 255 };
    cal->day_color = (VenomColor){ 33, 33, 33, 255 };
    cal->today_color = (VenomColor){ 63, 81, 181, 255 };
    cal->selected_color = (VenomColor){ 63, 81, 181, 255 };
    cal->disabled_color = (VenomColor){ 189, 189, 189, 255 };
    
    cal->on_select = NULL;
    cal->callback_data = NULL;
    
    widget->focusable = VENOM_TRUE;
}

static void calendar_measure(VenomWidget* widget, VenomF32 aw, VenomF32 ah,
                             VenomF32* out_w, VenomF32* out_h) {
    VenomCalendar* cal = (VenomCalendar*)widget;
    (void)aw; (void)ah;
    *out_w = cal->cell_size * 7;
    *out_h = HEADER_HEIGHT + cal->cell_size * 7;  /* header + day names + 6 rows */
}

static void calendar_draw(VenomWidget* widget, VenomCanvas* canvas) {
    VenomCalendar* cal = (VenomCalendar*)widget;
    VenomF32 cs = cal->cell_size;
    VenomDate today = venom_date_today();
    
    /* Draw header */
    VenomRectF header = {0, 0, cs * 7, HEADER_HEIGHT};
    VenomPaint hp = venom_paint_fill(cal->header_color);
    venom_canvas_draw_rect(canvas, header, &hp);
    
    /* Month/Year title */
    char title[64];
    snprintf(title, sizeof(title), "%s %d", MONTH_NAMES[cal->displayed_month.month - 1], cal->displayed_month.year);
    VenomPaint tp = venom_paint_fill((VenomColor){255,255,255,255});
    venom_canvas_draw_text(canvas, title, cs * 1.5f, HEADER_HEIGHT / 2 + 5, NULL, &tp);
    
    /* Nav buttons */
    venom_canvas_draw_text(canvas, "◀", 10, HEADER_HEIGHT / 2 + 5, NULL, &tp);
    venom_canvas_draw_text(canvas, "▶", cs * 7 - 25, HEADER_HEIGHT / 2 + 5, NULL, &tp);
    
    /* Day names */
    VenomPaint dayp = venom_paint_fill((VenomColor){97,97,97,255});
    for (int i = 0; i < 7; i++) {
        int idx = (i + cal->first_day_of_week) % 7;
        venom_canvas_draw_text(canvas, DAY_NAMES[idx], i * cs + cs/2 - 8, HEADER_HEIGHT + cs/2 + 4, NULL, &dayp);
    }
    
    /* Days grid */
    VenomU8 first_wd = first_day_of_month(cal->displayed_month.year, cal->displayed_month.month);
    VenomU32 days_in_month = venom_date_days_in_month(cal->displayed_month.year, cal->displayed_month.month);
    
    int offset = (first_wd - cal->first_day_of_week + 7) % 7;
    VenomF32 start_y = HEADER_HEIGHT + cs;
    
    for (VenomU32 d = 1; d <= days_in_month; d++) {
        int cell = offset + d - 1;
        int row = cell / 7;
        int col = cell % 7;
        
        VenomF32 cx = col * cs + cs / 2;
        VenomF32 cy = start_y + row * cs + cs / 2;
        
        VenomDate date = {cal->displayed_month.year, cal->displayed_month.month, (VenomU8)d};
        VenomBool is_today = venom_date_equals(date, today);
        VenomBool is_selected = venom_date_equals(date, cal->selected_date);
        VenomBool is_hover = ((VenomI32)d == cal->hover_day);
        
        /* Draw selection/hover circle */
        if (is_selected) {
            VenomPaint sp = venom_paint_fill(cal->selected_color);
            venom_canvas_draw_circle(canvas, cx, cy, cs/2 - 2, &sp);
        } else if (is_hover) {
            VenomPaint hp2 = venom_paint_fill((VenomColor){0,0,0,20});
            venom_canvas_draw_circle(canvas, cx, cy, cs/2 - 2, &hp2);
        }
        
        /* Draw today ring */
        if (is_today && !is_selected) {
            VenomPaint rp = venom_paint_stroke(cal->today_color, 2.0f);
            venom_canvas_draw_circle(canvas, cx, cy, cs/2 - 3, &rp);
        }
        
        /* Draw day number */
        char day_str[4];
        snprintf(day_str, sizeof(day_str), "%u", d);
        VenomColor tc = is_selected ? (VenomColor){255,255,255,255} : cal->day_color;
        VenomPaint dtp = venom_paint_fill(tc);
        venom_canvas_draw_text(canvas, day_str, cx - (d >= 10 ? 8 : 4), cy + 5, NULL, &dtp);
    }
}

static VenomBool calendar_on_event(VenomWidget* widget, const VenomEvent* event) {
    VenomCalendar* cal = (VenomCalendar*)widget;
    
    if (event->type == VENOM_EVENT_MOUSE_MOVE) {
        VenomF32 mx = (VenomF32)event->mouse.x;
        VenomF32 my = (VenomF32)event->mouse.y;
        
        if (my > HEADER_HEIGHT + cal->cell_size) {
            int col = (int)(mx / cal->cell_size);
            int row = (int)((my - HEADER_HEIGHT - cal->cell_size) / cal->cell_size);
            VenomU8 first_wd = first_day_of_month(cal->displayed_month.year, cal->displayed_month.month);
            int offset = (first_wd - cal->first_day_of_week + 7) % 7;
            int day = row * 7 + col - offset + 1;
            VenomU32 max = venom_date_days_in_month(cal->displayed_month.year, cal->displayed_month.month);
            cal->hover_day = (day >= 1 && day <= (VenomI32)max) ? day : -1;
            widget->needs_redraw = VENOM_TRUE;
        }
    }
    
    if (event->type == VENOM_EVENT_MOUSE_BUTTON_DOWN && event->mouse.button == VENOM_MOUSE_BUTTON_LEFT) {
        VenomF32 mx = (VenomF32)event->mouse.x;
        VenomF32 my = (VenomF32)event->mouse.y;
        
        /* Check nav buttons */
        if (my < HEADER_HEIGHT) {
            if (mx < 30) { venom_calendar_prev_month(cal); return VENOM_TRUE; }
            if (mx > cal->cell_size * 7 - 30) { venom_calendar_next_month(cal); return VENOM_TRUE; }
        }
        
        /* Check day click */
        if (cal->hover_day > 0) {
            VenomDate date = {cal->displayed_month.year, cal->displayed_month.month, (VenomU8)cal->hover_day};
            venom_calendar_select_date(cal, date);
            return VENOM_TRUE;
        }
    }
    
    return VENOM_FALSE;
}

const VenomWidgetClass venom_calendar_class = {
    .class_name = "VenomCalendar",
    .instance_size = sizeof(VenomCalendar),
    .parent_class = &venom_widget_class,
    .init = calendar_init,
    .destroy = NULL,
    .measure = calendar_measure,
    .layout = NULL,
    .draw = calendar_draw,
    .on_event = calendar_on_event,
};

VenomResultPtr venom_calendar_create(void) { return venom_widget_create(&venom_calendar_class); }

void venom_calendar_select_date(VenomCalendar* cal, VenomDate date) {
    if (cal) {
        cal->selected_date = date;
        venom_widget_invalidate((VenomWidget*)cal);
        if (cal->on_select) cal->on_select(cal, date, cal->callback_data);
    }
}

VenomDate venom_calendar_get_selected(const VenomCalendar* cal) {
    return cal ? cal->selected_date : (VenomDate){0,0,0};
}

void venom_calendar_go_to_today(VenomCalendar* cal) {
    if (cal) { cal->displayed_month = venom_date_today(); venom_widget_invalidate((VenomWidget*)cal); }
}

void venom_calendar_prev_month(VenomCalendar* cal) {
    if (!cal) return;
    if (cal->displayed_month.month == 1) { cal->displayed_month.month = 12; cal->displayed_month.year--; }
    else { cal->displayed_month.month--; }
    venom_widget_invalidate((VenomWidget*)cal);
}

void venom_calendar_next_month(VenomCalendar* cal) {
    if (!cal) return;
    if (cal->displayed_month.month == 12) { cal->displayed_month.month = 1; cal->displayed_month.year++; }
    else { cal->displayed_month.month++; }
    venom_widget_invalidate((VenomWidget*)cal);
}

void venom_calendar_set_on_select(VenomCalendar* cal, VenomCalendarCallback cb, void* data) {
    if (cal) { cal->on_select = cb; cal->callback_data = data; }
}

VenomWidget* _venom_calendar_build(const VenomCalendarConfig* config) {
    VenomResultPtr r = venom_calendar_create();
    if (!r.ok) return NULL;
    VenomCalendar* cal = (VenomCalendar*)r.value;
    if (config->initial_date.year > 0) cal->displayed_month = config->initial_date;
    cal->on_select = config->on_select;
    cal->callback_data = config->data;
    return (VenomWidget*)cal;
}
