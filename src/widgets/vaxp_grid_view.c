/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_grid_view.c - Grid layout implementation
 */

#include "vaxp/widgets/vaxp_grid_view.h"
#include "vaxp/core/vaxp_memory.h"
#include <string.h>

#define DEFAULT_COLUMNS 3
#define DEFAULT_SPACING 8.0f
#define DEFAULT_ITEM_HEIGHT 100.0f
#define INITIAL_CAPACITY 16

static void grid_view_init(VaxpWidget* widget) {
    VaxpGridView* grid = (VaxpGridView*)widget;
    
    grid->columns = DEFAULT_COLUMNS;
    grid->column_spacing = DEFAULT_SPACING;
    grid->row_spacing = DEFAULT_SPACING;
    
    grid->items = NULL;
    grid->item_count = 0;
    grid->item_capacity = 0;
    
    grid->builder = NULL;
    grid->builder_data = NULL;
    
    grid->scroll_offset = 0;
    grid->content_height = 0;
    
    grid->item_width = 0;
    grid->item_height = DEFAULT_ITEM_HEIGHT;
    grid->row_height = DEFAULT_ITEM_HEIGHT + DEFAULT_SPACING;
}

static void grid_view_destroy(VaxpWidget* widget) {
    VaxpGridView* grid = (VaxpGridView*)widget;
    
    if (grid->items) {
        vaxp_free(grid->items, grid->item_capacity * sizeof(void*));
        grid->items = NULL;
    }
    
    vaxp_widget_class.destroy(widget);
}

static void grid_view_measure(VaxpWidget* widget, VaxpF32 available_width, VaxpF32 available_height,
                              VaxpF32* out_width, VaxpF32* out_height) {
    VaxpGridView* grid = (VaxpGridView*)widget;
    
    *out_width = available_width;
    
    VaxpU32 rows = (grid->item_count + grid->columns - 1) / grid->columns;
    grid->content_height = rows * grid->row_height;
    
    *out_height = widget->layout.preferred_height > 0 ?
                  widget->layout.preferred_height :
                  (grid->content_height < available_height ? grid->content_height : available_height);
}

static void grid_view_draw(VaxpWidget* widget, VaxpCanvas* canvas) {
    VaxpGridView* grid = (VaxpGridView*)widget;
    
    VaxpF32 w = widget->bounds.width;
    VaxpF32 h = widget->bounds.height;
    
    /* Calculate item width */
    VaxpF32 item_w = grid->item_width > 0 ? grid->item_width :
                      (w - (grid->columns - 1) * grid->column_spacing) / grid->columns;
    
    /* Clip to bounds */
    vaxp_canvas_save(canvas);
    VaxpRectF clip = { 0, 0, w, h };
    vaxp_canvas_clip_rect(canvas, clip);
    
    /* Calculate visible row range */
    VaxpU32 first_row = (VaxpU32)(grid->scroll_offset / grid->row_height);
    VaxpU32 visible_rows = (VaxpU32)(h / grid->row_height) + 2;
    
    for (VaxpU32 row = first_row; row < first_row + visible_rows; row++) {
        for (VaxpU32 col = 0; col < grid->columns; col++) {
            VaxpU32 index = row * grid->columns + col;
            if (index >= grid->item_count) break;
            
            VaxpF32 x = col * (item_w + grid->column_spacing);
            VaxpF32 y = row * grid->row_height - grid->scroll_offset;
            
            if (grid->builder && index < grid->item_count) {
                VaxpWidget* item_widget = grid->builder(index, grid->items[index], grid->builder_data);
                if (item_widget) {
                    VaxpRectF item_bounds = { x, y, item_w, grid->item_height };
                    vaxp_widget_layout(item_widget, item_bounds);
                    
                    vaxp_canvas_save(canvas);
                    vaxp_canvas_translate(canvas, x, y);
                    vaxp_widget_draw(item_widget, canvas);
                    vaxp_canvas_restore(canvas);
                    
                    vaxp_unref(item_widget);
                }
            }
        }
    }
    
    vaxp_canvas_restore(canvas);
    
    /* Draw scrollbar if needed */
    if (grid->content_height > h) {
        VaxpF32 sb_width = 6.0f;
        VaxpF32 ratio = h / grid->content_height;
        VaxpF32 thumb_h = h * ratio;
        if (thumb_h < 30) thumb_h = 30;
        VaxpF32 thumb_y = (grid->scroll_offset / grid->content_height) * h;
        
        VaxpPaint sb_paint = vaxp_paint_fill((VaxpColor){ 150, 150, 150, 150 });
        VaxpRectF sb_rect = { w - sb_width - 2, thumb_y, sb_width, thumb_h };
        vaxp_canvas_draw_rounded_rect(canvas, sb_rect, sb_width / 2, &sb_paint);
    }
}

static VaxpBool grid_view_on_event(VaxpWidget* widget, const VaxpEvent* event) {
    VaxpGridView* grid = (VaxpGridView*)widget;
    
    if (event->type == VAXP_EVENT_MOUSE_SCROLL) {
        grid->scroll_offset -= event->scroll.y * grid->row_height / 2;
        
        if (grid->scroll_offset < 0) grid->scroll_offset = 0;
        VaxpF32 max_scroll = grid->content_height - widget->bounds.height;
        if (max_scroll < 0) max_scroll = 0;
        if (grid->scroll_offset > max_scroll) grid->scroll_offset = max_scroll;
        
        widget->needs_redraw = VAXP_TRUE;
        return VAXP_TRUE;
    }
    
    return VAXP_FALSE;
}

const VaxpWidgetClass vaxp_grid_view_class = {
    .class_name = "VaxpGridView",
    .instance_size = sizeof(VaxpGridView),
    .parent_class = &vaxp_widget_class,
    .init = grid_view_init,
    .destroy = grid_view_destroy,
    .measure = grid_view_measure,
    .layout = NULL,
    .draw = grid_view_draw,
    .on_event = grid_view_on_event,
    .on_state_changed = NULL,
};

VaxpResultPtr vaxp_grid_view_create(void) {
    return vaxp_widget_create(&vaxp_grid_view_class);
}

void vaxp_grid_view_set_columns(VaxpGridView* grid, VaxpU32 columns) {
    if (grid && columns > 0) {
        grid->columns = columns;
        vaxp_widget_invalidate((VaxpWidget*)grid);
    }
}

void vaxp_grid_view_set_spacing(VaxpGridView* grid, VaxpF32 column, VaxpF32 row) {
    if (grid) {
        grid->column_spacing = column;
        grid->row_spacing = row;
        grid->row_height = grid->item_height + row;
        vaxp_widget_invalidate((VaxpWidget*)grid);
    }
}

void vaxp_grid_view_set_builder(VaxpGridView* grid, VaxpGridItemBuilder builder, void* data) {
    if (grid) {
        grid->builder = builder;
        grid->builder_data = data;
    }
}

VaxpResult vaxp_grid_view_add_item(VaxpGridView* grid, void* item) {
    VAXP_ENSURE_NOT_NULL(grid);
    
    if (grid->item_count >= grid->item_capacity) {
        VaxpU32 new_cap = grid->item_capacity == 0 ? INITIAL_CAPACITY : grid->item_capacity * 2;
        void** new_items = (void**)vaxp_alloc(new_cap * sizeof(void*));
        if (!new_items) return VAXP_ERR_UNIT(VAXP_ERROR_OUT_OF_MEMORY);
        
        if (grid->items) {
            memcpy(new_items, grid->items, grid->item_count * sizeof(void*));
            vaxp_free(grid->items, grid->item_capacity * sizeof(void*));
        }
        
        grid->items = new_items;
        grid->item_capacity = new_cap;
    }
    
    grid->items[grid->item_count++] = item;
    vaxp_widget_invalidate((VaxpWidget*)grid);
    return VAXP_OK_UNIT();
}

void vaxp_grid_view_clear(VaxpGridView* grid) {
    if (grid) {
        grid->item_count = 0;
        grid->scroll_offset = 0;
        vaxp_widget_invalidate((VaxpWidget*)grid);
    }
}

void vaxp_grid_view_set_item_size(VaxpGridView* grid, VaxpF32 width, VaxpF32 height) {
    if (grid) {
        grid->item_width = width;
        grid->item_height = height;
        grid->row_height = height + grid->row_spacing;
        vaxp_widget_invalidate((VaxpWidget*)grid);
    }
}

VaxpWidget* _vaxp_grid_view_build(const VaxpGridViewConfig* config) {
    VaxpResultPtr result = vaxp_grid_view_create();
    if (!result.ok) return NULL;
    
    VaxpGridView* grid = (VaxpGridView*)result.value;
    
    if (config->columns > 0) grid->columns = config->columns;
    if (config->spacing > 0) {
        grid->column_spacing = config->spacing;
        grid->row_spacing = config->spacing;
        grid->row_height = grid->item_height + config->spacing;
    }
    grid->builder = config->builder;
    grid->builder_data = config->builder_data;
    
    return (VaxpWidget*)grid;
}
