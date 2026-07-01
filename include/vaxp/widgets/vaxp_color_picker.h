/*
 * VAXPUI - ColorPicker widget header
 */

#ifndef VAXP_COLOR_PICKER_H
#define VAXP_COLOR_PICKER_H

#include "vaxp/widgets/vaxp_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VaxpColorPicker VaxpColorPicker;
typedef void (*VaxpColorPickerCallback)(VaxpColorPicker* picker, VaxpColor color, void* data);

typedef enum VaxpColorPickerMode {
    VAXP_COLOR_PICKER_HSV,       /* Hue/Saturation/Value wheel */
    VAXP_COLOR_PICKER_RGB,       /* RGB sliders */
    VAXP_COLOR_PICKER_PALETTE,   /* Preset color palette */
} VaxpColorPickerMode;

struct VaxpColorPicker {
    VaxpWidget base;
    
    VaxpColor selected_color;
    VaxpColorPickerMode mode;
    
    /* HSV representation */
    VaxpF32 hue;          /* 0-360 */
    VaxpF32 saturation;   /* 0-1 */
    VaxpF32 value;        /* 0-1 */
    VaxpF32 alpha;        /* 0-1 */
    
    VaxpBool show_alpha;
    VaxpBool show_preview;
    VaxpBool show_hex_input;
    
    /* Palette */
    VaxpColor* palette;
    VaxpU32 palette_count;
    VaxpBool show_palette;
    
    /* State */
    VaxpBool dragging_hue;
    VaxpBool dragging_sv;
    VaxpBool dragging_alpha;
    
    VaxpColorPickerCallback on_change;
    void* callback_data;
};

VaxpResultPtr vaxp_color_picker_create(void);
void vaxp_color_picker_set_color(VaxpColorPicker* picker, VaxpColor color);
VaxpColor vaxp_color_picker_get_color(const VaxpColorPicker* picker);
void vaxp_color_picker_set_mode(VaxpColorPicker* picker, VaxpColorPickerMode mode);
void vaxp_color_picker_set_show_alpha(VaxpColorPicker* picker, VaxpBool show);
void vaxp_color_picker_set_palette(VaxpColorPicker* picker, const VaxpColor* colors, VaxpU32 count);
void vaxp_color_picker_set_on_change(VaxpColorPicker* picker, VaxpColorPickerCallback cb, void* data);

extern const VaxpWidgetClass vaxp_color_picker_class;

#define vaxp_color_picker(...) _vaxp_color_picker_build(&(VaxpColorPickerConfig){ __VA_ARGS__ })

typedef struct VaxpColorPickerConfig {
    VaxpColor initial_color;
    VaxpColorPickerMode mode;
    VaxpBool show_alpha;
    VaxpColorPickerCallback on_change;
    void* data;
} VaxpColorPickerConfig;

VaxpWidget* _vaxp_color_picker_build(const VaxpColorPickerConfig* config);

#ifdef __cplusplus
}
#endif

#endif
