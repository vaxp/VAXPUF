/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_tabs.h - TabBar and TabView widgets
 */

#ifndef VAXP_TABS_H
#define VAXP_TABS_H

#include "vaxp/widgets/vaxp_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VaxpTabBar VaxpTabBar;
typedef struct VaxpTabView VaxpTabView;
typedef struct VaxpTab VaxpTab;

typedef void (*VaxpTabCallback)(VaxpTabBar* bar, VaxpU32 index, void* user_data);

/* ============================================================================
 * TAB
 * ============================================================================ */

struct VaxpTab {
    char* label;
    char* icon;              /* Optional icon name/path */
    VaxpBool enabled;
    VaxpBool closable;
    VaxpWidget* content;    /* Content widget for TabView */
};

/* ============================================================================
 * TAB BAR
 * ============================================================================ */

typedef enum VaxpTabStyle {
    VAXP_TAB_STYLE_UNDERLINE,    /* Material style underline */
    VAXP_TAB_STYLE_PILL,         /* Pill/segment style */
    VAXP_TAB_STYLE_BOXED,        /* Boxed tabs */
} VaxpTabStyle;

struct VaxpTabBar {
    VaxpWidget base;
    
    VaxpTab* tabs;
    VaxpU32 tab_count;
    VaxpU32 tab_capacity;
    
    VaxpU32 selected_index;
    VaxpTabStyle style;
    VaxpBool scrollable;
    VaxpF32 scroll_offset;
    
    /* Styling */
    VaxpF32 tab_height;
    VaxpF32 tab_padding;
    VaxpF32 indicator_height;
    VaxpColor background_color;
    VaxpColor tab_color;
    VaxpColor selected_color;
    VaxpColor indicator_color;
    VaxpColor disabled_color;
    
    /* State */
    VaxpI32 hover_index;
    
    /* Callbacks */
    VaxpTabCallback on_change;
    void* callback_data;
    
    /* Linked TabView */
    VaxpTabView* tab_view;
};

VaxpResultPtr vaxp_tab_bar_create(void);
VaxpResult vaxp_tab_bar_add_tab(VaxpTabBar* bar, const char* label, VaxpWidget* content);
void vaxp_tab_bar_remove_tab(VaxpTabBar* bar, VaxpU32 index);
void vaxp_tab_bar_set_selected(VaxpTabBar* bar, VaxpU32 index);
VaxpU32 vaxp_tab_bar_get_selected(const VaxpTabBar* bar);
void vaxp_tab_bar_set_style(VaxpTabBar* bar, VaxpTabStyle style);
void vaxp_tab_bar_set_on_change(VaxpTabBar* bar, VaxpTabCallback callback, void* data);
void vaxp_tab_bar_link_view(VaxpTabBar* bar, VaxpTabView* view);

extern const VaxpWidgetClass vaxp_tab_bar_class;

/* ============================================================================
 * TAB VIEW
 * ============================================================================ */

struct VaxpTabView {
    VaxpWidget base;
    
    VaxpWidget** pages;
    VaxpU32 page_count;
    VaxpU32 page_capacity;
    
    VaxpU32 current_page;
    
    /* Linked TabBar */
    VaxpTabBar* tab_bar;
};

VaxpResultPtr vaxp_tab_view_create(void);
VaxpResult vaxp_tab_view_add_page(VaxpTabView* view, VaxpWidget* page);
void vaxp_tab_view_set_page(VaxpTabView* view, VaxpU32 index);
VaxpU32 vaxp_tab_view_get_page(const VaxpTabView* view);
void vaxp_tab_view_link_bar(VaxpTabView* view, VaxpTabBar* bar);

extern const VaxpWidgetClass vaxp_tab_view_class;

/* ============================================================================
 * CONVENIENCE MACROS
 * ============================================================================ */

#define vaxp_tab_bar(...) _vaxp_tab_bar_build(&(VaxpTabBarConfig){ __VA_ARGS__ })
#define vaxp_tab_view(...) _vaxp_tab_view_build(&(VaxpTabViewConfig){ __VA_ARGS__ })

typedef struct VaxpTabBarConfig {
    VaxpTabStyle style;
    VaxpU32 selected;
    VaxpTabCallback on_change;
    void* data;
} VaxpTabBarConfig;

typedef struct VaxpTabViewConfig {
    VaxpU32 current;
    VaxpTabBar* tab_bar;
} VaxpTabViewConfig;

VaxpWidget* _vaxp_tab_bar_build(const VaxpTabBarConfig* config);
VaxpWidget* _vaxp_tab_view_build(const VaxpTabViewConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VAXP_TABS_H */
