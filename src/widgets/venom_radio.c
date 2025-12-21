/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_radio.c - Radio button widget implementation
 */

#include "venom/widgets/venom_radio.h"
#include "venom/core/venom_memory.h"
#include <string.h>

#define DEFAULT_CIRCLE_SIZE 20.0f
#define DEFAULT_SPACING 8.0f
#define INITIAL_GROUP_CAPACITY 8

/* ============================================================================
 * RADIO GROUP
 * ============================================================================ */

VenomRadioGroup* venom_radio_group_create(void) {
    VenomRadioGroup* group = (VenomRadioGroup*)venom_alloc(sizeof(VenomRadioGroup));
    if (!group) return NULL;
    
    group->buttons = NULL;
    group->count = 0;
    group->capacity = 0;
    group->selected = NULL;
    
    return group;
}

void venom_radio_group_destroy(VenomRadioGroup* group) {
    if (!group) return;
    
    if (group->buttons) {
        venom_free(group->buttons, group->capacity * sizeof(VenomRadioButton*));
    }
    venom_free(group, sizeof(VenomRadioGroup));
}

void venom_radio_group_add(VenomRadioGroup* group, VenomRadioButton* radio) {
    if (!group || !radio) return;
    
    /* Expand if needed */
    if (group->count >= group->capacity) {
        VenomU32 new_cap = group->capacity == 0 ? INITIAL_GROUP_CAPACITY : group->capacity * 2;
        VenomRadioButton** new_arr = (VenomRadioButton**)venom_alloc(new_cap * sizeof(VenomRadioButton*));
        if (!new_arr) return;
        
        if (group->buttons) {
            memcpy(new_arr, group->buttons, group->count * sizeof(VenomRadioButton*));
            venom_free(group->buttons, group->capacity * sizeof(VenomRadioButton*));
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

VenomRadioButton* venom_radio_group_get_selected(const VenomRadioGroup* group) {
    return group ? group->selected : NULL;
}

/* ============================================================================
 * RADIO BUTTON
 * ============================================================================ */

static void radio_init(VenomWidget* widget) {
    VenomRadioButton* radio = (VenomRadioButton*)widget;
    
    radio->selected = VENOM_FALSE;
    radio->enabled = VENOM_TRUE;
    radio->label = NULL;
    radio->group = NULL;
    radio->value = 0;
    
    radio->circle_size = DEFAULT_CIRCLE_SIZE;
    radio->spacing = DEFAULT_SPACING;
    radio->selected_color = (VenomColor){ 63, 81, 181, 255 };
    radio->unselected_color = (VenomColor){ 150, 150, 150, 255 };
    radio->label_color = (VenomColor){ 33, 33, 33, 255 };
    
    radio->on_select = NULL;
    radio->callback_data = NULL;
    
    widget->focusable = VENOM_TRUE;
}

static void radio_destroy(VenomWidget* widget) {
    VenomRadioButton* radio = (VenomRadioButton*)widget;
    
    if (radio->label) {
        venom_free(radio->label, strlen(radio->label) + 1);
        radio->label = NULL;
    }
    
    venom_widget_class.destroy(widget);
}

static void radio_measure(VenomWidget* widget, VenomF32 available_width, VenomF32 available_height,
                          VenomF32* out_width, VenomF32* out_height) {
    VenomRadioButton* radio = (VenomRadioButton*)widget;
    (void)available_width; (void)available_height;
    
    *out_width = radio->circle_size;
    *out_height = radio->circle_size;
    
    if (radio->label && radio->label[0]) {
        *out_width += radio->spacing + (VenomF32)strlen(radio->label) * 8.0f;
    }
}

static void radio_draw(VenomWidget* widget, VenomCanvas* canvas) {
    VenomRadioButton* radio = (VenomRadioButton*)widget;
    
    VenomF32 cx = radio->circle_size / 2;
    VenomF32 cy = widget->bounds.height / 2;
    VenomF32 r = radio->circle_size / 2;
    
    /* Draw outer circle */
    VenomColor outer_color = radio->selected ? radio->selected_color : radio->unselected_color;
    if (widget->state & VENOM_WIDGET_STATE_FOCUSED) {
        outer_color = radio->selected_color;
    }
    VenomPaint outer_paint = venom_paint_stroke(outer_color, 2.0f);
    venom_canvas_draw_circle(canvas, cx, cy, r - 2, &outer_paint);
    
    /* Draw inner filled circle when selected */
    if (radio->selected) {
        VenomPaint inner_paint = venom_paint_fill(radio->selected_color);
        venom_canvas_draw_circle(canvas, cx, cy, r * 0.45f, &inner_paint);
    }
    
    /* Draw label */
    if (radio->label && radio->label[0]) {
        VenomPaint text_paint = venom_paint_fill(radio->enabled ? radio->label_color : 
                                                  (VenomColor){ 150, 150, 150, 255 });
        VenomF32 text_x = radio->circle_size + radio->spacing;
        VenomF32 text_y = widget->bounds.height / 2 + 5;
        venom_canvas_draw_text(canvas, radio->label, text_x, text_y, NULL, &text_paint);
    }
}

static VenomBool radio_on_event(VenomWidget* widget, const VenomEvent* event) {
    VenomRadioButton* radio = (VenomRadioButton*)widget;
    
    if (!radio->enabled) return VENOM_FALSE;
    
    VenomBool activate = VENOM_FALSE;
    
    switch (event->type) {
        case VENOM_EVENT_MOUSE_BUTTON_DOWN:
            if (event->mouse.button == VENOM_MOUSE_BUTTON_LEFT) {
                activate = VENOM_TRUE;
            }
            break;
            
        case VENOM_EVENT_KEY_DOWN:
            if (event->key.key == VENOM_KEY_SPACE || event->key.key == VENOM_KEY_RETURN) {
                activate = VENOM_TRUE;
            }
            break;
            
        default:
            break;
    }
    
    if (activate && !radio->selected) {
        /* Deselect all other radios in group */
        if (radio->group) {
            for (VenomU32 i = 0; i < radio->group->count; i++) {
                VenomRadioButton* other = radio->group->buttons[i];
                if (other != radio && other->selected) {
                    other->selected = VENOM_FALSE;
                    ((VenomWidget*)other)->needs_redraw = VENOM_TRUE;
                }
            }
            radio->group->selected = radio;
        }
        
        radio->selected = VENOM_TRUE;
        widget->needs_redraw = VENOM_TRUE;
        
        if (radio->on_select) {
            radio->on_select(radio, radio->callback_data);
        }
        
        return VENOM_TRUE;
    }
    
    return VENOM_FALSE;
}

const VenomWidgetClass venom_radio_class = {
    .class_name = "VenomRadioButton",
    .instance_size = sizeof(VenomRadioButton),
    .parent_class = &venom_widget_class,
    .init = radio_init,
    .destroy = radio_destroy,
    .measure = radio_measure,
    .layout = NULL,
    .draw = radio_draw,
    .on_event = radio_on_event,
    .on_state_changed = NULL,
};

VenomResultPtr venom_radio_create(void) {
    return venom_widget_create(&venom_radio_class);
}

void venom_radio_set_selected(VenomRadioButton* radio, VenomBool selected) {
    if (!radio) return;
    
    if (selected && !radio->selected) {
        /* Deselect others in group */
        if (radio->group) {
            for (VenomU32 i = 0; i < radio->group->count; i++) {
                VenomRadioButton* other = radio->group->buttons[i];
                if (other != radio) {
                    other->selected = VENOM_FALSE;
                    venom_widget_invalidate((VenomWidget*)other);
                }
            }
            radio->group->selected = radio;
        }
    }
    
    radio->selected = selected;
    venom_widget_invalidate((VenomWidget*)radio);
}

VenomBool venom_radio_is_selected(const VenomRadioButton* radio) {
    return radio ? radio->selected : VENOM_FALSE;
}

void venom_radio_set_label(VenomRadioButton* radio, const char* label) {
    if (!radio) return;
    
    if (radio->label) {
        venom_free(radio->label, strlen(radio->label) + 1);
        radio->label = NULL;
    }
    
    if (label) {
        VenomSize len = strlen(label) + 1;
        radio->label = (char*)venom_alloc(len);
        if (radio->label) {
            memcpy(radio->label, label, len);
        }
    }
    
    venom_widget_invalidate((VenomWidget*)radio);
}

void venom_radio_set_group(VenomRadioButton* radio, VenomRadioGroup* group) {
    if (radio && group) {
        venom_radio_group_add(group, radio);
    }
}

void venom_radio_set_value(VenomRadioButton* radio, int value) {
    if (radio) radio->value = value;
}

int venom_radio_get_value(const VenomRadioButton* radio) {
    return radio ? radio->value : 0;
}

void venom_radio_set_on_select(VenomRadioButton* radio, VenomRadioCallback callback, void* data) {
    if (radio) {
        radio->on_select = callback;
        radio->callback_data = data;
    }
}

VenomWidget* _venom_radio_build(const VenomRadioConfig* config) {
    VenomResultPtr result = venom_radio_create();
    if (!result.ok) return NULL;
    
    VenomRadioButton* radio = (VenomRadioButton*)result.value;
    
    if (config->label) venom_radio_set_label(radio, config->label);
    radio->value = config->value;
    radio->selected = config->selected;
    if (config->group) venom_radio_set_group(radio, config->group);
    radio->on_select = config->on_select;
    radio->callback_data = config->data;
    
    return (VenomWidget*)radio;
}
