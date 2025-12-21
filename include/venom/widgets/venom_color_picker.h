/*
 * VENOMUI - ColorPicker widget header
 */

#ifndef VENOM_COLOR_PICKER_H
#define VENOM_COLOR_PICKER_H

#include "venom/widgets/venom_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VenomColorPicker VenomColorPicker;
typedef void (*VenomColorPickerCallback)(VenomColorPicker* picker, VenomColor color, void* data);

typedef enum VenomColorPickerMode {
    VENOM_COLOR_PICKER_HSV,       /* Hue/Saturation/Value wheel */
    VENOM_COLOR_PICKER_RGB,       /* RGB sliders */
    VENOM_COLOR_PICKER_PALETTE,   /* Preset color palette */
} VenomColorPickerMode;

struct VenomColorPicker {
    VenomWidget base;
    
    VenomColor selected_color;
    VenomColorPickerMode mode;
    
    /* HSV representation */
    VenomF32 hue;          /* 0-360 */
    VenomF32 saturation;   /* 0-1 */
    VenomF32 value;        /* 0-1 */
    VenomF32 alpha;        /* 0-1 */
    
    VenomBool show_alpha;
    VenomBool show_preview;
    VenomBool show_hex_input;
    
    /* Palette */
    VenomColor* palette;
    VenomU32 palette_count;
    VenomBool show_palette;
    
    /* State */
    VenomBool dragging_hue;
    VenomBool dragging_sv;
    VenomBool dragging_alpha;
    
    VenomColorPickerCallback on_change;
    void* callback_data;
};

VenomResultPtr venom_color_picker_create(void);
void venom_color_picker_set_color(VenomColorPicker* picker, VenomColor color);
VenomColor venom_color_picker_get_color(const VenomColorPicker* picker);
void venom_color_picker_set_mode(VenomColorPicker* picker, VenomColorPickerMode mode);
void venom_color_picker_set_show_alpha(VenomColorPicker* picker, VenomBool show);
void venom_color_picker_set_palette(VenomColorPicker* picker, const VenomColor* colors, VenomU32 count);
void venom_color_picker_set_on_change(VenomColorPicker* picker, VenomColorPickerCallback cb, void* data);

extern const VenomWidgetClass venom_color_picker_class;

#define venom_color_picker(...) _venom_color_picker_build(&(VenomColorPickerConfig){ __VA_ARGS__ })

typedef struct VenomColorPickerConfig {
    VenomColor initial_color;
    VenomColorPickerMode mode;
    VenomBool show_alpha;
    VenomColorPickerCallback on_change;
    void* data;
} VenomColorPickerConfig;

VenomWidget* _venom_color_picker_build(const VenomColorPickerConfig* config);

#ifdef __cplusplus
}
#endif

#endif
