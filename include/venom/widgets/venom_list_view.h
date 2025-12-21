/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_list_view.h - ListView widget (scrollable list of items)
 */

#ifndef VENOM_LIST_VIEW_H
#define VENOM_LIST_VIEW_H

#include "venom/widgets/venom_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VenomListView VenomListView;

/* Item builder callback - creates widget for each item */
typedef VenomWidget* (*VenomListItemBuilder)(VenomU32 index, void* item_data, void* user_data);

/* Item selection callback */
typedef void (*VenomListSelectionCallback)(VenomListView* list, VenomU32 index, void* user_data);

struct VenomListView {
    VenomWidget base;
    
    /* Data source */
    void** items;                /* Array of item data pointers */
    VenomU32 item_count;
    VenomU32 item_capacity;
    
    /* Item building */
    VenomListItemBuilder item_builder;
    void* builder_data;
    
    /* Cached item widgets (for virtualization) */
    VenomWidget** item_widgets;
    VenomU32 widget_count;
    
    /* Scrolling */
    VenomF32 scroll_offset;
    VenomF32 item_height;        /* Fixed item height (0 = variable) */
    VenomF32 content_height;
    
    /* Selection */
    VenomI32 selected_index;     /* -1 = no selection */
    VenomBool multi_select;
    VenomListSelectionCallback on_select;
    void* select_data;
    
    /* Styling */
    VenomColor selection_color;
    VenomColor hover_color;
    VenomF32 item_padding;
    VenomBool show_dividers;
    
    /* State */
    VenomI32 hover_index;
    
};

VenomResultPtr venom_list_view_create(void);
void venom_list_view_set_builder(VenomListView* list, VenomListItemBuilder builder, void* data);
VenomResult venom_list_view_add_item(VenomListView* list, void* item_data);
VenomResult venom_list_view_remove_item(VenomListView* list, VenomU32 index);
void venom_list_view_clear(VenomListView* list);
VenomU32 venom_list_view_count(const VenomListView* list);
void venom_list_view_set_selected(VenomListView* list, VenomI32 index);
VenomI32 venom_list_view_get_selected(const VenomListView* list);
void venom_list_view_set_item_height(VenomListView* list, VenomF32 height);
void venom_list_view_scroll_to(VenomListView* list, VenomU32 index);
void venom_list_view_set_on_select(VenomListView* list, VenomListSelectionCallback callback, void* data);

extern const VenomWidgetClass venom_list_view_class;

#define venom_list_view(...) _venom_list_view_build(&(VenomListViewConfig){ __VA_ARGS__ })

typedef struct VenomListViewConfig {
    VenomListItemBuilder item_builder;
    void* builder_data;
    VenomF32 item_height;
    VenomListSelectionCallback on_select;
    void* select_data;
} VenomListViewConfig;

VenomWidget* _venom_list_view_build(const VenomListViewConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VENOM_LIST_VIEW_H */
