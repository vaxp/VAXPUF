/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_search_bar.h - Search input with icon
 */

#ifndef VAXP_SEARCH_BAR_H
#define VAXP_SEARCH_BAR_H

#include "vaxp/widgets/vaxp_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VaxpSearchBar VaxpSearchBar;
typedef void (*VaxpSearchCallback)(VaxpSearchBar* bar, const char* query, void* data);

struct VaxpSearchBar {
    VaxpWidget base;
    
    char* text;
    VaxpSize text_len;
    VaxpSize text_capacity;
    
    char* placeholder;
    VaxpU32 cursor_pos;
    VaxpBool show_clear;
    
    VaxpSearchCallback on_search;
    VaxpSearchCallback on_change;
    void* callback_data;
    
    VaxpColor background_color;
    VaxpColor border_color;
    VaxpColor text_color;
    VaxpColor placeholder_color;
    VaxpColor icon_color;
    VaxpF32 corner_radius;
    VaxpF32 height;
};

VaxpResultPtr vaxp_search_bar_create(void);
void vaxp_search_bar_set_text(VaxpSearchBar* bar, const char* text);
const char* vaxp_search_bar_get_text(const VaxpSearchBar* bar);
void vaxp_search_bar_set_placeholder(VaxpSearchBar* bar, const char* ph);
void vaxp_search_bar_set_on_search(VaxpSearchBar* bar, VaxpSearchCallback callback, void* data);
void vaxp_search_bar_set_on_change(VaxpSearchBar* bar, VaxpSearchCallback callback, void* data);
void vaxp_search_bar_clear(VaxpSearchBar* bar);

extern const VaxpWidgetClass vaxp_search_bar_class;

#define vaxp_search_bar(...) _vaxp_search_bar_build(&(VaxpSearchBarConfig){ __VA_ARGS__ })

typedef struct VaxpSearchBarConfig {
    const char* placeholder;
    VaxpSearchCallback on_search;
    void* data;
} VaxpSearchBarConfig;

VaxpWidget* _vaxp_search_bar_build(const VaxpSearchBarConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VAXP_SEARCH_BAR_H */
