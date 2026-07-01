/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_chip.c - Chip widget implementation
 */

#include "vaxp/widgets/vaxp_chip.h"
#include "vaxp/core/vaxp_memory.h"
#include <string.h>

#define DEFAULT_HEIGHT 32.0f

static void chip_init(VaxpWidget* widget) {
    VaxpChip* chip = (VaxpChip*)widget;
    
    chip->label = NULL;
    chip->icon = NULL;
    chip->type = VAXP_CHIP_CHOICE;
    chip->selected = VAXP_FALSE;
    chip->deletable = VAXP_FALSE;
    
    chip->on_click = NULL;
    chip->on_delete = NULL;
    chip->callback_data = NULL;
    
    chip->background_color = (VaxpColor){ 224, 224, 224, 255 };
    chip->selected_color = (VaxpColor){ 63, 81, 181, 255 };
    chip->text_color = (VaxpColor){ 33, 33, 33, 255 };
    chip->delete_color = (VaxpColor){ 97, 97, 97, 255 };
    chip->height = DEFAULT_HEIGHT;
    
    widget->focusable = VAXP_TRUE;
}

static void chip_destroy(VaxpWidget* widget) {
    VaxpChip* chip = (VaxpChip*)widget;
    
    if (chip->label) vaxp_free(chip->label, strlen(chip->label) + 1);
    if (chip->icon) vaxp_free(chip->icon, strlen(chip->icon) + 1);
    
    vaxp_widget_class.destroy(widget);
}

static void chip_measure(VaxpWidget* widget, VaxpF32 available_width, VaxpF32 available_height,
                         VaxpF32* out_width, VaxpF32* out_height) {
    VaxpChip* chip = (VaxpChip*)widget;
    (void)available_width; (void)available_height;
    
    VaxpF32 w = 24; /* Padding */
    
    if (chip->type == VAXP_CHIP_FILTER && chip->selected) w += 20; /* Checkmark */
    if (chip->label) w += (VaxpF32)strlen(chip->label) * 8;
    if (chip->deletable) w += 24; /* Delete button */
    
    *out_width = w;
    *out_height = chip->height;
}

static void chip_draw(VaxpWidget* widget, VaxpCanvas* canvas) {
    VaxpChip* chip = (VaxpChip*)widget;
    
    VaxpF32 w = widget->bounds.width;
    VaxpF32 h = chip->height;
    VaxpF32 r = h / 2;
    
    /* Draw background */
    VaxpColor bg = chip->selected ? chip->selected_color : chip->background_color;
    VaxpRectF bg_rect = { 0, 0, w, h };
    VaxpPaint bg_paint = vaxp_paint_fill(bg);
    vaxp_canvas_draw_rounded_rect(canvas, bg_rect, r, &bg_paint);
    
    VaxpF32 x = 12;
    
    /* Draw checkmark for filter type */
    if (chip->type == VAXP_CHIP_FILTER && chip->selected) {
        VaxpColor check_color = chip->selected ? 
            (VaxpColor){ 255, 255, 255, 255 } : chip->text_color;
        VaxpPaint check_paint = vaxp_paint_fill(check_color);
        vaxp_canvas_draw_text(canvas, "✓", x, h / 2 + 5, NULL, &check_paint);
        x += 18;
    }
    
    /* Draw label */
    if (chip->label) {
        VaxpColor text_color = chip->selected ?
            (VaxpColor){ 255, 255, 255, 255 } : chip->text_color;
        VaxpPaint text_paint = vaxp_paint_fill(text_color);
        vaxp_canvas_draw_text(canvas, chip->label, x, h / 2 + 5, NULL, &text_paint);
    }
    
    /* Draw delete button */
    if (chip->deletable) {
        VaxpPaint del_paint = vaxp_paint_fill(chip->delete_color);
        vaxp_canvas_draw_text(canvas, "✕", w - 20, h / 2 + 4, NULL, &del_paint);
    }
}

static VaxpBool chip_on_event(VaxpWidget* widget, const VaxpEvent* event) {
    VaxpChip* chip = (VaxpChip*)widget;
    
    if (event->type == VAXP_EVENT_MOUSE_BUTTON_DOWN && 
        event->mouse.button == VAXP_MOUSE_BUTTON_LEFT) {
        
        /* Check delete button */
        if (chip->deletable && event->mouse.x > widget->bounds.width - 24) {
            if (chip->on_delete) {
                chip->on_delete(chip, chip->callback_data);
            }
            return VAXP_TRUE;
        }
        
        /* Toggle selection for choice/filter types */
        if (chip->type == VAXP_CHIP_CHOICE || chip->type == VAXP_CHIP_FILTER) {
            chip->selected = !chip->selected;
            widget->needs_redraw = VAXP_TRUE;
        }
        
        if (chip->on_click) {
            chip->on_click(chip, chip->callback_data);
        }
        return VAXP_TRUE;
    }
    
    if (event->type == VAXP_EVENT_KEY_DOWN && 
        (event->key.key == VAXP_KEY_RETURN || event->key.key == VAXP_KEY_SPACE)) {
        if (chip->type == VAXP_CHIP_CHOICE || chip->type == VAXP_CHIP_FILTER) {
            chip->selected = !chip->selected;
            widget->needs_redraw = VAXP_TRUE;
        }
        if (chip->on_click) chip->on_click(chip, chip->callback_data);
        return VAXP_TRUE;
    }
    
    return VAXP_FALSE;
}

const VaxpWidgetClass vaxp_chip_class = {
    .class_name = "VaxpChip",
    .instance_size = sizeof(VaxpChip),
    .parent_class = &vaxp_widget_class,
    .init = chip_init,
    .destroy = chip_destroy,
    .measure = chip_measure,
    .layout = NULL,
    .draw = chip_draw,
    .on_event = chip_on_event,
    .on_state_changed = NULL,
};

VaxpResultPtr vaxp_chip_create(void) {
    return vaxp_widget_create(&vaxp_chip_class);
}

void vaxp_chip_set_label(VaxpChip* chip, const char* label) {
    if (!chip) return;
    
    if (chip->label) vaxp_free(chip->label, strlen(chip->label) + 1);
    chip->label = NULL;
    
    if (label) {
        VaxpSize len = strlen(label) + 1;
        chip->label = (char*)vaxp_alloc(len);
        if (chip->label) memcpy(chip->label, label, len);
    }
}

void vaxp_chip_set_selected(VaxpChip* chip, VaxpBool selected) {
    if (chip) {
        chip->selected = selected;
        vaxp_widget_invalidate((VaxpWidget*)chip);
    }
}

void vaxp_chip_set_on_click(VaxpChip* chip, VaxpChipCallback callback, void* data) {
    if (chip) {
        chip->on_click = callback;
        chip->callback_data = data;
    }
}

void vaxp_chip_set_on_delete(VaxpChip* chip, VaxpChipCallback callback, void* data) {
    if (chip) {
        chip->on_delete = callback;
        chip->deletable = (callback != NULL);
    }
}

VaxpWidget* _vaxp_chip_build(const VaxpChipConfig* config) {
    VaxpResultPtr result = vaxp_chip_create();
    if (!result.ok) return NULL;
    
    VaxpChip* chip = (VaxpChip*)result.value;
    
    if (config->label) vaxp_chip_set_label(chip, config->label);
    chip->type = config->type;
    chip->selected = config->selected;
    chip->on_click = config->on_click;
    chip->callback_data = config->data;
    
    return (VaxpWidget*)chip;
}
