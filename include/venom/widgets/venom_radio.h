/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_radio.h - Radio button widget (mutually exclusive selection)
 */

#ifndef VENOM_RADIO_H
#define VENOM_RADIO_H

#include "venom/widgets/venom_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VenomRadioGroup VenomRadioGroup;
typedef struct VenomRadioButton VenomRadioButton;

typedef void (*VenomRadioCallback)(VenomRadioButton* radio, void* user_data);

/* ============================================================================
 * RADIO GROUP - Manages mutual exclusion
 * ============================================================================ */

struct VenomRadioGroup {
    VenomRadioButton** buttons;
    VenomU32 count;
    VenomU32 capacity;
    VenomRadioButton* selected;
};

VenomRadioGroup* venom_radio_group_create(void);
void venom_radio_group_destroy(VenomRadioGroup* group);
void venom_radio_group_add(VenomRadioGroup* group, VenomRadioButton* radio);
VenomRadioButton* venom_radio_group_get_selected(const VenomRadioGroup* group);

/* ============================================================================
 * RADIO BUTTON
 * ============================================================================ */

struct VenomRadioButton {
    VenomWidget base;
    
    VenomBool selected;
    VenomBool enabled;
    char* label;
    VenomRadioGroup* group;
    
    /* Value for identification */
    int value;
    
    /* Styling */
    VenomF32 circle_size;
    VenomF32 spacing;
    VenomColor selected_color;
    VenomColor unselected_color;
    VenomColor label_color;
    
    /* Callback */
    VenomRadioCallback on_select;
    void* callback_data;
};

VenomResultPtr venom_radio_create(void);
void venom_radio_set_selected(VenomRadioButton* radio, VenomBool selected);
VenomBool venom_radio_is_selected(const VenomRadioButton* radio);
void venom_radio_set_label(VenomRadioButton* radio, const char* label);
void venom_radio_set_group(VenomRadioButton* radio, VenomRadioGroup* group);
void venom_radio_set_value(VenomRadioButton* radio, int value);
int venom_radio_get_value(const VenomRadioButton* radio);
void venom_radio_set_on_select(VenomRadioButton* radio, VenomRadioCallback callback, void* data);

extern const VenomWidgetClass venom_radio_class;

#define venom_radio(...) _venom_radio_build(&(VenomRadioConfig){ __VA_ARGS__ })

typedef struct VenomRadioConfig {
    const char* label;
    int value;
    VenomBool selected;
    VenomRadioGroup* group;
    VenomRadioCallback on_select;
    void* data;
} VenomRadioConfig;

VenomWidget* _venom_radio_build(const VenomRadioConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VENOM_RADIO_H */
