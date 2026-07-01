/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_grid_view.h - Grid layout widget
 */

#ifndef VAXP_GRID_VIEW_H
#define VAXP_GRID_VIEW_H

#include "vaxp/widgets/vaxp_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VaxpGridView VaxpGridView;

typedef VaxpWidget* (*VaxpGridItemBuilder)(VaxpU32 index, void* data, void* user_data);

struct VaxpGridView {
    VaxpWidget base;
    
    VaxpU32 columns;            /* Number of columns */
    VaxpF32 column_spacing;
    VaxpF32 row_spacing;
    
    /* Data */
    void** items;
    VaxpU32 item_count;
    VaxpU32 item_capacity;
    
    VaxpGridItemBuilder builder;
    void* builder_data;
    
    /* Scrolling */
    VaxpF32 scroll_offset;
    VaxpF32 content_height;
    
    /* Item sizing */
    VaxpF32 item_width;         /* 0 = auto calculate */
    VaxpF32 item_height;
    VaxpF32 row_height;
};

VaxpResultPtr vaxp_grid_view_create(void);
void vaxp_grid_view_set_columns(VaxpGridView* grid, VaxpU32 columns);
void vaxp_grid_view_set_spacing(VaxpGridView* grid, VaxpF32 column, VaxpF32 row);
void vaxp_grid_view_set_builder(VaxpGridView* grid, VaxpGridItemBuilder builder, void* data);
VaxpResult vaxp_grid_view_add_item(VaxpGridView* grid, void* item);
void vaxp_grid_view_clear(VaxpGridView* grid);
void vaxp_grid_view_set_item_size(VaxpGridView* grid, VaxpF32 width, VaxpF32 height);

extern const VaxpWidgetClass vaxp_grid_view_class;

#define vaxp_grid_view(...) _vaxp_grid_view_build(&(VaxpGridViewConfig){ __VA_ARGS__ })

typedef struct VaxpGridViewConfig {
    VaxpU32 columns;
    VaxpF32 spacing;
    VaxpGridItemBuilder builder;
    void* builder_data;
} VaxpGridViewConfig;

VaxpWidget* _vaxp_grid_view_build(const VaxpGridViewConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VAXP_GRID_VIEW_H */
