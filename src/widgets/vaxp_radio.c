/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_radio.c - Radio button widget implementation
 */

#include "vaxp/widgets/vaxp_radio.h"
#include "vaxp/core/vaxp_memory.h"
#include <string.h>

#define DEFAULT_CIRCLE_SIZE 20.0f
#define DEFAULT_SPACING 8.0f
#define INITIAL_GROUP_CAPACITY 8

/* ============================================================================
 * RADIO GROUP
 * ============================================================================ */

VaxpRadioGroup* vaxp_radio_group_create(void) {
    VaxpRadioGroup* group = (VaxpRadioGroup*)vaxp_alloc(sizeof(VaxpRadioGroup));
    if (!group) return NULL;
    
    group->buttons = NULL;
    group->count = 0;
    group->capacity = 0;
    group->selected = NULL;
    
    return group;
}

void vaxp_radio_group_destroy(VaxpRadioGroup* group) {
    if (!group) return;
    
    if (group->buttons) {
        vaxp_free(group->buttons, group->capacity * sizeof(VaxpRadioButton*));
    }
    vaxp_free(group, sizeof(VaxpRadioGroup));
}

void vaxp_radio_group_add(VaxpRadioGroup* group, VaxpRadioButton* radio) {
    if (!group || !radio) return;
    
    /* Expand if needed */
    if (group->count >= group->capacity) {
        VaxpU32 new_cap = group->capacity == 0 ? INITIAL_GROUP_CAPACITY : group->capacity * 2;
        VaxpRadioButton** new_arr = (VaxpRadioButton**)vaxp_alloc(new_cap * sizeof(VaxpRadioButton*));
        if (!new_arr) return;
        
        if (group->buttons) {
            memcpy(new_arr, group->buttons, group->count * sizeof(VaxpRadioButton*));
            vaxp_free(group->buttons, group->capacity * sizeof(VaxpRadioButton*));
        }
        
        group->buttons = new_arr;
        group->capacity = new_cap;
    }
    
    group->buttons[group->count++] = radio;
    radio->group = group;
    
    if (radio->selected) {
        group->selected = radio;
    }
}

VaxpRadioButton* vaxp_radio_group_get_selected(const VaxpRadioGroup* group) {
    return group ? group->selected : NULL;
}

/* ============================================================================
 * RADIO BUTTON
 * ============================================================================ */

static void radio_init(VaxpWidget* widget) {
    VaxpRadioButton* radio = (VaxpRadioButton*)widget;
    
    radio->selected = VAXP_FALSE;
    radio->enabled = VAXP_TRUE;
    radio->label = NULL;
    radio->group = NULL;
    radio->value = 0;
    
    radio->circle_size = DEFAULT_CIRCLE_SIZE;
    radio->spacing = DEFAULT_SPACING;
    radio->selected_color = (VaxpColor){ 63, 81, 181, 255 };
    radio->unselected_color = (VaxpColor){ 150, 150, 150, 255 };
    radio->label_color = (VaxpColor){ 33, 33, 33, 255 };
    
    radio->on_select = NULL;
    radio->callback_data = NULL;
    
    widget->focusable = VAXP_TRUE;
}

static void radio_destroy(VaxpWidget* widget) {
    VaxpRadioButton* radio = (VaxpRadioButton*)widget;
    
    if (radio->label) {
        vaxp_free(radio->label, strlen(radio->label) + 1);
        radio->label = NULL;
    }
    
    vaxp_widget_class.destroy(widget);
}

static void radio_measure(VaxpWidget* widget, VaxpF32 available_width, VaxpF32 available_height,
                          VaxpF32* out_width, VaxpF32* out_height) {
    VaxpRadioButton* radio = (VaxpRadioButton*)widget;
    (void)available_width; (void)available_height;
    
    *out_width = radio->circle_size;
    *out_height = radio->circle_size;
    
    if (radio->label && radio->label[0]) {
        *out_width += radio->spacing + (VaxpF32)strlen(radio->label) * 8.0f;
    }
}

static void radio_draw(VaxpWidget* widget, VaxpCanvas* canvas) {
    VaxpRadioButton* radio = (VaxpRadioButton*)widget;
    
    VaxpF32 cx = radio->circle_size / 2;
    VaxpF32 cy = widget->bounds.height / 2;
    VaxpF32 r = radio->circle_size / 2;
    
    /* Draw outer circle */
    VaxpColor outer_color = radio->selected ? radio->selected_color : radio->unselected_color;
    if (widget->state & VAXP_WIDGET_STATE_FOCUSED) {
        outer_color = radio->selected_color;
    }
    VaxpPaint outer_paint = vaxp_paint_stroke(outer_color, 2.0f);
    vaxp_canvas_draw_circle(canvas, cx, cy, r - 2, &outer_paint);
    
    /* Draw inner filled circle when selected */
    if (radio->selected) {
        VaxpPaint inner_paint = vaxp_paint_fill(radio->selected_color);
        vaxp_canvas_draw_circle(canvas, cx, cy, r * 0.45f, &inner_paint);
    }
    
    /* Draw label */
    if (radio->label && radio->label[0]) {
        VaxpPaint text_paint = vaxp_paint_fill(radio->enabled ? radio->label_color : 
                                                  (VaxpColor){ 150, 150, 150, 255 });
        VaxpF32 text_x = radio->circle_size + radio->spacing;
        VaxpF32 text_y = widget->bounds.height / 2 + 5;
        vaxp_canvas_draw_text(canvas, radio->label, text_x, text_y, NULL, &text_paint);
    }
}

static VaxpBool radio_on_event(VaxpWidget* widget, const VaxpEvent* event) {
    VaxpRadioButton* radio = (VaxpRadioButton*)widget;
    
    if (!radio->enabled) return VAXP_FALSE;
    
    VaxpBool activate = VAXP_FALSE;
    
    switch (event->type) {
        case VAXP_EVENT_MOUSE_BUTTON_DOWN:
            if (event->mouse.button == VAXP_MOUSE_BUTTON_LEFT) {
                activate = VAXP_TRUE;
            }
            break;
            
        case VAXP_EVENT_KEY_DOWN:
            if (event->key.key == VAXP_KEY_SPACE || event->key.key == VAXP_KEY_RETURN) {
                activate = VAXP_TRUE;
            }
            break;
            
        default:
            break;
    }
    
    if (activate && !radio->selected) {
        /* Deselect all other radios in group */
        if (radio->group) {
            for (VaxpU32 i = 0; i < radio->group->count; i++) {
                VaxpRadioButton* other = radio->group->buttons[i];
                if (other != radio && other->selected) {
                    other->selected = VAXP_FALSE;
                    ((VaxpWidget*)other)->needs_redraw = VAXP_TRUE;
                }
            }
            radio->group->selected = radio;
        }
        
        radio->selected = VAXP_TRUE;
        widget->needs_redraw = VAXP_TRUE;
        
        if (radio->on_select) {
            radio->on_select(radio, radio->callback_data);
        }
        
        return VAXP_TRUE;
    }
    
    return VAXP_FALSE;
}

const VaxpWidgetClass vaxp_radio_class = {
    .class_name = "VaxpRadioButton",
    .instance_size = sizeof(VaxpRadioButton),
    .parent_class = &vaxp_widget_class,
    .init = radio_init,
    .destroy = radio_destroy,
    .measure = radio_measure,
    .layout = NULL,
    .draw = radio_draw,
    .on_event = radio_on_event,
    .on_state_changed = NULL,
};

VaxpResultPtr vaxp_radio_create(void) {
    return vaxp_widget_create(&vaxp_radio_class);
}

void vaxp_radio_set_selected(VaxpRadioButton* radio, VaxpBool selected) {
    if (!radio) return;
    
    if (selected && !radio->selected) {
        /* Deselect others in group */
        if (radio->group) {
            for (VaxpU32 i = 0; i < radio->group->count; i++) {
                VaxpRadioButton* other = radio->group->buttons[i];
                if (other != radio) {
                    other->selected = VAXP_FALSE;
                    vaxp_widget_invalidate((VaxpWidget*)other);
                }
            }
            radio->group->selected = radio;
        }
    }
    
    radio->selected = selected;
    vaxp_widget_invalidate((VaxpWidget*)radio);
}

VaxpBool vaxp_radio_is_selected(const VaxpRadioButton* radio) {
    return radio ? radio->selected : VAXP_FALSE;
}

void vaxp_radio_set_label(VaxpRadioButton* radio, const char* label) {
    if (!radio) return;
    
    if (radio->label) {
        vaxp_free(radio->label, strlen(radio->label) + 1);
        radio->label = NULL;
    }
    
    if (label) {
        VaxpSize len = strlen(label) + 1;
        radio->label = (char*)vaxp_alloc(len);
        if (radio->label) {
            memcpy(radio->label, label, len);
        }
    }
    
    vaxp_widget_invalidate((VaxpWidget*)radio);
}

void vaxp_radio_set_group(VaxpRadioButton* radio, VaxpRadioGroup* group) {
    if (radio && group) {
        vaxp_radio_group_add(group, radio);
    }
}

void vaxp_radio_set_value(VaxpRadioButton* radio, int value) {
    if (radio) radio->value = value;
}

int vaxp_radio_get_value(const VaxpRadioButton* radio) {
    return radio ? radio->value : 0;
}

void vaxp_radio_set_on_select(VaxpRadioButton* radio, VaxpRadioCallback callback, void* data) {
    if (radio) {
        radio->on_select = callback;
        radio->callback_data = data;
    }
}

VaxpWidget* _vaxp_radio_build(const VaxpRadioConfig* config) {
    VaxpResultPtr result = vaxp_radio_create();
    if (!result.ok) return NULL;
    
    VaxpRadioButton* radio = (VaxpRadioButton*)result.value;
    
    if (config->label) vaxp_radio_set_label(radio, config->label);
    radio->value = config->value;
    radio->selected = config->selected;
    if (config->group) vaxp_radio_set_group(radio, config->group);
    radio->on_select = config->on_select;
    radio->callback_data = config->data;
    
    return (VaxpWidget*)radio;
}
