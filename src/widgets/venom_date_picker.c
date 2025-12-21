/*
 * VENOMUI - DatePicker widget implementation
 */

#include "venom/widgets/venom_date_picker.h"
#include "venom/core/venom_memory.h"
#include <string.h>
#include <stdio.h>

#define DEFAULT_HEIGHT 40.0f

static void date_picker_init(VenomWidget* widget) {
    VenomDatePicker* picker = (VenomDatePicker*)widget;
    
    picker->selected_date = venom_date_today();
    picker->has_min = picker->has_max = VENOM_FALSE;
    picker->placeholder = NULL;
    picker->format = NULL;
    picker->is_open = VENOM_FALSE;
    
    picker->background_color = (VenomColor){ 255, 255, 255, 255 };
    picker->border_color = (VenomColor){ 189, 189, 189, 255 };
    picker->text_color = (VenomColor){ 33, 33, 33, 255 };
    picker->height = DEFAULT_HEIGHT;
    picker->corner_radius = 4.0f;
    
    picker->on_change = NULL;
    picker->callback_data = NULL;
    
    widget->focusable = VENOM_TRUE;
}

static void date_picker_destroy(VenomWidget* widget) {
    VenomDatePicker* picker = (VenomDatePicker*)widget;
    if (picker->placeholder) venom_free(picker->placeholder, strlen(picker->placeholder) + 1);
    if (picker->format) venom_free(picker->format, strlen(picker->format) + 1);
    venom_widget_class.destroy(widget);
}

static void date_picker_measure(VenomWidget* widget, VenomF32 aw, VenomF32 ah, VenomF32* w, VenomF32* h) {
    VenomDatePicker* picker = (VenomDatePicker*)widget;
    (void)ah;
    *w = widget->layout.preferred_width > 0 ? widget->layout.preferred_width : (aw > 200 ? 200 : aw);
    *h = picker->height;
}

static void format_date(VenomDate date, char* buf, size_t size) {
    snprintf(buf, size, "%04d-%02d-%02d", date.year, date.month, date.day);
}

static void date_picker_draw(VenomWidget* widget, VenomCanvas* canvas) {
    VenomDatePicker* picker = (VenomDatePicker*)widget;
    VenomF32 w = widget->bounds.width;
    VenomF32 h = picker->height;
    
    /* Background */
    VenomRectF bg = {0, 0, w, h};
    VenomPaint bgp = venom_paint_fill(picker->background_color);
    venom_canvas_draw_rounded_rect(canvas, bg, picker->corner_radius, &bgp);
    
    /* Border */
    VenomColor bc = picker->is_open ? (VenomColor){63,81,181,255} : picker->border_color;
    VenomPaint bp = venom_paint_stroke(bc, picker->is_open ? 2.0f : 1.0f);
    venom_canvas_draw_rounded_rect(canvas, bg, picker->corner_radius, &bp);
    
    /* Text */
    char text[32];
    format_date(picker->selected_date, text, sizeof(text));
    VenomPaint tp = venom_paint_fill(picker->text_color);
    venom_canvas_draw_text(canvas, text, 12, h/2 + 5, NULL, &tp);
    
    /* Calendar icon */
    VenomPaint ip = venom_paint_fill((VenomColor){97,97,97,255});
    venom_canvas_draw_text(canvas, "📅", w - 30, h/2 + 5, NULL, &ip);
}

static VenomBool date_picker_on_event(VenomWidget* widget, const VenomEvent* event) {
    VenomDatePicker* picker = (VenomDatePicker*)widget;
    
    if (event->type == VENOM_EVENT_MOUSE_BUTTON_DOWN && event->mouse.button == VENOM_MOUSE_BUTTON_LEFT) {
        picker->is_open = !picker->is_open;
        widget->needs_redraw = VENOM_TRUE;
        return VENOM_TRUE;
    }
    
    return VENOM_FALSE;
}

const VenomWidgetClass venom_date_picker_class = {
    .class_name = "VenomDatePicker",
    .instance_size = sizeof(VenomDatePicker),
    .parent_class = &venom_widget_class,
    .init = date_picker_init,
    .destroy = date_picker_destroy,
    .measure = date_picker_measure,
    .draw = date_picker_draw,
    .on_event = date_picker_on_event,
};

VenomResultPtr venom_date_picker_create(void) { return venom_widget_create(&venom_date_picker_class); }

void venom_date_picker_set_date(VenomDatePicker* picker, VenomDate date) {
    if (picker) {
        picker->selected_date = date;
        venom_widget_invalidate((VenomWidget*)picker);
        if (picker->on_change) picker->on_change(picker, date, picker->callback_data);
    }
}

VenomDate venom_date_picker_get_date(const VenomDatePicker* picker) {
    return picker ? picker->selected_date : (VenomDate){0,0,0};
}

void venom_date_picker_set_placeholder(VenomDatePicker* picker, const char* ph) {
    if (!picker) return;
    if (picker->placeholder) venom_free(picker->placeholder, strlen(picker->placeholder) + 1);
    picker->placeholder = NULL;
    if (ph) {
        size_t len = strlen(ph) + 1;
        picker->placeholder = (char*)venom_alloc(len);
        if (picker->placeholder) memcpy(picker->placeholder, ph, len);
    }
}

void venom_date_picker_set_format(VenomDatePicker* picker, const char* fmt) {
    if (!picker) return;
    if (picker->format) venom_free(picker->format, strlen(picker->format) + 1);
    picker->format = NULL;
    if (fmt) {
        size_t len = strlen(fmt) + 1;
        picker->format = (char*)venom_alloc(len);
        if (picker->format) memcpy(picker->format, fmt, len);
    }
}

void venom_date_picker_open(VenomDatePicker* picker) {
    if (picker) { picker->is_open = VENOM_TRUE; venom_widget_invalidate((VenomWidget*)picker); }
}

void venom_date_picker_close(VenomDatePicker* picker) {
    if (picker) { picker->is_open = VENOM_FALSE; venom_widget_invalidate((VenomWidget*)picker); }
}

void venom_date_picker_set_on_change(VenomDatePicker* picker, VenomDatePickerCallback cb, void* data) {
    if (picker) { picker->on_change = cb; picker->callback_data = data; }
}

VenomWidget* _venom_date_picker_build(const VenomDatePickerConfig* config) {
    VenomResultPtr r = venom_date_picker_create();
    if (!r.ok) return NULL;
    VenomDatePicker* p = (VenomDatePicker*)r.value;
    if (config->initial_date.year > 0) p->selected_date = config->initial_date;
    if (config->placeholder) venom_date_picker_set_placeholder(p, config->placeholder);
    if (config->format) venom_date_picker_set_format(p, config->format);
    p->on_change = config->on_change;
    p->callback_data = config->data;
    return (VenomWidget*)p;
}
