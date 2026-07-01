/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_stepper.c - Step indicator implementation
 */

#include "vaxp/widgets/vaxp_stepper.h"
#include "vaxp/core/vaxp_memory.h"
#include <string.h>
#include <stdio.h>

#define INITIAL_CAPACITY 8
#define DEFAULT_STEP_SIZE 32.0f

static void stepper_init(VaxpWidget* widget) {
    VaxpStepper* stepper = (VaxpStepper*)widget;
    
    stepper->steps = NULL;
    stepper->step_count = 0;
    stepper->step_capacity = 0;
    stepper->current_step = 0;
    
    stepper->vertical = VAXP_FALSE;
    stepper->clickable = VAXP_TRUE;
    stepper->on_step_click = NULL;
    stepper->callback_data = NULL;
    
    stepper->active_color = (VaxpColor){ 63, 81, 181, 255 };
    stepper->completed_color = (VaxpColor){ 76, 175, 80, 255 };
    stepper->inactive_color = (VaxpColor){ 189, 189, 189, 255 };
    stepper->error_color = (VaxpColor){ 244, 67, 54, 255 };
    stepper->connector_color = (VaxpColor){ 189, 189, 189, 255 };
    stepper->step_size = DEFAULT_STEP_SIZE;
}

static void stepper_destroy(VaxpWidget* widget) {
    VaxpStepper* stepper = (VaxpStepper*)widget;
    
    for (VaxpU32 i = 0; i < stepper->step_count; i++) {
        if (stepper->steps[i].label) {
            vaxp_free(stepper->steps[i].label, strlen(stepper->steps[i].label) + 1);
        }
    }
    
    if (stepper->steps) {
        vaxp_free(stepper->steps, stepper->step_capacity * sizeof(VaxpStep));
    }
    
    vaxp_widget_class.destroy(widget);
}

static void stepper_measure(VaxpWidget* widget, VaxpF32 available_width, VaxpF32 available_height,
                            VaxpF32* out_width, VaxpF32* out_height) {
    VaxpStepper* stepper = (VaxpStepper*)widget;
    (void)available_height;
    
    if (stepper->vertical) {
        *out_width = 200;
        *out_height = stepper->step_count * 60.0f;
    } else {
        *out_width = available_width;
        *out_height = 80.0f;
    }
}

static void stepper_draw(VaxpWidget* widget, VaxpCanvas* canvas) {
    VaxpStepper* stepper = (VaxpStepper*)widget;
    
    if (stepper->step_count == 0) return;
    
    VaxpF32 w = widget->bounds.width;
    VaxpF32 step_spacing = w / stepper->step_count;
    VaxpF32 s = stepper->step_size;
    
    /* Draw connectors first */
    VaxpPaint conn_paint = vaxp_paint_stroke(stepper->connector_color, 2.0f);
    for (VaxpU32 i = 0; i < stepper->step_count - 1; i++) {
        VaxpF32 x1 = step_spacing * i + step_spacing / 2 + s / 2 + 4;
        VaxpF32 x2 = step_spacing * (i + 1) + step_spacing / 2 - s / 2 - 4;
        VaxpF32 y = 20;
        
        if (stepper->steps[i].completed) {
            conn_paint = vaxp_paint_stroke(stepper->completed_color, 2.0f);
        } else {
            conn_paint = vaxp_paint_stroke(stepper->connector_color, 2.0f);
        }
        
        vaxp_canvas_draw_line(canvas, x1, y, x2, y, &conn_paint);
    }
    
    /* Draw steps */
    for (VaxpU32 i = 0; i < stepper->step_count; i++) {
        VaxpF32 cx = step_spacing * i + step_spacing / 2;
        VaxpF32 cy = 20;
        
        VaxpColor color;
        if (stepper->steps[i].error) {
            color = stepper->error_color;
        } else if (stepper->steps[i].completed) {
            color = stepper->completed_color;
        } else if (i == stepper->current_step) {
            color = stepper->active_color;
        } else {
            color = stepper->inactive_color;
        }
        
        /* Draw circle */
        VaxpPaint circle_paint = vaxp_paint_fill(color);
        vaxp_canvas_draw_circle(canvas, cx, cy, s / 2, &circle_paint);
        
        /* Draw number or checkmark */
        VaxpPaint text_paint = vaxp_paint_fill((VaxpColor){ 255, 255, 255, 255 });
        
        if (stepper->steps[i].completed) {
            vaxp_canvas_draw_text(canvas, "✓", cx - 5, cy + 5, NULL, &text_paint);
        } else if (stepper->steps[i].error) {
            vaxp_canvas_draw_text(canvas, "!", cx - 3, cy + 5, NULL, &text_paint);
        } else {
            char num[8];
            snprintf(num, sizeof(num), "%u", i + 1);
            vaxp_canvas_draw_text(canvas, num, cx - 4, cy + 5, NULL, &text_paint);
        }
        
        /* Draw label */
        if (stepper->steps[i].label) {
            VaxpPaint label_paint = vaxp_paint_fill((VaxpColor){ 97, 97, 97, 255 });
            VaxpF32 label_w = (VaxpF32)strlen(stepper->steps[i].label) * 7;
            vaxp_canvas_draw_text(canvas, stepper->steps[i].label, cx - label_w / 2, cy + s / 2 + 20, NULL, &label_paint);
        }
    }
}

static VaxpBool stepper_on_event(VaxpWidget* widget, const VaxpEvent* event) {
    VaxpStepper* stepper = (VaxpStepper*)widget;
    
    if (!stepper->clickable) return VAXP_FALSE;
    
    if (event->type == VAXP_EVENT_MOUSE_BUTTON_DOWN && 
        event->mouse.button == VAXP_MOUSE_BUTTON_LEFT) {
        VaxpF32 mx = (VaxpF32)event->mouse.x;
        VaxpF32 step_spacing = widget->bounds.width / stepper->step_count;
        
        VaxpU32 clicked_step = (VaxpU32)(mx / step_spacing);
        if (clicked_step < stepper->step_count) {
            stepper->current_step = clicked_step;
            widget->needs_redraw = VAXP_TRUE;
            
            if (stepper->on_step_click) {
                stepper->on_step_click(stepper, clicked_step, stepper->callback_data);
            }
            return VAXP_TRUE;
        }
    }
    
    return VAXP_FALSE;
}

const VaxpWidgetClass vaxp_stepper_class = {
    .class_name = "VaxpStepper",
    .instance_size = sizeof(VaxpStepper),
    .parent_class = &vaxp_widget_class,
    .init = stepper_init,
    .destroy = stepper_destroy,
    .measure = stepper_measure,
    .layout = NULL,
    .draw = stepper_draw,
    .on_event = stepper_on_event,
    .on_state_changed = NULL,
};

VaxpResultPtr vaxp_stepper_create(void) {
    return vaxp_widget_create(&vaxp_stepper_class);
}

VaxpResult vaxp_stepper_add_step(VaxpStepper* stepper, const char* label) {
    VAXP_ENSURE_NOT_NULL(stepper);
    
    if (stepper->step_count >= stepper->step_capacity) {
        VaxpU32 new_cap = stepper->step_capacity == 0 ? INITIAL_CAPACITY : stepper->step_capacity * 2;
        VaxpStep* new_steps = (VaxpStep*)vaxp_alloc(new_cap * sizeof(VaxpStep));
        if (!new_steps) return VAXP_ERR_UNIT(VAXP_ERROR_OUT_OF_MEMORY);
        
        if (stepper->steps) {
            memcpy(new_steps, stepper->steps, stepper->step_count * sizeof(VaxpStep));
            vaxp_free(stepper->steps, stepper->step_capacity * sizeof(VaxpStep));
        }
        
        stepper->steps = new_steps;
        stepper->step_capacity = new_cap;
    }
    
    VaxpStep* step = &stepper->steps[stepper->step_count];
    memset(step, 0, sizeof(VaxpStep));
    
    if (label) {
        VaxpSize len = strlen(label) + 1;
        step->label = (char*)vaxp_alloc(len);
        if (step->label) memcpy(step->label, label, len);
    }
    
    stepper->step_count++;
    return VAXP_OK_UNIT();
}

void vaxp_stepper_set_current(VaxpStepper* stepper, VaxpU32 step) {
    if (stepper && step < stepper->step_count) {
        stepper->current_step = step;
        vaxp_widget_invalidate((VaxpWidget*)stepper);
    }
}

void vaxp_stepper_next(VaxpStepper* stepper) {
    if (stepper && stepper->current_step < stepper->step_count - 1) {
        stepper->steps[stepper->current_step].completed = VAXP_TRUE;
        stepper->current_step++;
        vaxp_widget_invalidate((VaxpWidget*)stepper);
    }
}

void vaxp_stepper_prev(VaxpStepper* stepper) {
    if (stepper && stepper->current_step > 0) {
        stepper->current_step--;
        vaxp_widget_invalidate((VaxpWidget*)stepper);
    }
}

void vaxp_stepper_complete_step(VaxpStepper* stepper, VaxpU32 step) {
    if (stepper && step < stepper->step_count) {
        stepper->steps[step].completed = VAXP_TRUE;
        vaxp_widget_invalidate((VaxpWidget*)stepper);
    }
}

void vaxp_stepper_set_error(VaxpStepper* stepper, VaxpU32 step, VaxpBool error) {
    if (stepper && step < stepper->step_count) {
        stepper->steps[step].error = error;
        vaxp_widget_invalidate((VaxpWidget*)stepper);
    }
}
