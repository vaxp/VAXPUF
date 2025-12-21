/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_switch.h - Switch widget (toggle on/off)
 */

#ifndef VENOM_SWITCH_H
#define VENOM_SWITCH_H

#include "venom/widgets/venom_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VenomSwitch VenomSwitch;

typedef void (*VenomSwitchCallback)(VenomSwitch* sw, VenomBool on, void* user_data);

struct VenomSwitch {
    VenomWidget base;
    
    VenomBool on;
    VenomBool enabled;
    
    /* Styling */
    VenomF32 width;
    VenomF32 height;
    VenomColor on_color;
    VenomColor off_color;
    VenomColor thumb_color;
    
    /* Callback */
    VenomSwitchCallback on_change;
    void* callback_data;
};

VenomResultPtr venom_switch_create(void);
void venom_switch_set_on(VenomSwitch* sw, VenomBool on);
VenomBool venom_switch_is_on(const VenomSwitch* sw);
void venom_switch_set_enabled(VenomSwitch* sw, VenomBool enabled);
void venom_switch_set_on_change(VenomSwitch* sw, VenomSwitchCallback callback, void* data);

extern const VenomWidgetClass venom_switch_class;

#define venom_switch(...) _venom_switch_build(&(VenomSwitchConfig){ __VA_ARGS__ })

typedef struct VenomSwitchConfig {
    VenomBool on;
    VenomSwitchCallback on_change;
    void* data;
} VenomSwitchConfig;

VenomWidget* _venom_switch_build(const VenomSwitchConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VENOM_SWITCH_H */
