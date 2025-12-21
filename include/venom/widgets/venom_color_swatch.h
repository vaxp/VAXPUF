/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_color_swatch.h - Color display/picker swatch
 */

#ifndef VENOM_COLOR_SWATCH_H
#define VENOM_COLOR_SWATCH_H

#include "venom/widgets/venom_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VenomColorSwatch VenomColorSwatch;
typedef void (*VenomColorSwatchCallback)(VenomColorSwatch* swatch, VenomColor color, void* data);

struct VenomColorSwatch {
    VenomWidget base;
    
    VenomColor color;
    VenomF32 size;
    VenomF32 corner_radius;
    VenomBool show_border;
    VenomBool selectable;
    VenomBool selected;
    
    VenomColorSwatchCallback on_click;
    void* callback_data;
};

VenomResultPtr venom_color_swatch_create(void);
void venom_color_swatch_set_color(VenomColorSwatch* swatch, VenomColor color);
VenomColor venom_color_swatch_get_color(const VenomColorSwatch* swatch);
void venom_color_swatch_set_size(VenomColorSwatch* swatch, VenomF32 size);
void venom_color_swatch_set_on_click(VenomColorSwatch* swatch, VenomColorSwatchCallback callback, void* data);

extern const VenomWidgetClass venom_color_swatch_class;

#define venom_color_swatch(...) _venom_color_swatch_build(&(VenomColorSwatchConfig){ __VA_ARGS__ })

typedef struct VenomColorSwatchConfig {
    VenomColor color;
    VenomF32 size;
    VenomBool selectable;
    VenomColorSwatchCallback on_click;
    void* data;
} VenomColorSwatchConfig;

VenomWidget* _venom_color_swatch_build(const VenomColorSwatchConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VENOM_COLOR_SWATCH_H */
