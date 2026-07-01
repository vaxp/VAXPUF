/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_checkbox.h - Checkbox widget (boolean toggle)
 */

#ifndef VAXP_CHECKBOX_H
#define VAXP_CHECKBOX_H

#include "vaxp/widgets/vaxp_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VaxpCheckbox VaxpCheckbox;

typedef void (*VaxpCheckboxCallback)(VaxpCheckbox* checkbox, VaxpBool checked, void* user_data);

struct VaxpCheckbox {
    VaxpWidget base;
    
    VaxpBool checked;
    VaxpBool enabled;
    char* label;
    
    /* Styling */
    VaxpF32 box_size;
    VaxpF32 spacing;        /* Space between box and label */
    VaxpColor check_color;
    VaxpColor box_color;
    VaxpColor label_color;
    
    /* Callback */
    VaxpCheckboxCallback on_change;
    void* callback_data;
};

VaxpResultPtr vaxp_checkbox_create(void);
void vaxp_checkbox_set_checked(VaxpCheckbox* cb, VaxpBool checked);
VaxpBool vaxp_checkbox_is_checked(const VaxpCheckbox* cb);
void vaxp_checkbox_set_label(VaxpCheckbox* cb, const char* label);
void vaxp_checkbox_set_enabled(VaxpCheckbox* cb, VaxpBool enabled);
void vaxp_checkbox_set_on_change(VaxpCheckbox* cb, VaxpCheckboxCallback callback, void* data);

extern const VaxpWidgetClass vaxp_checkbox_class;

/* Convenience macro */
#define vaxp_checkbox(...) _vaxp_checkbox_build(&(VaxpCheckboxConfig){ __VA_ARGS__ })

typedef struct VaxpCheckboxConfig {
    const char* label;
    VaxpBool checked;
    VaxpCheckboxCallback on_change;
    void* data;
} VaxpCheckboxConfig;

VaxpWidget* _vaxp_checkbox_build(const VaxpCheckboxConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VAXP_CHECKBOX_H */
