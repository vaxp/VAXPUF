/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_tabs.h - TabBar and TabView widgets
 */

#ifndef VENOM_TABS_H
#define VENOM_TABS_H

#include "venom/widgets/venom_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VenomTabBar VenomTabBar;
typedef struct VenomTabView VenomTabView;
typedef struct VenomTab VenomTab;

typedef void (*VenomTabCallback)(VenomTabBar* bar, VenomU32 index, void* user_data);

/* ============================================================================
 * TAB
 * ============================================================================ */

struct VenomTab {
    char* label;
    char* icon;              /* Optional icon name/path */
    VenomBool enabled;
    VenomBool closable;
    VenomWidget* content;    /* Content widget for TabView */
};

/* ============================================================================
 * TAB BAR
 * ============================================================================ */

typedef enum VenomTabStyle {
    VENOM_TAB_STYLE_UNDERLINE,    /* Material style underline */
    VENOM_TAB_STYLE_PILL,         /* Pill/segment style */
    VENOM_TAB_STYLE_BOXED,        /* Boxed tabs */
} VenomTabStyle;

struct VenomTabBar {
    VenomWidget base;
    
    VenomTab* tabs;
    VenomU32 tab_count;
    VenomU32 tab_capacity;
    
    VenomU32 selected_index;
    VenomTabStyle style;
    VenomBool scrollable;
    VenomF32 scroll_offset;
    
    /* Styling */
    VenomF32 tab_height;
    VenomF32 tab_padding;
    VenomF32 indicator_height;
    VenomColor background_color;
    VenomColor tab_color;
    VenomColor selected_color;
    VenomColor indicator_color;
    VenomColor disabled_color;
    
    /* State */
    VenomI32 hover_index;
    
    /* Callbacks */
    VenomTabCallback on_change;
    void* callback_data;
    
    /* Linked TabView */
    VenomTabView* tab_view;
};

VenomResultPtr venom_tab_bar_create(void);
VenomResult venom_tab_bar_add_tab(VenomTabBar* bar, const char* label, VenomWidget* content);
void venom_tab_bar_remove_tab(VenomTabBar* bar, VenomU32 index);
void venom_tab_bar_set_selected(VenomTabBar* bar, VenomU32 index);
VenomU32 venom_tab_bar_get_selected(const VenomTabBar* bar);
void venom_tab_bar_set_style(VenomTabBar* bar, VenomTabStyle style);
void venom_tab_bar_set_on_change(VenomTabBar* bar, VenomTabCallback callback, void* data);
void venom_tab_bar_link_view(VenomTabBar* bar, VenomTabView* view);

extern const VenomWidgetClass venom_tab_bar_class;

/* ============================================================================
 * TAB VIEW
 * ============================================================================ */

struct VenomTabView {
    VenomWidget base;
    
    VenomWidget** pages;
    VenomU32 page_count;
    VenomU32 page_capacity;
    
    VenomU32 current_page;
    
    /* Linked TabBar */
    VenomTabBar* tab_bar;
};

VenomResultPtr venom_tab_view_create(void);
VenomResult venom_tab_view_add_page(VenomTabView* view, VenomWidget* page);
void venom_tab_view_set_page(VenomTabView* view, VenomU32 index);
VenomU32 venom_tab_view_get_page(const VenomTabView* view);
void venom_tab_view_link_bar(VenomTabView* view, VenomTabBar* bar);

extern const VenomWidgetClass venom_tab_view_class;

/* ============================================================================
 * CONVENIENCE MACROS
 * ============================================================================ */

#define venom_tab_bar(...) _venom_tab_bar_build(&(VenomTabBarConfig){ __VA_ARGS__ })
#define venom_tab_view(...) _venom_tab_view_build(&(VenomTabViewConfig){ __VA_ARGS__ })

typedef struct VenomTabBarConfig {
    VenomTabStyle style;
    VenomU32 selected;
    VenomTabCallback on_change;
    void* data;
} VenomTabBarConfig;

typedef struct VenomTabViewConfig {
    VenomU32 current;
    VenomTabBar* tab_bar;
} VenomTabViewConfig;

VenomWidget* _venom_tab_bar_build(const VenomTabBarConfig* config);
VenomWidget* _venom_tab_view_build(const VenomTabViewConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VENOM_TABS_H */
