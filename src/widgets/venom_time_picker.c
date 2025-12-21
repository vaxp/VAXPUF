/*
 * VENOMUI - TimePicker widget implementation
 */

#include "venom/widgets/venom_time_picker.h"
#include "venom/core/venom_memory.h"
#include <stdio.h>
#include <time.h>

#define DEFAULT_HEIGHT 40.0f
#define FIELD_WIDTH 40.0f

static void time_picker_init(VenomWidget* widget) {
    VenomTimePicker* picker = (VenomTimePicker*)widget;
    
    /* Get current time */
    time_t t = time(NULL);
    struct tm* tm = localtime(&t);
    picker->selected_time = (VenomTime){ (VenomU8)tm->tm_hour, (VenomU8)tm->tm_min, 0 };
    
    picker->use_24h = VENOM_TRUE;
    picker->show_seconds = VENOM_FALSE;
    picker->minute_step = 1;
    picker->is_open = VENOM_FALSE;
    picker->editing_field = -1;
    
    picker->background_color = (VenomColor){ 255, 255, 255, 255 };
    picker->border_color = (VenomColor){ 189, 189, 189, 255 };
    picker->text_color = (VenomColor){ 33, 33, 33, 255 };
    picker->height = DEFAULT_HEIGHT;
    
    picker->on_change = NULL;
    picker->callback_data = NULL;
    
    widget->focusable = VENOM_TRUE;
}

static void time_picker_measure(VenomWidget* widget, VenomF32 aw, VenomF32 ah, VenomF32* w, VenomF32* h) {
    VenomTimePicker* picker = (VenomTimePicker*)widget;
    (void)aw; (void)ah;
    *w = picker->show_seconds ? 160.0f : 120.0f;
    *h = picker->height;
}

static void time_picker_draw(VenomWidget* widget, VenomCanvas* canvas) {
    VenomTimePicker* picker = (VenomTimePicker*)widget;
    VenomF32 w = widget->bounds.width;
    VenomF32 h = picker->height;
    
    /* Background */
    VenomRectF bg = {0, 0, w, h};
    VenomPaint bgp = venom_paint_fill(picker->background_color);
    venom_canvas_draw_rounded_rect(canvas, bg, 4.0f, &bgp);
    
    /* Border */
    VenomPaint bp = venom_paint_stroke(picker->border_color, 1.0f);
    venom_canvas_draw_rounded_rect(canvas, bg, 4.0f, &bp);
    
    /* Time display */
    VenomU8 hour = picker->selected_time.hour;
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
    
    VenomPaint tp = venom_paint_fill(picker->text_color);
    venom_canvas_draw_text(canvas, text, 12, h/2 + 5, NULL, &tp);
    
    /* Up/Down buttons */
    VenomPaint ip = venom_paint_fill((VenomColor){97,97,97,255});
    venom_canvas_draw_text(canvas, "▲", w - 25, h/2 - 5, NULL, &ip);
    venom_canvas_draw_text(canvas, "▼", w - 25, h/2 + 12, NULL, &ip);
}

static void adjust_time(VenomTimePicker* picker, int field, int delta) {
    switch (field) {
        case 0: /* hour */
            picker->selected_time.hour = (VenomU8)((picker->selected_time.hour + delta + 24) % 24);
            break;
        case 1: /* minute */
            picker->selected_time.minute = (VenomU8)((picker->selected_time.minute + delta + 60) % 60);
            break;
        case 2: /* second */
            picker->selected_time.second = (VenomU8)((picker->selected_time.second + delta + 60) % 60);
            break;
    }
    
    if (picker->on_change) {
        picker->on_change(picker, picker->selected_time, picker->callback_data);
    }
}

static VenomBool time_picker_on_event(VenomWidget* widget, const VenomEvent* event) {
    VenomTimePicker* picker = (VenomTimePicker*)widget;
    
    if (event->type == VENOM_EVENT_MOUSE_BUTTON_DOWN && event->mouse.button == VENOM_MOUSE_BUTTON_LEFT) {
        VenomF32 mx = (VenomF32)event->mouse.x;
        VenomF32 my = (VenomF32)event->mouse.y;
        VenomF32 w = widget->bounds.width;
        VenomF32 h = picker->height;
        
        /* Check up/down buttons */
        if (mx > w - 30) {
            if (my < h/2) { adjust_time(picker, 1, 1); }  /* Up */
            else { adjust_time(picker, 1, -1); }          /* Down */
            widget->needs_redraw = VENOM_TRUE;
            return VENOM_TRUE;
        }
        
        /* Determine which field was clicked */
        VenomF32 x = 12;
        if (mx >= x && mx < x + FIELD_WIDTH) picker->editing_field = 0;
        else if (mx >= x + FIELD_WIDTH && mx < x + FIELD_WIDTH * 2) picker->editing_field = 1;
        else if (picker->show_seconds && mx >= x + FIELD_WIDTH * 2) picker->editing_field = 2;
        else picker->editing_field = 1;  /* Default to minutes */
        
        widget->needs_redraw = VENOM_TRUE;
        return VENOM_TRUE;
    }
    
    if (event->type == VENOM_EVENT_KEY_DOWN && picker->editing_field >= 0) {
        if (event->key.key == VENOM_KEY_UP) {
            adjust_time(picker, picker->editing_field, picker->editing_field == 1 ? picker->minute_step : 1);
            widget->needs_redraw = VENOM_TRUE;
            return VENOM_TRUE;
        }
        if (event->key.key == VENOM_KEY_DOWN) {
            adjust_time(picker, picker->editing_field, picker->editing_field == 1 ? -picker->minute_step : -1);
            widget->needs_redraw = VENOM_TRUE;
            return VENOM_TRUE;
        }
    }
    
    if (event->type == VENOM_EVENT_MOUSE_SCROLL) {
        adjust_time(picker, picker->editing_field >= 0 ? picker->editing_field : 1, 
                    (VenomI32)event->scroll.y);
        widget->needs_redraw = VENOM_TRUE;
        return VENOM_TRUE;
    }
    
    return VENOM_FALSE;
}

const VenomWidgetClass venom_time_picker_class = {
    .class_name = "VenomTimePicker",
    .instance_size = sizeof(VenomTimePicker),
    .parent_class = &venom_widget_class,
    .init = time_picker_init,
    .destroy = NULL,
    .measure = time_picker_measure,
    .draw = time_picker_draw,
    .on_event = time_picker_on_event,
};

VenomResultPtr venom_time_picker_create(void) { return venom_widget_create(&venom_time_picker_class); }

void venom_time_picker_set_time(VenomTimePicker* picker, VenomTime time) {
    if (picker) {
        picker->selected_time = time;
        venom_widget_invalidate((VenomWidget*)picker);
        if (picker->on_change) picker->on_change(picker, time, picker->callback_data);
    }
}

VenomTime venom_time_picker_get_time(const VenomTimePicker* picker) {
    return picker ? picker->selected_time : (VenomTime){0,0,0};
}

void venom_time_picker_set_24h(VenomTimePicker* picker, VenomBool use) {
    if (picker) { picker->use_24h = use; venom_widget_invalidate((VenomWidget*)picker); }
}

void venom_time_picker_set_show_seconds(VenomTimePicker* picker, VenomBool show) {
    if (picker) { picker->show_seconds = show; venom_widget_invalidate((VenomWidget*)picker); }
}

void venom_time_picker_set_minute_step(VenomTimePicker* picker, VenomU8 step) {
    if (picker && step > 0) picker->minute_step = step;
}

void venom_time_picker_set_on_change(VenomTimePicker* picker, VenomTimePickerCallback cb, void* data) {
    if (picker) { picker->on_change = cb; picker->callback_data = data; }
}

VenomWidget* _venom_time_picker_build(const VenomTimePickerConfig* config) {
    VenomResultPtr r = venom_time_picker_create();
    if (!r.ok) return NULL;
    VenomTimePicker* p = (VenomTimePicker*)r.value;
    if (config->initial_time.hour > 0 || config->initial_time.minute > 0) p->selected_time = config->initial_time;
    p->use_24h = config->use_24h;
    p->show_seconds = config->show_seconds;
    p->on_change = config->on_change;
    p->callback_data = config->data;
    return (VenomWidget*)p;
}
