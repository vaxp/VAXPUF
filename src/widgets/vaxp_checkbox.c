/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_checkbox.c - Checkbox widget implementation
 */

#include "vaxp/widgets/vaxp_checkbox.h"
#include "vaxp/core/vaxp_memory.h"
#include <string.h>

#define DEFAULT_BOX_SIZE 18.0f
#define DEFAULT_SPACING 8.0f

static void checkbox_init(VaxpWidget* widget) {
    VaxpCheckbox* cb = (VaxpCheckbox*)widget;
    
    cb->checked = VAXP_FALSE;
    cb->enabled = VAXP_TRUE;
    cb->label = NULL;
    
    cb->box_size = DEFAULT_BOX_SIZE;
    cb->spacing = DEFAULT_SPACING;
    cb->check_color = (VaxpColor){ 63, 81, 181, 255 };  /* Primary */
    cb->box_color = (VaxpColor){ 150, 150, 150, 255 };
    cb->label_color = (VaxpColor){ 33, 33, 33, 255 };
    
    cb->on_change = NULL;
    cb->callback_data = NULL;
    
    widget->focusable = VAXP_TRUE;
}

static void checkbox_destroy(VaxpWidget* widget) {
    VaxpCheckbox* cb = (VaxpCheckbox*)widget;
    if (cb->label) {
        vaxp_free(cb->label, strlen(cb->label) + 1);
        cb->label = NULL;
    }
    vaxp_widget_class.destroy(widget);
}

static void checkbox_measure(VaxpWidget* widget, VaxpF32 available_width, VaxpF32 available_height,
                             VaxpF32* out_width, VaxpF32* out_height) {
    VaxpCheckbox* cb = (VaxpCheckbox*)widget;
    (void)available_width; (void)available_height;
    
    *out_width = cb->box_size;
    *out_height = cb->box_size;
    
    /* Add label width if present (approximate) */
    if (cb->label && cb->label[0]) {
        *out_width += cb->spacing + (VaxpF32)strlen(cb->label) * 8.0f;
    }
}

static void checkbox_draw(VaxpWidget* widget, VaxpCanvas* canvas) {
    VaxpCheckbox* cb = (VaxpCheckbox*)widget;
    
    VaxpF32 ox = 0, oy = (widget->bounds.height - cb->box_size) / 2;
    
    /* Draw box background */
    VaxpRectF box_rect = { ox, oy, cb->box_size, cb->box_size };
    VaxpColor bg = cb->checked ? cb->check_color : (VaxpColor){ 255, 255, 255, 255 };
    VaxpPaint bg_paint = vaxp_paint_fill(bg);
    vaxp_canvas_draw_rounded_rect(canvas, box_rect, 3.0f, &bg_paint);
    
    /* Draw box border */
    VaxpColor border_color = cb->checked ? cb->check_color : cb->box_color;
    if (widget->state & VAXP_WIDGET_STATE_FOCUSED) {
        border_color = cb->check_color;
    }
    VaxpPaint border_paint = vaxp_paint_stroke(border_color, 2.0f);
    vaxp_canvas_draw_rounded_rect(canvas, box_rect, 3.0f, &border_paint);
    
    /* Draw checkmark when checked */
    if (cb->checked) {
        VaxpPaint check_paint = vaxp_paint_stroke((VaxpColor){ 255, 255, 255, 255 }, 2.5f);
        
        /* Draw checkmark path */
        VaxpF32 cx = ox + cb->box_size / 2;
        VaxpF32 cy = oy + cb->box_size / 2;
        VaxpF32 sz = cb->box_size * 0.3f;
        
        vaxp_canvas_draw_line(canvas, cx - sz, cy, cx - sz/3, cy + sz/2, &check_paint);
        vaxp_canvas_draw_line(canvas, cx - sz/3, cy + sz/2, cx + sz, cy - sz/2, &check_paint);
    }
    
    /* Draw label */
    if (cb->label && cb->label[0]) {
        VaxpPaint text_paint = vaxp_paint_fill(cb->enabled ? cb->label_color : 
                                                  (VaxpColor){ 150, 150, 150, 255 });
        VaxpF32 text_x = ox + cb->box_size + cb->spacing;
        VaxpF32 text_y = widget->bounds.height / 2 + 5;  /* Approx center */
        vaxp_canvas_draw_text(canvas, cb->label, text_x, text_y, NULL, &text_paint);
    }
}

static VaxpBool checkbox_on_event(VaxpWidget* widget, const VaxpEvent* event) {
    VaxpCheckbox* cb = (VaxpCheckbox*)widget;
    
    if (!cb->enabled) return VAXP_FALSE;
    
    switch (event->type) {
        case VAXP_EVENT_MOUSE_BUTTON_DOWN:
            if (event->mouse.button == VAXP_MOUSE_BUTTON_LEFT) {
                cb->checked = !cb->checked;
                widget->needs_redraw = VAXP_TRUE;
                
                if (cb->on_change) {
                    cb->on_change(cb, cb->checked, cb->callback_data);
                }
                return VAXP_TRUE;
            }
            break;
            
        case VAXP_EVENT_KEY_DOWN:
            if (event->key.key == VAXP_KEY_SPACE || event->key.key == VAXP_KEY_RETURN) {
                cb->checked = !cb->checked;
                widget->needs_redraw = VAXP_TRUE;
                
                if (cb->on_change) {
                    cb->on_change(cb, cb->checked, cb->callback_data);
                }
                return VAXP_TRUE;
            }
            break;
            
        default:
            break;
    }
    
    return VAXP_FALSE;
}

const VaxpWidgetClass vaxp_checkbox_class = {
    .class_name = "VaxpCheckbox",
    .instance_size = sizeof(VaxpCheckbox),
    .parent_class = &vaxp_widget_class,
    .init = checkbox_init,
    .destroy = checkbox_destroy,
    .measure = checkbox_measure,
    .layout = NULL,
    .draw = checkbox_draw,
    .on_event = checkbox_on_event,
    .on_state_changed = NULL,
};

VaxpResultPtr vaxp_checkbox_create(void) {
    return vaxp_widget_create(&vaxp_checkbox_class);
}

void vaxp_checkbox_set_checked(VaxpCheckbox* cb, VaxpBool checked) {
    if (cb) {
        cb->checked = checked;
        vaxp_widget_invalidate((VaxpWidget*)cb);
    }
}

VaxpBool vaxp_checkbox_is_checked(const VaxpCheckbox* cb) {
    return cb ? cb->checked : VAXP_FALSE;
}

void vaxp_checkbox_set_label(VaxpCheckbox* cb, const char* label) {
    if (!cb) return;
    
    if (cb->label) {
        vaxp_free(cb->label, strlen(cb->label) + 1);
        cb->label = NULL;
    }
    
    if (label) {
        VaxpSize len = strlen(label) + 1;
        cb->label = (char*)vaxp_alloc(len);
        if (cb->label) {
            memcpy(cb->label, label, len);
        }
    }
    
    vaxp_widget_invalidate((VaxpWidget*)cb);
}

void vaxp_checkbox_set_enabled(VaxpCheckbox* cb, VaxpBool enabled) {
    if (cb) {
        cb->enabled = enabled;
        vaxp_widget_invalidate((VaxpWidget*)cb);
    }
}

void vaxp_checkbox_set_on_change(VaxpCheckbox* cb, VaxpCheckboxCallback callback, void* data) {
    if (cb) {
        cb->on_change = callback;
        cb->callback_data = data;
    }
}

VaxpWidget* _vaxp_checkbox_build(const VaxpCheckboxConfig* config) {
    VaxpResultPtr result = vaxp_checkbox_create();
    if (!result.ok) return NULL;
    
    VaxpCheckbox* cb = (VaxpCheckbox*)result.value;
    
    if (config->label) vaxp_checkbox_set_label(cb, config->label);
    cb->checked = config->checked;
    cb->on_change = config->on_change;
    cb->callback_data = config->data;
    
    return (VaxpWidget*)cb;
}
