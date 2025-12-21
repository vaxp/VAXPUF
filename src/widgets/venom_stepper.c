/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_stepper.c - Step indicator implementation
 */

#include "venom/widgets/venom_stepper.h"
#include "venom/core/venom_memory.h"
#include <string.h>
#include <stdio.h>

#define INITIAL_CAPACITY 8
#define DEFAULT_STEP_SIZE 32.0f

static void stepper_init(VenomWidget* widget) {
    VenomStepper* stepper = (VenomStepper*)widget;
    
    stepper->steps = NULL;
    stepper->step_count = 0;
    stepper->step_capacity = 0;
    stepper->current_step = 0;
    
    stepper->vertical = VENOM_FALSE;
    stepper->clickable = VENOM_TRUE;
    stepper->on_step_click = NULL;
    stepper->callback_data = NULL;
    
    stepper->active_color = (VenomColor){ 63, 81, 181, 255 };
    stepper->completed_color = (VenomColor){ 76, 175, 80, 255 };
    stepper->inactive_color = (VenomColor){ 189, 189, 189, 255 };
    stepper->error_color = (VenomColor){ 244, 67, 54, 255 };
    stepper->connector_color = (VenomColor){ 189, 189, 189, 255 };
    stepper->step_size = DEFAULT_STEP_SIZE;
}

static void stepper_destroy(VenomWidget* widget) {
    VenomStepper* stepper = (VenomStepper*)widget;
    
    for (VenomU32 i = 0; i < stepper->step_count; i++) {
        if (stepper->steps[i].label) {
            venom_free(stepper->steps[i].label, strlen(stepper->steps[i].label) + 1);
        }
    }
    
    if (stepper->steps) {
        venom_free(stepper->steps, stepper->step_capacity * sizeof(VenomStep));
    }
    
    venom_widget_class.destroy(widget);
}

static void stepper_measure(VenomWidget* widget, VenomF32 available_width, VenomF32 available_height,
                            VenomF32* out_width, VenomF32* out_height) {
    VenomStepper* stepper = (VenomStepper*)widget;
    (void)available_height;
    
    if (stepper->vertical) {
        *out_width = 200;
        *out_height = stepper->step_count * 60.0f;
    } else {
        *out_width = available_width;
        *out_height = 80.0f;
    }
}

static void stepper_draw(VenomWidget* widget, VenomCanvas* canvas) {
    VenomStepper* stepper = (VenomStepper*)widget;
    
    if (stepper->step_count == 0) return;
    
    VenomF32 w = widget->bounds.width;
    VenomF32 step_spacing = w / stepper->step_count;
    VenomF32 s = stepper->step_size;
    
    /* Draw connectors first */
    VenomPaint conn_paint = venom_paint_stroke(stepper->connector_color, 2.0f);
    for (VenomU32 i = 0; i < stepper->step_count - 1; i++) {
        VenomF32 x1 = step_spacing * i + step_spacing / 2 + s / 2 + 4;
        VenomF32 x2 = step_spacing * (i + 1) + step_spacing / 2 - s / 2 - 4;
        VenomF32 y = 20;
        
        if (stepper->steps[i].completed) {
            conn_paint = venom_paint_stroke(stepper->completed_color, 2.0f);
        } else {
            conn_paint = venom_paint_stroke(stepper->connector_color, 2.0f);
        }
        
        venom_canvas_draw_line(canvas, x1, y, x2, y, &conn_paint);
    }
    
    /* Draw steps */
    for (VenomU32 i = 0; i < stepper->step_count; i++) {
        VenomF32 cx = step_spacing * i + step_spacing / 2;
        VenomF32 cy = 20;
        
        VenomColor color;
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
        VenomPaint circle_paint = venom_paint_fill(color);
        venom_canvas_draw_circle(canvas, cx, cy, s / 2, &circle_paint);
        
        /* Draw number or checkmark */
        VenomPaint text_paint = venom_paint_fill((VenomColor){ 255, 255, 255, 255 });
        
        if (stepper->steps[i].completed) {
            venom_canvas_draw_text(canvas, "✓", cx - 5, cy + 5, NULL, &text_paint);
        } else if (stepper->steps[i].error) {
            venom_canvas_draw_text(canvas, "!", cx - 3, cy + 5, NULL, &text_paint);
        } else {
            char num[8];
            snprintf(num, sizeof(num), "%u", i + 1);
            venom_canvas_draw_text(canvas, num, cx - 4, cy + 5, NULL, &text_paint);
        }
        
        /* Draw label */
        if (stepper->steps[i].label) {
            VenomPaint label_paint = venom_paint_fill((VenomColor){ 97, 97, 97, 255 });
            VenomF32 label_w = (VenomF32)strlen(stepper->steps[i].label) * 7;
            venom_canvas_draw_text(canvas, stepper->steps[i].label, cx - label_w / 2, cy + s / 2 + 20, NULL, &label_paint);
        }
    }
}

static VenomBool stepper_on_event(VenomWidget* widget, const VenomEvent* event) {
    VenomStepper* stepper = (VenomStepper*)widget;
    
    if (!stepper->clickable) return VENOM_FALSE;
    
    if (event->type == VENOM_EVENT_MOUSE_BUTTON_DOWN && 
        event->mouse.button == VENOM_MOUSE_BUTTON_LEFT) {
        VenomF32 mx = (VenomF32)event->mouse.x;
        VenomF32 step_spacing = widget->bounds.width / stepper->step_count;
        
        VenomU32 clicked_step = (VenomU32)(mx / step_spacing);
        if (clicked_step < stepper->step_count) {
            stepper->current_step = clicked_step;
            widget->needs_redraw = VENOM_TRUE;
            
            if (stepper->on_step_click) {
                stepper->on_step_click(stepper, clicked_step, stepper->callback_data);
            }
            return VENOM_TRUE;
        }
    }
    
    return VENOM_FALSE;
}

const VenomWidgetClass venom_stepper_class = {
    .class_name = "VenomStepper",
    .instance_size = sizeof(VenomStepper),
    .parent_class = &venom_widget_class,
    .init = stepper_init,
    .destroy = stepper_destroy,
    .measure = stepper_measure,
    .layout = NULL,
    .draw = stepper_draw,
    .on_event = stepper_on_event,
    .on_state_changed = NULL,
};

VenomResultPtr venom_stepper_create(void) {
    return venom_widget_create(&venom_stepper_class);
}

VenomResult venom_stepper_add_step(VenomStepper* stepper, const char* label) {
    VENOM_ENSURE_NOT_NULL(stepper);
    
    if (stepper->step_count >= stepper->step_capacity) {
        VenomU32 new_cap = stepper->step_capacity == 0 ? INITIAL_CAPACITY : stepper->step_capacity * 2;
        VenomStep* new_steps = (VenomStep*)venom_alloc(new_cap * sizeof(VenomStep));
        if (!new_steps) return VENOM_ERR_UNIT(VENOM_ERROR_OUT_OF_MEMORY);
        
        if (stepper->steps) {
            memcpy(new_steps, stepper->steps, stepper->step_count * sizeof(VenomStep));
            venom_free(stepper->steps, stepper->step_capacity * sizeof(VenomStep));
        }
        
        stepper->steps = new_steps;
        stepper->step_capacity = new_cap;
    }
    
    VenomStep* step = &stepper->steps[stepper->step_count];
    memset(step, 0, sizeof(VenomStep));
    
    if (label) {
        VenomSize len = strlen(label) + 1;
        step->label = (char*)venom_alloc(len);
        if (step->label) memcpy(step->label, label, len);
    }
    
    stepper->step_count++;
    return VENOM_OK_UNIT();
}

void venom_stepper_set_current(VenomStepper* stepper, VenomU32 step) {
    if (stepper && step < stepper->step_count) {
        stepper->current_step = step;
        venom_widget_invalidate((VenomWidget*)stepper);
    }
}

void venom_stepper_next(VenomStepper* stepper) {
    if (stepper && stepper->current_step < stepper->step_count - 1) {
        stepper->steps[stepper->current_step].completed = VENOM_TRUE;
        stepper->current_step++;
        venom_widget_invalidate((VenomWidget*)stepper);
    }
}

void venom_stepper_prev(VenomStepper* stepper) {
    if (stepper && stepper->current_step > 0) {
        stepper->current_step--;
        venom_widget_invalidate((VenomWidget*)stepper);
    }
}

void venom_stepper_complete_step(VenomStepper* stepper, VenomU32 step) {
    if (stepper && step < stepper->step_count) {
        stepper->steps[step].completed = VENOM_TRUE;
        venom_widget_invalidate((VenomWidget*)stepper);
    }
}

void venom_stepper_set_error(VenomStepper* stepper, VenomU32 step, VenomBool error) {
    if (stepper && step < stepper->step_count) {
        stepper->steps[step].error = error;
        venom_widget_invalidate((VenomWidget*)stepper);
    }
}
