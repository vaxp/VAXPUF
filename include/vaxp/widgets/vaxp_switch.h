/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_switch.h - Switch widget (toggle on/off)
 */

#ifndef VAXP_SWITCH_H
#define VAXP_SWITCH_H

#include "vaxp/widgets/vaxp_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VaxpSwitch VaxpSwitch;

typedef void (*VaxpSwitchCallback)(VaxpSwitch* sw, VaxpBool on, void* user_data);

struct VaxpSwitch {
    VaxpWidget base;
    
    VaxpBool on;
    VaxpBool enabled;
    
    /* Styling */
    VaxpF32 width;
    VaxpF32 height;
    VaxpColor on_color;
    VaxpColor off_color;
    VaxpColor thumb_color;
    
    /* Callback */
    VaxpSwitchCallback on_change;
    void* callback_data;
};

VaxpResultPtr vaxp_switch_create(void);
void vaxp_switch_set_on(VaxpSwitch* sw, VaxpBool on);
VaxpBool vaxp_switch_is_on(const VaxpSwitch* sw);
void vaxp_switch_set_enabled(VaxpSwitch* sw, VaxpBool enabled);
void vaxp_switch_set_on_change(VaxpSwitch* sw, VaxpSwitchCallback callback, void* data);

extern const VaxpWidgetClass vaxp_switch_class;

#define vaxp_switch(...) _vaxp_switch_build(&(VaxpSwitchConfig){ __VA_ARGS__ })

typedef struct VaxpSwitchConfig {
    VaxpBool on;
    VaxpSwitchCallback on_change;
    void* data;
} VaxpSwitchConfig;

VaxpWidget* _vaxp_switch_build(const VaxpSwitchConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VAXP_SWITCH_H */
