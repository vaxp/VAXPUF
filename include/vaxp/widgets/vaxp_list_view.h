/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_list_view.h - ListView widget (scrollable list of items)
 */

#ifndef VAXP_LIST_VIEW_H
#define VAXP_LIST_VIEW_H

#include "vaxp/widgets/vaxp_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VaxpListView VaxpListView;

/* Item builder callback - creates widget for each item */
typedef VaxpWidget* (*VaxpListItemBuilder)(VaxpU32 index, void* item_data, void* user_data);

/* Item selection callback */
typedef void (*VaxpListSelectionCallback)(VaxpListView* list, VaxpU32 index, void* user_data);

struct VaxpListView {
    VaxpWidget base;
    
    /* Data source */
    void** items;                /* Array of item data pointers */
    VaxpU32 item_count;
    VaxpU32 item_capacity;
    
    /* Item building */
    VaxpListItemBuilder item_builder;
    void* builder_data;
    
    /* Cached item widgets (for virtualization) */
    VaxpWidget** item_widgets;
    VaxpU32 widget_count;
    
    /* Scrolling */
    VaxpF32 scroll_offset;
    VaxpF32 item_height;        /* Fixed item height (0 = variable) */
    VaxpF32 content_height;
    
    /* Selection */
    VaxpI32 selected_index;     /* -1 = no selection */
    VaxpBool multi_select;
    VaxpListSelectionCallback on_select;
    void* select_data;
    
    /* Styling */
    VaxpColor selection_color;
    VaxpColor hover_color;
    VaxpF32 item_padding;
    VaxpBool show_dividers;
    
    /* State */
    VaxpI32 hover_index;
    
};

VaxpResultPtr vaxp_list_view_create(void);
void vaxp_list_view_set_builder(VaxpListView* list, VaxpListItemBuilder builder, void* data);
VaxpResult vaxp_list_view_add_item(VaxpListView* list, void* item_data);
VaxpResult vaxp_list_view_remove_item(VaxpListView* list, VaxpU32 index);
void vaxp_list_view_clear(VaxpListView* list);
VaxpU32 vaxp_list_view_count(const VaxpListView* list);
void vaxp_list_view_set_selected(VaxpListView* list, VaxpI32 index);
VaxpI32 vaxp_list_view_get_selected(const VaxpListView* list);
void vaxp_list_view_set_item_height(VaxpListView* list, VaxpF32 height);
void vaxp_list_view_scroll_to(VaxpListView* list, VaxpU32 index);
void vaxp_list_view_set_on_select(VaxpListView* list, VaxpListSelectionCallback callback, void* data);

extern const VaxpWidgetClass vaxp_list_view_class;

#define vaxp_list_view(...) _vaxp_list_view_build(&(VaxpListViewConfig){ __VA_ARGS__ })

typedef struct VaxpListViewConfig {
    VaxpListItemBuilder item_builder;
    void* builder_data;
    VaxpF32 item_height;
    VaxpListSelectionCallback on_select;
    void* select_data;
} VaxpListViewConfig;

VaxpWidget* _vaxp_list_view_build(const VaxpListViewConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VAXP_LIST_VIEW_H */
