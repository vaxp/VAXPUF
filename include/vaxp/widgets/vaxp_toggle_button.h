/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_toggle_button.h - Toggle button (like a button that stays pressed)
 */

#ifndef VAXP_TOGGLE_BUTTON_H
#define VAXP_TOGGLE_BUTTON_H

#include "vaxp/widgets/vaxp_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VaxpToggleButton VaxpToggleButton;
typedef void (*VaxpToggleButtonCallback)(VaxpToggleButton* btn, VaxpBool toggled, void* data);

struct VaxpToggleButton {
    VaxpWidget base;
    
    char* label;
    VaxpBool toggled;
    
    VaxpToggleButtonCallback on_toggle;
    void* callback_data;
    
    VaxpColor normal_color;
    VaxpColor toggled_color;
    VaxpColor text_color;
    VaxpColor toggled_text_color;
    VaxpF32 corner_radius;
    VaxpF32 height;
};

VaxpResultPtr vaxp_toggle_button_create(void);
void vaxp_toggle_button_set_label(VaxpToggleButton* btn, const char* label);
void vaxp_toggle_button_set_toggled(VaxpToggleButton* btn, VaxpBool toggled);
VaxpBool vaxp_toggle_button_get_toggled(const VaxpToggleButton* btn);
void vaxp_toggle_button_set_on_toggle(VaxpToggleButton* btn, VaxpToggleButtonCallback callback, void* data);

extern const VaxpWidgetClass vaxp_toggle_button_class;

#define vaxp_toggle_button(...) _vaxp_toggle_button_build(&(VaxpToggleButtonConfig){ __VA_ARGS__ })

typedef struct VaxpToggleButtonConfig {
    const char* label;
    VaxpBool toggled;
    VaxpToggleButtonCallback on_toggle;
    void* data;
} VaxpToggleButtonConfig;

VaxpWidget* _vaxp_toggle_button_build(const VaxpToggleButtonConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VAXP_TOGGLE_BUTTON_H */
