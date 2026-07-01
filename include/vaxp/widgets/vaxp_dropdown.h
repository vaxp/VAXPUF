/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_dropdown.h - Dropdown selection widget
 */

#ifndef VAXP_DROPDOWN_H
#define VAXP_DROPDOWN_H

#include "vaxp/widgets/vaxp_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VaxpDropdown VaxpDropdown;

typedef void (*VaxpDropdownCallback)(VaxpDropdown* dropdown, VaxpU32 index, const char* value, void* user_data);

typedef struct VaxpDropdownItem {
    char* label;
    char* value;
    VaxpBool enabled;
} VaxpDropdownItem;

struct VaxpDropdown {
    VaxpWidget base;
    
    VaxpDropdownItem* items;
    VaxpU32 item_count;
    VaxpU32 item_capacity;
    
    VaxpI32 selected_index;     /* -1 = none selected */
    char* placeholder;
    
    VaxpBool is_open;
    VaxpI32 hover_index;
    VaxpF32 max_dropdown_height;
    VaxpF32 scroll_offset;
    
    /* Styling */
    VaxpF32 height;
    VaxpF32 item_height;
    VaxpF32 padding;
    VaxpF32 corner_radius;
    VaxpColor background_color;
    VaxpColor border_color;
    VaxpColor text_color;
    VaxpColor placeholder_color;
    VaxpColor dropdown_bg;
    VaxpColor hover_color;
    VaxpColor selected_bg;
    
    /* Callback */
    VaxpDropdownCallback on_change;
    void* callback_data;
};

VaxpResultPtr vaxp_dropdown_create(void);
VaxpResult vaxp_dropdown_add_item(VaxpDropdown* dd, const char* label, const char* value);
void vaxp_dropdown_clear(VaxpDropdown* dd);
void vaxp_dropdown_set_selected(VaxpDropdown* dd, VaxpI32 index);
VaxpI32 vaxp_dropdown_get_selected(const VaxpDropdown* dd);
const char* vaxp_dropdown_get_selected_value(const VaxpDropdown* dd);
const char* vaxp_dropdown_get_selected_label(const VaxpDropdown* dd);
void vaxp_dropdown_set_placeholder(VaxpDropdown* dd, const char* placeholder);
void vaxp_dropdown_set_on_change(VaxpDropdown* dd, VaxpDropdownCallback callback, void* data);

extern const VaxpWidgetClass vaxp_dropdown_class;

#define vaxp_dropdown(...) _vaxp_dropdown_build(&(VaxpDropdownConfig){ __VA_ARGS__ })

typedef struct VaxpDropdownConfig {
    const char* placeholder;
    VaxpI32 selected;
    VaxpDropdownCallback on_change;
    void* data;
} VaxpDropdownConfig;

VaxpWidget* _vaxp_dropdown_build(const VaxpDropdownConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VAXP_DROPDOWN_H */
