/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_checkbox.c - Checkbox widget implementation
 */

#include "venom/widgets/venom_checkbox.h"
#include "venom/core/venom_memory.h"
#include <string.h>

#define DEFAULT_BOX_SIZE 18.0f
#define DEFAULT_SPACING 8.0f

static void checkbox_init(VenomWidget* widget) {
    VenomCheckbox* cb = (VenomCheckbox*)widget;
    
    cb->checked = VENOM_FALSE;
    cb->enabled = VENOM_TRUE;
    cb->label = NULL;
    
    cb->box_size = DEFAULT_BOX_SIZE;
    cb->spacing = DEFAULT_SPACING;
    cb->check_color = (VenomColor){ 63, 81, 181, 255 };  /* Primary */
    cb->box_color = (VenomColor){ 150, 150, 150, 255 };
    cb->label_color = (VenomColor){ 33, 33, 33, 255 };
    
    cb->on_change = NULL;
    cb->callback_data = NULL;
    
    widget->focusable = VENOM_TRUE;
}

static void checkbox_destroy(VenomWidget* widget) {
    VenomCheckbox* cb = (VenomCheckbox*)widget;
    if (cb->label) {
        venom_free(cb->label, strlen(cb->label) + 1);
        cb->label = NULL;
    }
    venom_widget_class.destroy(widget);
}

static void checkbox_measure(VenomWidget* widget, VenomF32 available_width, VenomF32 available_height,
                             VenomF32* out_width, VenomF32* out_height) {
    VenomCheckbox* cb = (VenomCheckbox*)widget;
    (void)available_width; (void)available_height;
    
    *out_width = cb->box_size;
    *out_height = cb->box_size;
    
    /* Add label width if present (approximate) */
    if (cb->label && cb->label[0]) {
        *out_width += cb->spacing + (VenomF32)strlen(cb->label) * 8.0f;
    }
}

static void checkbox_draw(VenomWidget* widget, VenomCanvas* canvas) {
    VenomCheckbox* cb = (VenomCheckbox*)widget;
    
    VenomF32 ox = 0, oy = (widget->bounds.height - cb->box_size) / 2;
    
    /* Draw box background */
    VenomRectF box_rect = { ox, oy, cb->box_size, cb->box_size };
    VenomColor bg = cb->checked ? cb->check_color : (VenomColor){ 255, 255, 255, 255 };
    VenomPaint bg_paint = venom_paint_fill(bg);
    venom_canvas_draw_rounded_rect(canvas, box_rect, 3.0f, &bg_paint);
    
    /* Draw box border */
    VenomColor border_color = cb->checked ? cb->check_color : cb->box_color;
    if (widget->state & VENOM_WIDGET_STATE_FOCUSED) {
        border_color = cb->check_color;
    }
    VenomPaint border_paint = venom_paint_stroke(border_color, 2.0f);
    venom_canvas_draw_rounded_rect(canvas, box_rect, 3.0f, &border_paint);
    
    /* Draw checkmark when checked */
    if (cb->checked) {
        VenomPaint check_paint = venom_paint_stroke((VenomColor){ 255, 255, 255, 255 }, 2.5f);
        
        /* Draw checkmark path */
        VenomF32 cx = ox + cb->box_size / 2;
        VenomF32 cy = oy + cb->box_size / 2;
        VenomF32 sz = cb->box_size * 0.3f;
        
        venom_canvas_draw_line(canvas, cx - sz, cy, cx - sz/3, cy + sz/2, &check_paint);
        venom_canvas_draw_line(canvas, cx - sz/3, cy + sz/2, cx + sz, cy - sz/2, &check_paint);
    }
    
    /* Draw label */
    if (cb->label && cb->label[0]) {
        VenomPaint text_paint = venom_paint_fill(cb->enabled ? cb->label_color : 
                                                  (VenomColor){ 150, 150, 150, 255 });
        VenomF32 text_x = ox + cb->box_size + cb->spacing;
        VenomF32 text_y = widget->bounds.height / 2 + 5;  /* Approx center */
        venom_canvas_draw_text(canvas, cb->label, text_x, text_y, NULL, &text_paint);
    }
}

static VenomBool checkbox_on_event(VenomWidget* widget, const VenomEvent* event) {
    VenomCheckbox* cb = (VenomCheckbox*)widget;
    
    if (!cb->enabled) return VENOM_FALSE;
    
    switch (event->type) {
        case VENOM_EVENT_MOUSE_BUTTON_DOWN:
            if (event->mouse.button == VENOM_MOUSE_BUTTON_LEFT) {
                cb->checked = !cb->checked;
                widget->needs_redraw = VENOM_TRUE;
                
                if (cb->on_change) {
                    cb->on_change(cb, cb->checked, cb->callback_data);
                }
                return VENOM_TRUE;
            }
            break;
            
        case VENOM_EVENT_KEY_DOWN:
            if (event->key.key == VENOM_KEY_SPACE || event->key.key == VENOM_KEY_RETURN) {
                cb->checked = !cb->checked;
                widget->needs_redraw = VENOM_TRUE;
                
                if (cb->on_change) {
                    cb->on_change(cb, cb->checked, cb->callback_data);
                }
                return VENOM_TRUE;
            }
            break;
            
        default:
            break;
    }
    
    return VENOM_FALSE;
}

const VenomWidgetClass venom_checkbox_class = {
    .class_name = "VenomCheckbox",
    .instance_size = sizeof(VenomCheckbox),
    .parent_class = &venom_widget_class,
    .init = checkbox_init,
    .destroy = checkbox_destroy,
    .measure = checkbox_measure,
    .layout = NULL,
    .draw = checkbox_draw,
    .on_event = checkbox_on_event,
    .on_state_changed = NULL,
};

VenomResultPtr venom_checkbox_create(void) {
    return venom_widget_create(&venom_checkbox_class);
}

void venom_checkbox_set_checked(VenomCheckbox* cb, VenomBool checked) {
    if (cb) {
        cb->checked = checked;
        venom_widget_invalidate((VenomWidget*)cb);
    }
}

VenomBool venom_checkbox_is_checked(const VenomCheckbox* cb) {
    return cb ? cb->checked : VENOM_FALSE;
}

void venom_checkbox_set_label(VenomCheckbox* cb, const char* label) {
    if (!cb) return;
    
    if (cb->label) {
        venom_free(cb->label, strlen(cb->label) + 1);
        cb->label = NULL;
    }
    
    if (label) {
        VenomSize len = strlen(label) + 1;
        cb->label = (char*)venom_alloc(len);
        if (cb->label) {
            memcpy(cb->label, label, len);
        }
    }
    
    venom_widget_invalidate((VenomWidget*)cb);
}

void venom_checkbox_set_enabled(VenomCheckbox* cb, VenomBool enabled) {
    if (cb) {
        cb->enabled = enabled;
        venom_widget_invalidate((VenomWidget*)cb);
    }
}

void venom_checkbox_set_on_change(VenomCheckbox* cb, VenomCheckboxCallback callback, void* data) {
    if (cb) {
        cb->on_change = callback;
        cb->callback_data = data;
    }
}

VenomWidget* _venom_checkbox_build(const VenomCheckboxConfig* config) {
    VenomResultPtr result = venom_checkbox_create();
    if (!result.ok) return NULL;
    
    VenomCheckbox* cb = (VenomCheckbox*)result.value;
    
    if (config->label) venom_checkbox_set_label(cb, config->label);
    cb->checked = config->checked;
    cb->on_change = config->on_change;
    cb->callback_data = config->data;
    
    return (VenomWidget*)cb;
}
