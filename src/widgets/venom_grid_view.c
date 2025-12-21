/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_grid_view.c - Grid layout implementation
 */

#include "venom/widgets/venom_grid_view.h"
#include "venom/core/venom_memory.h"
#include <string.h>

#define DEFAULT_COLUMNS 3
#define DEFAULT_SPACING 8.0f
#define DEFAULT_ITEM_HEIGHT 100.0f
#define INITIAL_CAPACITY 16

static void grid_view_init(VenomWidget* widget) {
    VenomGridView* grid = (VenomGridView*)widget;
    
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

static void grid_view_destroy(VenomWidget* widget) {
    VenomGridView* grid = (VenomGridView*)widget;
    
    if (grid->items) {
        venom_free(grid->items, grid->item_capacity * sizeof(void*));
        grid->items = NULL;
    }
    
    venom_widget_class.destroy(widget);
}

static void grid_view_measure(VenomWidget* widget, VenomF32 available_width, VenomF32 available_height,
                              VenomF32* out_width, VenomF32* out_height) {
    VenomGridView* grid = (VenomGridView*)widget;
    
    *out_width = available_width;
    
    VenomU32 rows = (grid->item_count + grid->columns - 1) / grid->columns;
    grid->content_height = rows * grid->row_height;
    
    *out_height = widget->layout.preferred_height > 0 ?
                  widget->layout.preferred_height :
                  (grid->content_height < available_height ? grid->content_height : available_height);
}

static void grid_view_draw(VenomWidget* widget, VenomCanvas* canvas) {
    VenomGridView* grid = (VenomGridView*)widget;
    
    VenomF32 w = widget->bounds.width;
    VenomF32 h = widget->bounds.height;
    
    /* Calculate item width */
    VenomF32 item_w = grid->item_width > 0 ? grid->item_width :
                      (w - (grid->columns - 1) * grid->column_spacing) / grid->columns;
    
    /* Clip to bounds */
    venom_canvas_save(canvas);
    VenomRectF clip = { 0, 0, w, h };
    venom_canvas_clip_rect(canvas, clip);
    
    /* Calculate visible row range */
    VenomU32 first_row = (VenomU32)(grid->scroll_offset / grid->row_height);
    VenomU32 visible_rows = (VenomU32)(h / grid->row_height) + 2;
    
    for (VenomU32 row = first_row; row < first_row + visible_rows; row++) {
        for (VenomU32 col = 0; col < grid->columns; col++) {
            VenomU32 index = row * grid->columns + col;
            if (index >= grid->item_count) break;
            
            VenomF32 x = col * (item_w + grid->column_spacing);
            VenomF32 y = row * grid->row_height - grid->scroll_offset;
            
            if (grid->builder && index < grid->item_count) {
                VenomWidget* item_widget = grid->builder(index, grid->items[index], grid->builder_data);
                if (item_widget) {
                    VenomRectF item_bounds = { x, y, item_w, grid->item_height };
                    venom_widget_layout(item_widget, item_bounds);
                    
                    venom_canvas_save(canvas);
                    venom_canvas_translate(canvas, x, y);
                    venom_widget_draw(item_widget, canvas);
                    venom_canvas_restore(canvas);
                    
                    venom_unref(item_widget);
                }
            }
        }
    }
    
    venom_canvas_restore(canvas);
    
    /* Draw scrollbar if needed */
    if (grid->content_height > h) {
        VenomF32 sb_width = 6.0f;
        VenomF32 ratio = h / grid->content_height;
        VenomF32 thumb_h = h * ratio;
        if (thumb_h < 30) thumb_h = 30;
        VenomF32 thumb_y = (grid->scroll_offset / grid->content_height) * h;
        
        VenomPaint sb_paint = venom_paint_fill((VenomColor){ 150, 150, 150, 150 });
        VenomRectF sb_rect = { w - sb_width - 2, thumb_y, sb_width, thumb_h };
        venom_canvas_draw_rounded_rect(canvas, sb_rect, sb_width / 2, &sb_paint);
    }
}

static VenomBool grid_view_on_event(VenomWidget* widget, const VenomEvent* event) {
    VenomGridView* grid = (VenomGridView*)widget;
    
    if (event->type == VENOM_EVENT_MOUSE_SCROLL) {
        grid->scroll_offset -= event->scroll.y * grid->row_height / 2;
        
        if (grid->scroll_offset < 0) grid->scroll_offset = 0;
        VenomF32 max_scroll = grid->content_height - widget->bounds.height;
        if (max_scroll < 0) max_scroll = 0;
        if (grid->scroll_offset > max_scroll) grid->scroll_offset = max_scroll;
        
        widget->needs_redraw = VENOM_TRUE;
        return VENOM_TRUE;
    }
    
    return VENOM_FALSE;
}

const VenomWidgetClass venom_grid_view_class = {
    .class_name = "VenomGridView",
    .instance_size = sizeof(VenomGridView),
    .parent_class = &venom_widget_class,
    .init = grid_view_init,
    .destroy = grid_view_destroy,
    .measure = grid_view_measure,
    .layout = NULL,
    .draw = grid_view_draw,
    .on_event = grid_view_on_event,
    .on_state_changed = NULL,
};

VenomResultPtr venom_grid_view_create(void) {
    return venom_widget_create(&venom_grid_view_class);
}

void venom_grid_view_set_columns(VenomGridView* grid, VenomU32 columns) {
    if (grid && columns > 0) {
        grid->columns = columns;
        venom_widget_invalidate((VenomWidget*)grid);
    }
}

void venom_grid_view_set_spacing(VenomGridView* grid, VenomF32 column, VenomF32 row) {
    if (grid) {
        grid->column_spacing = column;
        grid->row_spacing = row;
        grid->row_height = grid->item_height + row;
        venom_widget_invalidate((VenomWidget*)grid);
    }
}

void venom_grid_view_set_builder(VenomGridView* grid, VenomGridItemBuilder builder, void* data) {
    if (grid) {
        grid->builder = builder;
        grid->builder_data = data;
    }
}

VenomResult venom_grid_view_add_item(VenomGridView* grid, void* item) {
    VENOM_ENSURE_NOT_NULL(grid);
    
    if (grid->item_count >= grid->item_capacity) {
        VenomU32 new_cap = grid->item_capacity == 0 ? INITIAL_CAPACITY : grid->item_capacity * 2;
        void** new_items = (void**)venom_alloc(new_cap * sizeof(void*));
        if (!new_items) return VENOM_ERR_UNIT(VENOM_ERROR_OUT_OF_MEMORY);
        
        if (grid->items) {
            memcpy(new_items, grid->items, grid->item_count * sizeof(void*));
            venom_free(grid->items, grid->item_capacity * sizeof(void*));
        }
        
        grid->items = new_items;
        grid->item_capacity = new_cap;
    }
    
    grid->items[grid->item_count++] = item;
    venom_widget_invalidate((VenomWidget*)grid);
    return VENOM_OK_UNIT();
}

void venom_grid_view_clear(VenomGridView* grid) {
    if (grid) {
        grid->item_count = 0;
        grid->scroll_offset = 0;
        venom_widget_invalidate((VenomWidget*)grid);
    }
}

void venom_grid_view_set_item_size(VenomGridView* grid, VenomF32 width, VenomF32 height) {
    if (grid) {
        grid->item_width = width;
        grid->item_height = height;
        grid->row_height = height + grid->row_spacing;
        venom_widget_invalidate((VenomWidget*)grid);
    }
}

VenomWidget* _venom_grid_view_build(const VenomGridViewConfig* config) {
    VenomResultPtr result = venom_grid_view_create();
    if (!result.ok) return NULL;
    
    VenomGridView* grid = (VenomGridView*)result.value;
    
    if (config->columns > 0) grid->columns = config->columns;
    if (config->spacing > 0) {
        grid->column_spacing = config->spacing;
        grid->row_spacing = config->spacing;
        grid->row_height = grid->item_height + config->spacing;
    }
    grid->builder = config->builder;
    grid->builder_data = config->builder_data;
    
    return (VenomWidget*)grid;
}
