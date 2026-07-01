/*
 * VAXPUI - Calendar widget implementation
 */

#include "vaxp/widgets/vaxp_calendar.h"
#include "vaxp/core/vaxp_memory.h"
#include <time.h>
#include <stdio.h>
#include <string.h>

#define CELL_SIZE 36.0f
#define HEADER_HEIGHT 44.0f

static const char* DAY_NAMES[] = {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"};
static const char* MONTH_NAMES[] = {"January", "February", "March", "April", "May", "June",
                                    "July", "August", "September", "October", "November", "December"};

VaxpDate vaxp_date_today(void) {
    time_t t = time(NULL);
    struct tm* tm = localtime(&t);
    return (VaxpDate){ tm->tm_year + 1900, (VaxpU8)(tm->tm_mon + 1), (VaxpU8)tm->tm_mday };
}

VaxpBool vaxp_date_equals(VaxpDate a, VaxpDate b) {
    return a.year == b.year && a.month == b.month && a.day == b.day;
}

VaxpU32 vaxp_date_days_in_month(VaxpI32 year, VaxpU8 month) {
    static const VaxpU8 days[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    if (month == 2 && ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0)) return 29;
    return days[month - 1];
}

static VaxpU8 first_day_of_month(VaxpI32 year, VaxpU8 month) {
    struct tm tm = {0};
    tm.tm_year = year - 1900;
    tm.tm_mon = month - 1;
    tm.tm_mday = 1;
    mktime(&tm);
    return (VaxpU8)tm.tm_wday;
}

static void calendar_init(VaxpWidget* widget) {
    VaxpCalendar* cal = (VaxpCalendar*)widget;
    
    cal->displayed_month = vaxp_date_today();
    cal->selected_date = cal->displayed_month;
    cal->has_min = cal->has_max = VAXP_FALSE;
    cal->first_day_of_week = 0;
    cal->show_week_numbers = VAXP_FALSE;
    cal->cell_size = CELL_SIZE;
    cal->hover_day = -1;
    
    cal->header_color = (VaxpColor){ 63, 81, 181, 255 };
    cal->day_color = (VaxpColor){ 33, 33, 33, 255 };
    cal->today_color = (VaxpColor){ 63, 81, 181, 255 };
    cal->selected_color = (VaxpColor){ 63, 81, 181, 255 };
    cal->disabled_color = (VaxpColor){ 189, 189, 189, 255 };
    
    cal->on_select = NULL;
    cal->callback_data = NULL;
    
    widget->focusable = VAXP_TRUE;
}

static void calendar_measure(VaxpWidget* widget, VaxpF32 aw, VaxpF32 ah,
                             VaxpF32* out_w, VaxpF32* out_h) {
    VaxpCalendar* cal = (VaxpCalendar*)widget;
    (void)aw; (void)ah;
    *out_w = cal->cell_size * 7;
    *out_h = HEADER_HEIGHT + cal->cell_size * 7;  /* header + day names + 6 rows */
}

static void calendar_draw(VaxpWidget* widget, VaxpCanvas* canvas) {
    VaxpCalendar* cal = (VaxpCalendar*)widget;
    VaxpF32 cs = cal->cell_size;
    VaxpDate today = vaxp_date_today();
    
    /* Draw header */
    VaxpRectF header = {0, 0, cs * 7, HEADER_HEIGHT};
    VaxpPaint hp = vaxp_paint_fill(cal->header_color);
    vaxp_canvas_draw_rect(canvas, header, &hp);
    
    /* Month/Year title */
    char title[64];
    snprintf(title, sizeof(title), "%s %d", MONTH_NAMES[cal->displayed_month.month - 1], cal->displayed_month.year);
    VaxpPaint tp = vaxp_paint_fill((VaxpColor){255,255,255,255});
    vaxp_canvas_draw_text(canvas, title, cs * 1.5f, HEADER_HEIGHT / 2 + 5, NULL, &tp);
    
    /* Nav buttons */
    vaxp_canvas_draw_text(canvas, "◀", 10, HEADER_HEIGHT / 2 + 5, NULL, &tp);
    vaxp_canvas_draw_text(canvas, "▶", cs * 7 - 25, HEADER_HEIGHT / 2 + 5, NULL, &tp);
    
    /* Day names */
    VaxpPaint dayp = vaxp_paint_fill((VaxpColor){97,97,97,255});
    for (int i = 0; i < 7; i++) {
        int idx = (i + cal->first_day_of_week) % 7;
        vaxp_canvas_draw_text(canvas, DAY_NAMES[idx], i * cs + cs/2 - 8, HEADER_HEIGHT + cs/2 + 4, NULL, &dayp);
    }
    
    /* Days grid */
    VaxpU8 first_wd = first_day_of_month(cal->displayed_month.year, cal->displayed_month.month);
    VaxpU32 days_in_month = vaxp_date_days_in_month(cal->displayed_month.year, cal->displayed_month.month);
    
    int offset = (first_wd - cal->first_day_of_week + 7) % 7;
    VaxpF32 start_y = HEADER_HEIGHT + cs;
    
    for (VaxpU32 d = 1; d <= days_in_month; d++) {
        int cell = offset + d - 1;
        int row = cell / 7;
        int col = cell % 7;
        
        VaxpF32 cx = col * cs + cs / 2;
        VaxpF32 cy = start_y + row * cs + cs / 2;
        
        VaxpDate date = {cal->displayed_month.year, cal->displayed_month.month, (VaxpU8)d};
        VaxpBool is_today = vaxp_date_equals(date, today);
        VaxpBool is_selected = vaxp_date_equals(date, cal->selected_date);
        VaxpBool is_hover = ((VaxpI32)d == cal->hover_day);
        
        /* Draw selection/hover circle */
        if (is_selected) {
            VaxpPaint sp = vaxp_paint_fill(cal->selected_color);
            vaxp_canvas_draw_circle(canvas, cx, cy, cs/2 - 2, &sp);
        } else if (is_hover) {
            VaxpPaint hp2 = vaxp_paint_fill((VaxpColor){0,0,0,20});
            vaxp_canvas_draw_circle(canvas, cx, cy, cs/2 - 2, &hp2);
        }
        
        /* Draw today ring */
        if (is_today && !is_selected) {
            VaxpPaint rp = vaxp_paint_stroke(cal->today_color, 2.0f);
            vaxp_canvas_draw_circle(canvas, cx, cy, cs/2 - 3, &rp);
        }
        
        /* Draw day number */
        char day_str[4];
        snprintf(day_str, sizeof(day_str), "%u", d);
        VaxpColor tc = is_selected ? (VaxpColor){255,255,255,255} : cal->day_color;
        VaxpPaint dtp = vaxp_paint_fill(tc);
        vaxp_canvas_draw_text(canvas, day_str, cx - (d >= 10 ? 8 : 4), cy + 5, NULL, &dtp);
    }
}

static VaxpBool calendar_on_event(VaxpWidget* widget, const VaxpEvent* event) {
    VaxpCalendar* cal = (VaxpCalendar*)widget;
    
    if (event->type == VAXP_EVENT_MOUSE_MOVE) {
        VaxpF32 mx = (VaxpF32)event->mouse.x;
        VaxpF32 my = (VaxpF32)event->mouse.y;
        
        if (my > HEADER_HEIGHT + cal->cell_size) {
            int col = (int)(mx / cal->cell_size);
            int row = (int)((my - HEADER_HEIGHT - cal->cell_size) / cal->cell_size);
            VaxpU8 first_wd = first_day_of_month(cal->displayed_month.year, cal->displayed_month.month);
            int offset = (first_wd - cal->first_day_of_week + 7) % 7;
            int day = row * 7 + col - offset + 1;
            VaxpU32 max = vaxp_date_days_in_month(cal->displayed_month.year, cal->displayed_month.month);
            cal->hover_day = (day >= 1 && day <= (VaxpI32)max) ? day : -1;
            widget->needs_redraw = VAXP_TRUE;
        }
    }
    
    if (event->type == VAXP_EVENT_MOUSE_BUTTON_DOWN && event->mouse.button == VAXP_MOUSE_BUTTON_LEFT) {
        VaxpF32 mx = (VaxpF32)event->mouse.x;
        VaxpF32 my = (VaxpF32)event->mouse.y;
        
        /* Check nav buttons */
        if (my < HEADER_HEIGHT) {
            if (mx < 30) { vaxp_calendar_prev_month(cal); return VAXP_TRUE; }
            if (mx > cal->cell_size * 7 - 30) { vaxp_calendar_next_month(cal); return VAXP_TRUE; }
        }
        
        /* Check day click */
        if (cal->hover_day > 0) {
            VaxpDate date = {cal->displayed_month.year, cal->displayed_month.month, (VaxpU8)cal->hover_day};
            vaxp_calendar_select_date(cal, date);
            return VAXP_TRUE;
        }
    }
    
    return VAXP_FALSE;
}

const VaxpWidgetClass vaxp_calendar_class = {
    .class_name = "VaxpCalendar",
    .instance_size = sizeof(VaxpCalendar),
    .parent_class = &vaxp_widget_class,
    .init = calendar_init,
    .destroy = NULL,
    .measure = calendar_measure,
    .layout = NULL,
    .draw = calendar_draw,
    .on_event = calendar_on_event,
};

VaxpResultPtr vaxp_calendar_create(void) { return vaxp_widget_create(&vaxp_calendar_class); }

void vaxp_calendar_select_date(VaxpCalendar* cal, VaxpDate date) {
    if (cal) {
        cal->selected_date = date;
        vaxp_widget_invalidate((VaxpWidget*)cal);
        if (cal->on_select) cal->on_select(cal, date, cal->callback_data);
    }
}

VaxpDate vaxp_calendar_get_selected(const VaxpCalendar* cal) {
    return cal ? cal->selected_date : (VaxpDate){0,0,0};
}

void vaxp_calendar_go_to_today(VaxpCalendar* cal) {
    if (cal) { cal->displayed_month = vaxp_date_today(); vaxp_widget_invalidate((VaxpWidget*)cal); }
}

void vaxp_calendar_prev_month(VaxpCalendar* cal) {
    if (!cal) return;
    if (cal->displayed_month.month == 1) { cal->displayed_month.month = 12; cal->displayed_month.year--; }
    else { cal->displayed_month.month--; }
    vaxp_widget_invalidate((VaxpWidget*)cal);
}

void vaxp_calendar_next_month(VaxpCalendar* cal) {
    if (!cal) return;
    if (cal->displayed_month.month == 12) { cal->displayed_month.month = 1; cal->displayed_month.year++; }
    else { cal->displayed_month.month++; }
    vaxp_widget_invalidate((VaxpWidget*)cal);
}

void vaxp_calendar_set_on_select(VaxpCalendar* cal, VaxpCalendarCallback cb, void* data) {
    if (cal) { cal->on_select = cb; cal->callback_data = data; }
}

VaxpWidget* _vaxp_calendar_build(const VaxpCalendarConfig* config) {
    VaxpResultPtr r = vaxp_calendar_create();
    if (!r.ok) return NULL;
    VaxpCalendar* cal = (VaxpCalendar*)r.value;
    if (config->initial_date.year > 0) cal->displayed_month = config->initial_date;
    cal->on_select = config->on_select;
    cal->callback_data = config->data;
    return (VaxpWidget*)cal;
}
