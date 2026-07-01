/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_slider.h - Slider widget (value selection)
 */

#ifndef VAXP_SLIDER_H
#define VAXP_SLIDER_H

#include "vaxp/widgets/vaxp_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VaxpSlider VaxpSlider;

typedef void (*VaxpSliderCallback)(VaxpSlider* slider, VaxpF32 value, void* user_data);

struct VaxpSlider {
    VaxpWidget base;
    
    VaxpF32 value;          /* Current value (0.0 - 1.0) */
    VaxpF32 min;            /* Minimum value */
    VaxpF32 max;            /* Maximum value */
    VaxpF32 step;           /* Step size (0 = continuous) */
    
    VaxpBool enabled;
    VaxpBool dragging;
    
    /* Styling */
    VaxpF32 track_height;
    VaxpF32 thumb_radius;
    VaxpColor track_color;
    VaxpColor fill_color;
    VaxpColor thumb_color;
    
    /* Callback */
    VaxpSliderCallback on_change;
    void* callback_data;
};

VaxpResultPtr vaxp_slider_create(void);
void vaxp_slider_set_value(VaxpSlider* slider, VaxpF32 value);
VaxpF32 vaxp_slider_get_value(const VaxpSlider* slider);
void vaxp_slider_set_range(VaxpSlider* slider, VaxpF32 min, VaxpF32 max);
void vaxp_slider_set_step(VaxpSlider* slider, VaxpF32 step);
void vaxp_slider_set_on_change(VaxpSlider* slider, VaxpSliderCallback callback, void* data);

extern const VaxpWidgetClass vaxp_slider_class;

#define vaxp_slider(...) _vaxp_slider_build(&(VaxpSliderConfig){ __VA_ARGS__ })

typedef struct VaxpSliderConfig {
    VaxpF32 value;
    VaxpF32 min;
    VaxpF32 max;
    VaxpF32 step;
    VaxpSliderCallback on_change;
    void* data;
} VaxpSliderConfig;

VaxpWidget* _vaxp_slider_build(const VaxpSliderConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VAXP_SLIDER_H */
