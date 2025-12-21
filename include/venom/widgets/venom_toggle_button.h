/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_toggle_button.h - Toggle button (like a button that stays pressed)
 */

#ifndef VENOM_TOGGLE_BUTTON_H
#define VENOM_TOGGLE_BUTTON_H

#include "venom/widgets/venom_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VenomToggleButton VenomToggleButton;
typedef void (*VenomToggleButtonCallback)(VenomToggleButton* btn, VenomBool toggled, void* data);

struct VenomToggleButton {
    VenomWidget base;
    
    char* label;
    VenomBool toggled;
    
    VenomToggleButtonCallback on_toggle;
    void* callback_data;
    
    VenomColor normal_color;
    VenomColor toggled_color;
    VenomColor text_color;
    VenomColor toggled_text_color;
    VenomF32 corner_radius;
    VenomF32 height;
};

VenomResultPtr venom_toggle_button_create(void);
void venom_toggle_button_set_label(VenomToggleButton* btn, const char* label);
void venom_toggle_button_set_toggled(VenomToggleButton* btn, VenomBool toggled);
VenomBool venom_toggle_button_get_toggled(const VenomToggleButton* btn);
void venom_toggle_button_set_on_toggle(VenomToggleButton* btn, VenomToggleButtonCallback callback, void* data);

extern const VenomWidgetClass venom_toggle_button_class;

#define venom_toggle_button(...) _venom_toggle_button_build(&(VenomToggleButtonConfig){ __VA_ARGS__ })

typedef struct VenomToggleButtonConfig {
    const char* label;
    VenomBool toggled;
    VenomToggleButtonCallback on_toggle;
    void* data;
} VenomToggleButtonConfig;

VenomWidget* _venom_toggle_button_build(const VenomToggleButtonConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VENOM_TOGGLE_BUTTON_H */
