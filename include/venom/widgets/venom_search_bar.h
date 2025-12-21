/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_search_bar.h - Search input with icon
 */

#ifndef VENOM_SEARCH_BAR_H
#define VENOM_SEARCH_BAR_H

#include "venom/widgets/venom_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VenomSearchBar VenomSearchBar;
typedef void (*VenomSearchCallback)(VenomSearchBar* bar, const char* query, void* data);

struct VenomSearchBar {
    VenomWidget base;
    
    char* text;
    VenomSize text_len;
    VenomSize text_capacity;
    
    char* placeholder;
    VenomU32 cursor_pos;
    VenomBool show_clear;
    
    VenomSearchCallback on_search;
    VenomSearchCallback on_change;
    void* callback_data;
    
    VenomColor background_color;
    VenomColor border_color;
    VenomColor text_color;
    VenomColor placeholder_color;
    VenomColor icon_color;
    VenomF32 corner_radius;
    VenomF32 height;
};

VenomResultPtr venom_search_bar_create(void);
void venom_search_bar_set_text(VenomSearchBar* bar, const char* text);
const char* venom_search_bar_get_text(const VenomSearchBar* bar);
void venom_search_bar_set_placeholder(VenomSearchBar* bar, const char* ph);
void venom_search_bar_set_on_search(VenomSearchBar* bar, VenomSearchCallback callback, void* data);
void venom_search_bar_set_on_change(VenomSearchBar* bar, VenomSearchCallback callback, void* data);
void venom_search_bar_clear(VenomSearchBar* bar);

extern const VenomWidgetClass venom_search_bar_class;

#define venom_search_bar(...) _venom_search_bar_build(&(VenomSearchBarConfig){ __VA_ARGS__ })

typedef struct VenomSearchBarConfig {
    const char* placeholder;
    VenomSearchCallback on_search;
    void* data;
} VenomSearchBarConfig;

VenomWidget* _venom_search_bar_build(const VenomSearchBarConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VENOM_SEARCH_BAR_H */
