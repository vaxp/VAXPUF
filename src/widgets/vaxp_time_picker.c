/*
 * VAXPUI - TimePicker widget implementation
 */

#include "vaxp/widgets/vaxp_time_picker.h"
#include "vaxp/core/vaxp_memory.h"
#include <stdio.h>
#include <time.h>

#define DEFAULT_HEIGHT 40.0f
#define FIELD_WIDTH 40.0f

static void time_picker_init(VaxpWidget* widget) {
    VaxpTimePicker* picker = (VaxpTimePicker*)widget;
    
    /* Get current time */
    time_t t = time(NULL);
    struct tm* tm = localtime(&t);
    picker->selected_time = (VaxpTime){ (VaxpU8)tm->tm_hour, (VaxpU8)tm->tm_min, 0 };
    
    picker->use_24h = VAXP_TRUE;
    picker->show_seconds = VAXP_FALSE;
    picker->minute_step = 1;
    picker->is_open = VAXP_FALSE;
    picker->editing_field = -1;
    
    picker->background_color = (VaxpColor){ 255, 255, 255, 255 };
    picker->border_color = (VaxpColor){ 189, 189, 189, 255 };
    picker->text_color = (VaxpColor){ 33, 33, 33, 255 };
    picker->height = DEFAULT_HEIGHT;
    
    picker->on_change = NULL;
    picker->callback_data = NULL;
    
    widget->focusable = VAXP_TRUE;
}

static void time_picker_measure(VaxpWidget* widget, VaxpF32 aw, VaxpF32 ah, VaxpF32* w, VaxpF32* h) {
    VaxpTimePicker* picker = (VaxpTimePicker*)widget;
    (void)aw; (void)ah;
    *w = picker->show_seconds ? 160.0f : 120.0f;
    *h = picker->height;
}

static void time_picker_draw(VaxpWidget* widget, VaxpCanvas* canvas) {
    VaxpTimePicker* picker = (VaxpTimePicker*)widget;
    VaxpF32 w = widget->bounds.width;
    VaxpF32 h = picker->height;
    
    /* Background */
    VaxpRectF bg = {0, 0, w, h};
    VaxpPaint bgp = vaxp_paint_fill(picker->background_color);
    vaxp_canvas_draw_rounded_rect(canvas, bg, 4.0f, &bgp);
    
    /* Border */
    VaxpPaint bp = vaxp_paint_stroke(picker->border_color, 1.0f);
    vaxp_canvas_draw_rounded_rect(canvas, bg, 4.0f, &bp);
    
    /* Time display */
    VaxpU8 hour = picker->selected_time.hour;
    const char* ampm = "";
    if (!picker->use_24h) {
        ampm = hour >= 12 ? " PM" : " AM";
        hour = hour % 12;
        if (hour == 0) hour = 12;
    }
    
    char text[32];
    if (picker->show_seconds) {
        snprintf(text, sizeof(text), "%02d:%02d:%02d%s", hour, picker->selected_time.minute, 
                 picker->selected_time.second, ampm);
    } else {
        snprintf(text, sizeof(text), "%02d:%02d%s", hour, picker->selected_time.minute, ampm);
    }
    
    VaxpPaint tp = vaxp_paint_fill(picker->text_color);
    vaxp_canvas_draw_text(canvas, text, 12, h/2 + 5, NULL, &tp);
    
    /* Up/Down buttons */
    VaxpPaint ip = vaxp_paint_fill((VaxpColor){97,97,97,255});
    vaxp_canvas_draw_text(canvas, "▲", w - 25, h/2 - 5, NULL, &ip);
    vaxp_canvas_draw_text(canvas, "▼", w - 25, h/2 + 12, NULL, &ip);
}

static void adjust_time(VaxpTimePicker* picker, int field, int delta) {
    switch (field) {
        case 0: /* hour */
            picker->selected_time.hour = (VaxpU8)((picker->selected_time.hour + delta + 24) % 24);
            break;
        case 1: /* minute */
            picker->selected_time.minute = (VaxpU8)((picker->selected_time.minute + delta + 60) % 60);
            break;
        case 2: /* second */
            picker->selected_time.second = (VaxpU8)((picker->selected_time.second + delta + 60) % 60);
            break;
    }
    
    if (picker->on_change) {
        picker->on_change(picker, picker->selected_time, picker->callback_data);
    }
}

static VaxpBool time_picker_on_event(VaxpWidget* widget, const VaxpEvent* event) {
    VaxpTimePicker* picker = (VaxpTimePicker*)widget;
    
    if (event->type == VAXP_EVENT_MOUSE_BUTTON_DOWN && event->mouse.button == VAXP_MOUSE_BUTTON_LEFT) {
        VaxpF32 mx = (VaxpF32)event->mouse.x;
        VaxpF32 my = (VaxpF32)event->mouse.y;
        VaxpF32 w = widget->bounds.width;
        VaxpF32 h = picker->height;
        
        /* Check up/down buttons */
        if (mx > w - 30) {
            if (my < h/2) { adjust_time(picker, 1, 1); }  /* Up */
            else { adjust_time(picker, 1, -1); }          /* Down */
            widget->needs_redraw = VAXP_TRUE;
            return VAXP_TRUE;
        }
        
        /* Determine which field was clicked */
        VaxpF32 x = 12;
        if (mx >= x && mx < x + FIELD_WIDTH) picker->editing_field = 0;
        else if (mx >= x + FIELD_WIDTH && mx < x + FIELD_WIDTH * 2) picker->editing_field = 1;
        else if (picker->show_seconds && mx >= x + FIELD_WIDTH * 2) picker->editing_field = 2;
        else picker->editing_field = 1;  /* Default to minutes */
        
        widget->needs_redraw = VAXP_TRUE;
        return VAXP_TRUE;
    }
    
    if (event->type == VAXP_EVENT_KEY_DOWN && picker->editing_field >= 0) {
        if (event->key.key == VAXP_KEY_UP) {
            adjust_time(picker, picker->editing_field, picker->editing_field == 1 ? picker->minute_step : 1);
            widget->needs_redraw = VAXP_TRUE;
            return VAXP_TRUE;
        }
        if (event->key.key == VAXP_KEY_DOWN) {
            adjust_time(picker, picker->editing_field, picker->editing_field == 1 ? -picker->minute_step : -1);
            widget->needs_redraw = VAXP_TRUE;
            return VAXP_TRUE;
        }
    }
    
    if (event->type == VAXP_EVENT_MOUSE_SCROLL) {
        adjust_time(picker, picker->editing_field >= 0 ? picker->editing_field : 1, 
                    (VaxpI32)event->scroll.y);
        widget->needs_redraw = VAXP_TRUE;
        return VAXP_TRUE;
    }
    
    return VAXP_FALSE;
}

const VaxpWidgetClass vaxp_time_picker_class = {
    .class_name = "VaxpTimePicker",
    .instance_size = sizeof(VaxpTimePicker),
    .parent_class = &vaxp_widget_class,
    .init = time_picker_init,
    .destroy = NULL,
    .measure = time_picker_measure,
    .draw = time_picker_draw,
    .on_event = time_picker_on_event,
};

VaxpResultPtr vaxp_time_picker_create(void) { return vaxp_widget_create(&vaxp_time_picker_class); }

void vaxp_time_picker_set_time(VaxpTimePicker* picker, VaxpTime time) {
    if (picker) {
        picker->selected_time = time;
        vaxp_widget_invalidate((VaxpWidget*)picker);
        if (picker->on_change) picker->on_change(picker, time, picker->callback_data);
    }
}

VaxpTime vaxp_time_picker_get_time(const VaxpTimePicker* picker) {
    return picker ? picker->selected_time : (VaxpTime){0,0,0};
}

void vaxp_time_picker_set_24h(VaxpTimePicker* picker, VaxpBool use) {
    if (picker) { picker->use_24h = use; vaxp_widget_invalidate((VaxpWidget*)picker); }
}

void vaxp_time_picker_set_show_seconds(VaxpTimePicker* picker, VaxpBool show) {
    if (picker) { picker->show_seconds = show; vaxp_widget_invalidate((VaxpWidget*)picker); }
}

void vaxp_time_picker_set_minute_step(VaxpTimePicker* picker, VaxpU8 step) {
    if (picker && step > 0) picker->minute_step = step;
}

void vaxp_time_picker_set_on_change(VaxpTimePicker* picker, VaxpTimePickerCallback cb, void* data) {
    if (picker) { picker->on_change = cb; picker->callback_data = data; }
}

VaxpWidget* _vaxp_time_picker_build(const VaxpTimePickerConfig* config) {
    VaxpResultPtr r = vaxp_time_picker_create();
    if (!r.ok) return NULL;
    VaxpTimePicker* p = (VaxpTimePicker*)r.value;
    if (config->initial_time.hour > 0 || config->initial_time.minute > 0) p->selected_time = config->initial_time;
    p->use_24h = config->use_24h;
    p->show_seconds = config->show_seconds;
    p->on_change = config->on_change;
    p->callback_data = config->data;
    return (VaxpWidget*)p;
}
