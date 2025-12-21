/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_slider.h - Slider widget (value selection)
 */

#ifndef VENOM_SLIDER_H
#define VENOM_SLIDER_H

#include "venom/widgets/venom_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VenomSlider VenomSlider;

typedef void (*VenomSliderCallback)(VenomSlider* slider, VenomF32 value, void* user_data);

struct VenomSlider {
    VenomWidget base;
    
    VenomF32 value;          /* Current value (0.0 - 1.0) */
    VenomF32 min;            /* Minimum value */
    VenomF32 max;            /* Maximum value */
    VenomF32 step;           /* Step size (0 = continuous) */
    
    VenomBool enabled;
    VenomBool dragging;
    
    /* Styling */
    VenomF32 track_height;
    VenomF32 thumb_radius;
    VenomColor track_color;
    VenomColor fill_color;
    VenomColor thumb_color;
    
    /* Callback */
    VenomSliderCallback on_change;
    void* callback_data;
};

VenomResultPtr venom_slider_create(void);
void venom_slider_set_value(VenomSlider* slider, VenomF32 value);
VenomF32 venom_slider_get_value(const VenomSlider* slider);
void venom_slider_set_range(VenomSlider* slider, VenomF32 min, VenomF32 max);
void venom_slider_set_step(VenomSlider* slider, VenomF32 step);
void venom_slider_set_on_change(VenomSlider* slider, VenomSliderCallback callback, void* data);

extern const VenomWidgetClass venom_slider_class;

#define venom_slider(...) _venom_slider_build(&(VenomSliderConfig){ __VA_ARGS__ })

typedef struct VenomSliderConfig {
    VenomF32 value;
    VenomF32 min;
    VenomF32 max;
    VenomF32 step;
    VenomSliderCallback on_change;
    void* data;
} VenomSliderConfig;

VenomWidget* _venom_slider_build(const VenomSliderConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VENOM_SLIDER_H */
