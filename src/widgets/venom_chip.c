/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_chip.c - Chip widget implementation
 */

#include "venom/widgets/venom_chip.h"
#include "venom/core/venom_memory.h"
#include <string.h>

#define DEFAULT_HEIGHT 32.0f

static void chip_init(VenomWidget* widget) {
    VenomChip* chip = (VenomChip*)widget;
    
    chip->label = NULL;
    chip->icon = NULL;
    chip->type = VENOM_CHIP_CHOICE;
    chip->selected = VENOM_FALSE;
    chip->deletable = VENOM_FALSE;
    
    chip->on_click = NULL;
    chip->on_delete = NULL;
    chip->callback_data = NULL;
    
    chip->background_color = (VenomColor){ 224, 224, 224, 255 };
    chip->selected_color = (VenomColor){ 63, 81, 181, 255 };
    chip->text_color = (VenomColor){ 33, 33, 33, 255 };
    chip->delete_color = (VenomColor){ 97, 97, 97, 255 };
    chip->height = DEFAULT_HEIGHT;
    
    widget->focusable = VENOM_TRUE;
}

static void chip_destroy(VenomWidget* widget) {
    VenomChip* chip = (VenomChip*)widget;
    
    if (chip->label) venom_free(chip->label, strlen(chip->label) + 1);
    if (chip->icon) venom_free(chip->icon, strlen(chip->icon) + 1);
    
    venom_widget_class.destroy(widget);
}

static void chip_measure(VenomWidget* widget, VenomF32 available_width, VenomF32 available_height,
                         VenomF32* out_width, VenomF32* out_height) {
    VenomChip* chip = (VenomChip*)widget;
    (void)available_width; (void)available_height;
    
    VenomF32 w = 24; /* Padding */
    
    if (chip->type == VENOM_CHIP_FILTER && chip->selected) w += 20; /* Checkmark */
    if (chip->label) w += (VenomF32)strlen(chip->label) * 8;
    if (chip->deletable) w += 24; /* Delete button */
    
    *out_width = w;
    *out_height = chip->height;
}

static void chip_draw(VenomWidget* widget, VenomCanvas* canvas) {
    VenomChip* chip = (VenomChip*)widget;
    
    VenomF32 w = widget->bounds.width;
    VenomF32 h = chip->height;
    VenomF32 r = h / 2;
    
    /* Draw background */
    VenomColor bg = chip->selected ? chip->selected_color : chip->background_color;
    VenomRectF bg_rect = { 0, 0, w, h };
    VenomPaint bg_paint = venom_paint_fill(bg);
    venom_canvas_draw_rounded_rect(canvas, bg_rect, r, &bg_paint);
    
    VenomF32 x = 12;
    
    /* Draw checkmark for filter type */
    if (chip->type == VENOM_CHIP_FILTER && chip->selected) {
        VenomColor check_color = chip->selected ? 
            (VenomColor){ 255, 255, 255, 255 } : chip->text_color;
        VenomPaint check_paint = venom_paint_fill(check_color);
        venom_canvas_draw_text(canvas, "✓", x, h / 2 + 5, NULL, &check_paint);
        x += 18;
    }
    
    /* Draw label */
    if (chip->label) {
        VenomColor text_color = chip->selected ?
            (VenomColor){ 255, 255, 255, 255 } : chip->text_color;
        VenomPaint text_paint = venom_paint_fill(text_color);
        venom_canvas_draw_text(canvas, chip->label, x, h / 2 + 5, NULL, &text_paint);
    }
    
    /* Draw delete button */
    if (chip->deletable) {
        VenomPaint del_paint = venom_paint_fill(chip->delete_color);
        venom_canvas_draw_text(canvas, "✕", w - 20, h / 2 + 4, NULL, &del_paint);
    }
}

static VenomBool chip_on_event(VenomWidget* widget, const VenomEvent* event) {
    VenomChip* chip = (VenomChip*)widget;
    
    if (event->type == VENOM_EVENT_MOUSE_BUTTON_DOWN && 
        event->mouse.button == VENOM_MOUSE_BUTTON_LEFT) {
        
        /* Check delete button */
        if (chip->deletable && event->mouse.x > widget->bounds.width - 24) {
            if (chip->on_delete) {
                chip->on_delete(chip, chip->callback_data);
            }
            return VENOM_TRUE;
        }
        
        /* Toggle selection for choice/filter types */
        if (chip->type == VENOM_CHIP_CHOICE || chip->type == VENOM_CHIP_FILTER) {
            chip->selected = !chip->selected;
            widget->needs_redraw = VENOM_TRUE;
        }
        
        if (chip->on_click) {
            chip->on_click(chip, chip->callback_data);
        }
        return VENOM_TRUE;
    }
    
    if (event->type == VENOM_EVENT_KEY_DOWN && 
        (event->key.key == VENOM_KEY_RETURN || event->key.key == VENOM_KEY_SPACE)) {
        if (chip->type == VENOM_CHIP_CHOICE || chip->type == VENOM_CHIP_FILTER) {
            chip->selected = !chip->selected;
            widget->needs_redraw = VENOM_TRUE;
        }
        if (chip->on_click) chip->on_click(chip, chip->callback_data);
        return VENOM_TRUE;
    }
    
    return VENOM_FALSE;
}

const VenomWidgetClass venom_chip_class = {
    .class_name = "VenomChip",
    .instance_size = sizeof(VenomChip),
    .parent_class = &venom_widget_class,
    .init = chip_init,
    .destroy = chip_destroy,
    .measure = chip_measure,
    .layout = NULL,
    .draw = chip_draw,
    .on_event = chip_on_event,
    .on_state_changed = NULL,
};

VenomResultPtr venom_chip_create(void) {
    return venom_widget_create(&venom_chip_class);
}

void venom_chip_set_label(VenomChip* chip, const char* label) {
    if (!chip) return;
    
    if (chip->label) venom_free(chip->label, strlen(chip->label) + 1);
    chip->label = NULL;
    
    if (label) {
        VenomSize len = strlen(label) + 1;
        chip->label = (char*)venom_alloc(len);
        if (chip->label) memcpy(chip->label, label, len);
    }
}

void venom_chip_set_selected(VenomChip* chip, VenomBool selected) {
    if (chip) {
        chip->selected = selected;
        venom_widget_invalidate((VenomWidget*)chip);
    }
}

void venom_chip_set_on_click(VenomChip* chip, VenomChipCallback callback, void* data) {
    if (chip) {
        chip->on_click = callback;
        chip->callback_data = data;
    }
}

void venom_chip_set_on_delete(VenomChip* chip, VenomChipCallback callback, void* data) {
    if (chip) {
        chip->on_delete = callback;
        chip->deletable = (callback != NULL);
    }
}

VenomWidget* _venom_chip_build(const VenomChipConfig* config) {
    VenomResultPtr result = venom_chip_create();
    if (!result.ok) return NULL;
    
    VenomChip* chip = (VenomChip*)result.value;
    
    if (config->label) venom_chip_set_label(chip, config->label);
    chip->type = config->type;
    chip->selected = config->selected;
    chip->on_click = config->on_click;
    chip->callback_data = config->data;
    
    return (VenomWidget*)chip;
}
