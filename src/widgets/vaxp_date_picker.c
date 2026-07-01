/*
 * VAXPUI - DatePicker widget implementation
 */

#include "vaxp/widgets/vaxp_date_picker.h"
#include "vaxp/core/vaxp_memory.h"
#include <string.h>
#include <stdio.h>

#define DEFAULT_HEIGHT 40.0f

static void date_picker_init(VaxpWidget* widget) {
    VaxpDatePicker* picker = (VaxpDatePicker*)widget;
    
    picker->selected_date = vaxp_date_today();
    picker->has_min = picker->has_max = VAXP_FALSE;
    picker->placeholder = NULL;
    picker->format = NULL;
    picker->is_open = VAXP_FALSE;
    
    picker->background_color = (VaxpColor){ 255, 255, 255, 255 };
    picker->border_color = (VaxpColor){ 189, 189, 189, 255 };
    picker->text_color = (VaxpColor){ 33, 33, 33, 255 };
    picker->height = DEFAULT_HEIGHT;
    picker->corner_radius = 4.0f;
    
    picker->on_change = NULL;
    picker->callback_data = NULL;
    
    widget->focusable = VAXP_TRUE;
}

static void date_picker_destroy(VaxpWidget* widget) {
    VaxpDatePicker* picker = (VaxpDatePicker*)widget;
    if (picker->placeholder) vaxp_free(picker->placeholder, strlen(picker->placeholder) + 1);
    if (picker->format) vaxp_free(picker->format, strlen(picker->format) + 1);
    vaxp_widget_class.destroy(widget);
}

static void date_picker_measure(VaxpWidget* widget, VaxpF32 aw, VaxpF32 ah, VaxpF32* w, VaxpF32* h) {
    VaxpDatePicker* picker = (VaxpDatePicker*)widget;
    (void)ah;
    *w = widget->layout.preferred_width > 0 ? widget->layout.preferred_width : (aw > 200 ? 200 : aw);
    *h = picker->height;
}

static void format_date(VaxpDate date, char* buf, size_t size) {
    snprintf(buf, size, "%04d-%02d-%02d", date.year, date.month, date.day);
}

static void date_picker_draw(VaxpWidget* widget, VaxpCanvas* canvas) {
    VaxpDatePicker* picker = (VaxpDatePicker*)widget;
    VaxpF32 w = widget->bounds.width;
    VaxpF32 h = picker->height;
    
    /* Background */
    VaxpRectF bg = {0, 0, w, h};
    VaxpPaint bgp = vaxp_paint_fill(picker->background_color);
    vaxp_canvas_draw_rounded_rect(canvas, bg, picker->corner_radius, &bgp);
    
    /* Border */
    VaxpColor bc = picker->is_open ? (VaxpColor){63,81,181,255} : picker->border_color;
    VaxpPaint bp = vaxp_paint_stroke(bc, picker->is_open ? 2.0f : 1.0f);
    vaxp_canvas_draw_rounded_rect(canvas, bg, picker->corner_radius, &bp);
    
    /* Text */
    char text[32];
    format_date(picker->selected_date, text, sizeof(text));
    VaxpPaint tp = vaxp_paint_fill(picker->text_color);
    vaxp_canvas_draw_text(canvas, text, 12, h/2 + 5, NULL, &tp);
    
    /* Calendar icon */
    VaxpPaint ip = vaxp_paint_fill((VaxpColor){97,97,97,255});
    vaxp_canvas_draw_text(canvas, "📅", w - 30, h/2 + 5, NULL, &ip);
}

static VaxpBool date_picker_on_event(VaxpWidget* widget, const VaxpEvent* event) {
    VaxpDatePicker* picker = (VaxpDatePicker*)widget;
    
    if (event->type == VAXP_EVENT_MOUSE_BUTTON_DOWN && event->mouse.button == VAXP_MOUSE_BUTTON_LEFT) {
        picker->is_open = !picker->is_open;
        widget->needs_redraw = VAXP_TRUE;
        return VAXP_TRUE;
    }
    
    return VAXP_FALSE;
}

const VaxpWidgetClass vaxp_date_picker_class = {
    .class_name = "VaxpDatePicker",
    .instance_size = sizeof(VaxpDatePicker),
    .parent_class = &vaxp_widget_class,
    .init = date_picker_init,
    .destroy = date_picker_destroy,
    .measure = date_picker_measure,
    .draw = date_picker_draw,
    .on_event = date_picker_on_event,
};

VaxpResultPtr vaxp_date_picker_create(void) { return vaxp_widget_create(&vaxp_date_picker_class); }

void vaxp_date_picker_set_date(VaxpDatePicker* picker, VaxpDate date) {
    if (picker) {
        picker->selected_date = date;
        vaxp_widget_invalidate((VaxpWidget*)picker);
        if (picker->on_change) picker->on_change(picker, date, picker->callback_data);
    }
}

VaxpDate vaxp_date_picker_get_date(const VaxpDatePicker* picker) {
    return picker ? picker->selected_date : (VaxpDate){0,0,0};
}

void vaxp_date_picker_set_placeholder(VaxpDatePicker* picker, const char* ph) {
    if (!picker) return;
    if (picker->placeholder) vaxp_free(picker->placeholder, strlen(picker->placeholder) + 1);
    picker->placeholder = NULL;
    if (ph) {
        size_t len = strlen(ph) + 1;
        picker->placeholder = (char*)vaxp_alloc(len);
        if (picker->placeholder) memcpy(picker->placeholder, ph, len);
    }
}

void vaxp_date_picker_set_format(VaxpDatePicker* picker, const char* fmt) {
    if (!picker) return;
    if (picker->format) vaxp_free(picker->format, strlen(picker->format) + 1);
    picker->format = NULL;
    if (fmt) {
        size_t len = strlen(fmt) + 1;
        picker->format = (char*)vaxp_alloc(len);
        if (picker->format) memcpy(picker->format, fmt, len);
    }
}

void vaxp_date_picker_open(VaxpDatePicker* picker) {
    if (picker) { picker->is_open = VAXP_TRUE; vaxp_widget_invalidate((VaxpWidget*)picker); }
}

void vaxp_date_picker_close(VaxpDatePicker* picker) {
    if (picker) { picker->is_open = VAXP_FALSE; vaxp_widget_invalidate((VaxpWidget*)picker); }
}

void vaxp_date_picker_set_on_change(VaxpDatePicker* picker, VaxpDatePickerCallback cb, void* data) {
    if (picker) { picker->on_change = cb; picker->callback_data = data; }
}

VaxpWidget* _vaxp_date_picker_build(const VaxpDatePickerConfig* config) {
    VaxpResultPtr r = vaxp_date_picker_create();
    if (!r.ok) return NULL;
    VaxpDatePicker* p = (VaxpDatePicker*)r.value;
    if (config->initial_date.year > 0) p->selected_date = config->initial_date;
    if (config->placeholder) vaxp_date_picker_set_placeholder(p, config->placeholder);
    if (config->format) vaxp_date_picker_set_format(p, config->format);
    p->on_change = config->on_change;
    p->callback_data = config->data;
    return (VaxpWidget*)p;
}
