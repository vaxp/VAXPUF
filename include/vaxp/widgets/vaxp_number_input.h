/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_number_input.h - Numeric input with +/- buttons
 */

#ifndef VAXP_NUMBER_INPUT_H
#define VAXP_NUMBER_INPUT_H

#include "vaxp/widgets/vaxp_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VaxpNumberInput VaxpNumberInput;
typedef void (*VaxpNumberCallback)(VaxpNumberInput* input, VaxpF32 value, void* data);

struct VaxpNumberInput {
    VaxpWidget base;
    
    VaxpF32 value;
    VaxpF32 min;
    VaxpF32 max;
    VaxpF32 step;
    VaxpU32 decimals;
    
    VaxpNumberCallback on_change;
    void* callback_data;
    
    VaxpColor background_color;
    VaxpColor border_color;
    VaxpColor button_color;
    VaxpColor text_color;
    VaxpF32 height;
};

VaxpResultPtr vaxp_number_input_create(void);
void vaxp_number_input_set_value(VaxpNumberInput* input, VaxpF32 value);
VaxpF32 vaxp_number_input_get_value(const VaxpNumberInput* input);
void vaxp_number_input_set_range(VaxpNumberInput* input, VaxpF32 min, VaxpF32 max);
void vaxp_number_input_set_step(VaxpNumberInput* input, VaxpF32 step);
void vaxp_number_input_set_on_change(VaxpNumberInput* input, VaxpNumberCallback callback, void* data);

extern const VaxpWidgetClass vaxp_number_input_class;

#define vaxp_number_input(...) _vaxp_number_input_build(&(VaxpNumberInputConfig){ __VA_ARGS__ })

typedef struct VaxpNumberInputConfig {
    VaxpF32 value;
    VaxpF32 min;
    VaxpF32 max;
    VaxpF32 step;
    VaxpNumberCallback on_change;
    void* data;
} VaxpNumberInputConfig;

VaxpWidget* _vaxp_number_input_build(const VaxpNumberInputConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VAXP_NUMBER_INPUT_H */
