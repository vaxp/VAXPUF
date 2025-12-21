/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_checkbox.h - Checkbox widget (boolean toggle)
 */

#ifndef VENOM_CHECKBOX_H
#define VENOM_CHECKBOX_H

#include "venom/widgets/venom_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VenomCheckbox VenomCheckbox;

typedef void (*VenomCheckboxCallback)(VenomCheckbox* checkbox, VenomBool checked, void* user_data);

struct VenomCheckbox {
    VenomWidget base;
    
    VenomBool checked;
    VenomBool enabled;
    char* label;
    
    /* Styling */
    VenomF32 box_size;
    VenomF32 spacing;        /* Space between box and label */
    VenomColor check_color;
    VenomColor box_color;
    VenomColor label_color;
    
    /* Callback */
    VenomCheckboxCallback on_change;
    void* callback_data;
};

VenomResultPtr venom_checkbox_create(void);
void venom_checkbox_set_checked(VenomCheckbox* cb, VenomBool checked);
VenomBool venom_checkbox_is_checked(const VenomCheckbox* cb);
void venom_checkbox_set_label(VenomCheckbox* cb, const char* label);
void venom_checkbox_set_enabled(VenomCheckbox* cb, VenomBool enabled);
void venom_checkbox_set_on_change(VenomCheckbox* cb, VenomCheckboxCallback callback, void* data);

extern const VenomWidgetClass venom_checkbox_class;

/* Convenience macro */
#define venom_checkbox(...) _venom_checkbox_build(&(VenomCheckboxConfig){ __VA_ARGS__ })

typedef struct VenomCheckboxConfig {
    const char* label;
    VenomBool checked;
    VenomCheckboxCallback on_change;
    void* data;
} VenomCheckboxConfig;

VenomWidget* _venom_checkbox_build(const VenomCheckboxConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VENOM_CHECKBOX_H */
