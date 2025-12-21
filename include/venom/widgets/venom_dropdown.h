/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_dropdown.h - Dropdown selection widget
 */

#ifndef VENOM_DROPDOWN_H
#define VENOM_DROPDOWN_H

#include "venom/widgets/venom_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VenomDropdown VenomDropdown;

typedef void (*VenomDropdownCallback)(VenomDropdown* dropdown, VenomU32 index, const char* value, void* user_data);

typedef struct VenomDropdownItem {
    char* label;
    char* value;
    VenomBool enabled;
} VenomDropdownItem;

struct VenomDropdown {
    VenomWidget base;
    
    VenomDropdownItem* items;
    VenomU32 item_count;
    VenomU32 item_capacity;
    
    VenomI32 selected_index;     /* -1 = none selected */
    char* placeholder;
    
    VenomBool is_open;
    VenomI32 hover_index;
    VenomF32 max_dropdown_height;
    VenomF32 scroll_offset;
    
    /* Styling */
    VenomF32 height;
    VenomF32 item_height;
    VenomF32 padding;
    VenomF32 corner_radius;
    VenomColor background_color;
    VenomColor border_color;
    VenomColor text_color;
    VenomColor placeholder_color;
    VenomColor dropdown_bg;
    VenomColor hover_color;
    VenomColor selected_bg;
    
    /* Callback */
    VenomDropdownCallback on_change;
    void* callback_data;
};

VenomResultPtr venom_dropdown_create(void);
VenomResult venom_dropdown_add_item(VenomDropdown* dd, const char* label, const char* value);
void venom_dropdown_clear(VenomDropdown* dd);
void venom_dropdown_set_selected(VenomDropdown* dd, VenomI32 index);
VenomI32 venom_dropdown_get_selected(const VenomDropdown* dd);
const char* venom_dropdown_get_selected_value(const VenomDropdown* dd);
const char* venom_dropdown_get_selected_label(const VenomDropdown* dd);
void venom_dropdown_set_placeholder(VenomDropdown* dd, const char* placeholder);
void venom_dropdown_set_on_change(VenomDropdown* dd, VenomDropdownCallback callback, void* data);

extern const VenomWidgetClass venom_dropdown_class;

#define venom_dropdown(...) _venom_dropdown_build(&(VenomDropdownConfig){ __VA_ARGS__ })

typedef struct VenomDropdownConfig {
    const char* placeholder;
    VenomI32 selected;
    VenomDropdownCallback on_change;
    void* data;
} VenomDropdownConfig;

VenomWidget* _venom_dropdown_build(const VenomDropdownConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VENOM_DROPDOWN_H */
