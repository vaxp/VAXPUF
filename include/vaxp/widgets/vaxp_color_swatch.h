/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_color_swatch.h - Color display/picker swatch
 */

#ifndef VAXP_COLOR_SWATCH_H
#define VAXP_COLOR_SWATCH_H

#include "vaxp/widgets/vaxp_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VaxpColorSwatch VaxpColorSwatch;
typedef void (*VaxpColorSwatchCallback)(VaxpColorSwatch* swatch, VaxpColor color, void* data);

struct VaxpColorSwatch {
    VaxpWidget base;
    
    VaxpColor color;
    VaxpF32 size;
    VaxpF32 corner_radius;
    VaxpBool show_border;
    VaxpBool selectable;
    VaxpBool selected;
    
    VaxpColorSwatchCallback on_click;
    void* callback_data;
};

VaxpResultPtr vaxp_color_swatch_create(void);
void vaxp_color_swatch_set_color(VaxpColorSwatch* swatch, VaxpColor color);
VaxpColor vaxp_color_swatch_get_color(const VaxpColorSwatch* swatch);
void vaxp_color_swatch_set_size(VaxpColorSwatch* swatch, VaxpF32 size);
void vaxp_color_swatch_set_on_click(VaxpColorSwatch* swatch, VaxpColorSwatchCallback callback, void* data);

extern const VaxpWidgetClass vaxp_color_swatch_class;

#define vaxp_color_swatch(...) _vaxp_color_swatch_build(&(VaxpColorSwatchConfig){ __VA_ARGS__ })

typedef struct VaxpColorSwatchConfig {
    VaxpColor color;
    VaxpF32 size;
    VaxpBool selectable;
    VaxpColorSwatchCallback on_click;
    void* data;
} VaxpColorSwatchConfig;

VaxpWidget* _vaxp_color_swatch_build(const VaxpColorSwatchConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VAXP_COLOR_SWATCH_H */
