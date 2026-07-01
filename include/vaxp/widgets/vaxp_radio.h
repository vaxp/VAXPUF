/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_radio.h - Radio button widget (mutually exclusive selection)
 */

#ifndef VAXP_RADIO_H
#define VAXP_RADIO_H

#include "vaxp/widgets/vaxp_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VaxpRadioGroup VaxpRadioGroup;
typedef struct VaxpRadioButton VaxpRadioButton;

typedef void (*VaxpRadioCallback)(VaxpRadioButton* radio, void* user_data);

/* ============================================================================
 * RADIO GROUP - Manages mutual exclusion
 * ============================================================================ */

struct VaxpRadioGroup {
    VaxpRadioButton** buttons;
    VaxpU32 count;
    VaxpU32 capacity;
    VaxpRadioButton* selected;
};

VaxpRadioGroup* vaxp_radio_group_create(void);
void vaxp_radio_group_destroy(VaxpRadioGroup* group);
void vaxp_radio_group_add(VaxpRadioGroup* group, VaxpRadioButton* radio);
VaxpRadioButton* vaxp_radio_group_get_selected(const VaxpRadioGroup* group);

/* ============================================================================
 * RADIO BUTTON
 * ============================================================================ */

struct VaxpRadioButton {
    VaxpWidget base;
    
    VaxpBool selected;
    VaxpBool enabled;
    char* label;
    VaxpRadioGroup* group;
    
    /* Value for identification */
    int value;
    
    /* Styling */
    VaxpF32 circle_size;
    VaxpF32 spacing;
    VaxpColor selected_color;
    VaxpColor unselected_color;
    VaxpColor label_color;
    
    /* Callback */
    VaxpRadioCallback on_select;
    void* callback_data;
};

VaxpResultPtr vaxp_radio_create(void);
void vaxp_radio_set_selected(VaxpRadioButton* radio, VaxpBool selected);
VaxpBool vaxp_radio_is_selected(const VaxpRadioButton* radio);
void vaxp_radio_set_label(VaxpRadioButton* radio, const char* label);
void vaxp_radio_set_group(VaxpRadioButton* radio, VaxpRadioGroup* group);
void vaxp_radio_set_value(VaxpRadioButton* radio, int value);
int vaxp_radio_get_value(const VaxpRadioButton* radio);
void vaxp_radio_set_on_select(VaxpRadioButton* radio, VaxpRadioCallback callback, void* data);

extern const VaxpWidgetClass vaxp_radio_class;

#define vaxp_radio(...) _vaxp_radio_build(&(VaxpRadioConfig){ __VA_ARGS__ })

typedef struct VaxpRadioConfig {
    const char* label;
    int value;
    VaxpBool selected;
    VaxpRadioGroup* group;
    VaxpRadioCallback on_select;
    void* data;
} VaxpRadioConfig;

VaxpWidget* _vaxp_radio_build(const VaxpRadioConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VAXP_RADIO_H */
