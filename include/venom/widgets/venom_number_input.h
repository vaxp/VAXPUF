/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_number_input.h - Numeric input with +/- buttons
 */

#ifndef VENOM_NUMBER_INPUT_H
#define VENOM_NUMBER_INPUT_H

#include "venom/widgets/venom_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VenomNumberInput VenomNumberInput;
typedef void (*VenomNumberCallback)(VenomNumberInput* input, VenomF32 value, void* data);

struct VenomNumberInput {
    VenomWidget base;
    
    VenomF32 value;
    VenomF32 min;
    VenomF32 max;
    VenomF32 step;
    VenomU32 decimals;
    
    VenomNumberCallback on_change;
    void* callback_data;
    
    VenomColor background_color;
    VenomColor border_color;
    VenomColor button_color;
    VenomColor text_color;
    VenomF32 height;
};

VenomResultPtr venom_number_input_create(void);
void venom_number_input_set_value(VenomNumberInput* input, VenomF32 value);
VenomF32 venom_number_input_get_value(const VenomNumberInput* input);
void venom_number_input_set_range(VenomNumberInput* input, VenomF32 min, VenomF32 max);
void venom_number_input_set_step(VenomNumberInput* input, VenomF32 step);
void venom_number_input_set_on_change(VenomNumberInput* input, VenomNumberCallback callback, void* data);

extern const VenomWidgetClass venom_number_input_class;

#define venom_number_input(...) _venom_number_input_build(&(VenomNumberInputConfig){ __VA_ARGS__ })

typedef struct VenomNumberInputConfig {
    VenomF32 value;
    VenomF32 min;
    VenomF32 max;
    VenomF32 step;
    VenomNumberCallback on_change;
    void* data;
} VenomNumberInputConfig;

VenomWidget* _venom_number_input_build(const VenomNumberInputConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VENOM_NUMBER_INPUT_H */
