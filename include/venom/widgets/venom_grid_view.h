/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_grid_view.h - Grid layout widget
 */

#ifndef VENOM_GRID_VIEW_H
#define VENOM_GRID_VIEW_H

#include "venom/widgets/venom_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VenomGridView VenomGridView;

typedef VenomWidget* (*VenomGridItemBuilder)(VenomU32 index, void* data, void* user_data);

struct VenomGridView {
    VenomWidget base;
    
    VenomU32 columns;            /* Number of columns */
    VenomF32 column_spacing;
    VenomF32 row_spacing;
    
    /* Data */
    void** items;
    VenomU32 item_count;
    VenomU32 item_capacity;
    
    VenomGridItemBuilder builder;
    void* builder_data;
    
    /* Scrolling */
    VenomF32 scroll_offset;
    VenomF32 content_height;
    
    /* Item sizing */
    VenomF32 item_width;         /* 0 = auto calculate */
    VenomF32 item_height;
    VenomF32 row_height;
};

VenomResultPtr venom_grid_view_create(void);
void venom_grid_view_set_columns(VenomGridView* grid, VenomU32 columns);
void venom_grid_view_set_spacing(VenomGridView* grid, VenomF32 column, VenomF32 row);
void venom_grid_view_set_builder(VenomGridView* grid, VenomGridItemBuilder builder, void* data);
VenomResult venom_grid_view_add_item(VenomGridView* grid, void* item);
void venom_grid_view_clear(VenomGridView* grid);
void venom_grid_view_set_item_size(VenomGridView* grid, VenomF32 width, VenomF32 height);

extern const VenomWidgetClass venom_grid_view_class;

#define venom_grid_view(...) _venom_grid_view_build(&(VenomGridViewConfig){ __VA_ARGS__ })

typedef struct VenomGridViewConfig {
    VenomU32 columns;
    VenomF32 spacing;
    VenomGridItemBuilder builder;
    void* builder_data;
} VenomGridViewConfig;

VenomWidget* _venom_grid_view_build(const VenomGridViewConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VENOM_GRID_VIEW_H */
