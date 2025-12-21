/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_table.c - Data Table Widget implementation
 */

#include "venom/widgets/venom_table.h"
#include "venom/core/venom_memory.h"
#include "venom/graphics/venom_canvas.h"
#include <string.h>
#include <stdio.h>

/* Dimensions */
#define TABLE_HEADER_HEIGHT 48.0f
#define TABLE_DEFAULT_ROW_HEIGHT 44.0f
#define TABLE_CELL_PADDING 12.0f
#define TABLE_BORDER_WIDTH 1.0f

/* ============================================================================
 * FORWARD DECLARATIONS
 * ============================================================================ */

static void table_init(VenomWidget* widget);
static void table_destroy(VenomWidget* widget);
static void table_measure(VenomWidget* widget, VenomF32 aw, VenomF32 ah,
                           VenomF32* ow, VenomF32* oh);
static void table_layout(VenomWidget* widget, VenomRectF bounds);
static void table_draw(VenomWidget* widget, VenomCanvas* canvas);
static VenomBool table_on_event(VenomWidget* widget, const VenomEvent* event);

/* ============================================================================
 * CLASS DEFINITION
 * ============================================================================ */

const VenomWidgetClass venom_table_class = {
    .class_name = "VenomTable",
    .instance_size = sizeof(VenomTable),
    .parent_class = &venom_widget_class,
    .init = table_init,
    .destroy = table_destroy,
    .measure = table_measure,
    .layout = table_layout,
    .draw = table_draw,
    .on_event = table_on_event,
    .on_state_changed = NULL,
};

/* ============================================================================
 * HELPERS
 * ============================================================================ */

static void free_row(VenomTableRow* row) {
    if (row->values) {
        for (VenomU32 i = 0; i < row->value_count; i++) {
            if (row->values[i]) {
                venom_free(row->values[i], strlen(row->values[i]) + 1);
            }
        }
        venom_free(row->values, row->value_count * sizeof(char*));
        row->values = NULL;
    }
    row->value_count = 0;
}

/* ============================================================================
 * LIFECYCLE
 * ============================================================================ */

static void table_init(VenomWidget* widget) {
    VenomTable* table = (VenomTable*)widget;
    
    table->columns = NULL;
    table->column_count = 0;
    table->rows = NULL;
    table->row_count = 0;
    table->row_capacity = 0;
    table->selectable = VENOM_TRUE;
    table->multi_select = VENOM_FALSE;
    table->hovered_row = -1;
    table->sort_column = -1;
    table->sort_direction = VENOM_SORT_NONE;
    table->scroll_y = 0;
    table->row_height = TABLE_DEFAULT_ROW_HEIGHT;
    
    /* Colors */
    table->header_bg = venom_color_rgb(248, 249, 250);
    table->header_text = venom_color_rgb(50, 50, 60);
    table->row_bg = VENOM_COLOR_WHITE;
    table->alt_row_bg = venom_color_rgb(250, 250, 252);
    table->selected_bg = venom_color_rgb(232, 240, 254);
    table->hover_bg = venom_color_rgb(245, 245, 248);
    table->border_color = venom_color_rgb(222, 226, 230);
    table->striped = VENOM_TRUE;
    table->bordered = VENOM_TRUE;
    
    table->on_row_click = NULL;
    table->on_sort = NULL;
    table->user_data = NULL;
}

static void table_destroy(VenomWidget* widget) {
    VenomTable* table = (VenomTable*)widget;
    
    /* Free columns */
    if (table->columns) {
        venom_free(table->columns, table->column_count * sizeof(VenomTableColumn));
        table->columns = NULL;
    }
    table->column_count = 0;
    
    /* Free rows */
    if (table->rows) {
        for (VenomU32 i = 0; i < table->row_count; i++) {
            free_row(&table->rows[i]);
        }
        venom_free(table->rows, table->row_capacity * sizeof(VenomTableRow));
        table->rows = NULL;
    }
    table->row_count = 0;
    table->row_capacity = 0;
    
    venom_widget_class.destroy(widget);
}

/* ============================================================================
 * LAYOUT
 * ============================================================================ */

static void table_measure(VenomWidget* widget, VenomF32 aw, VenomF32 ah,
                           VenomF32* ow, VenomF32* oh) {
    VenomTable* table = (VenomTable*)widget;
    
    *ow = aw;
    VenomF32 content_height = TABLE_HEADER_HEIGHT + table->row_count * table->row_height;
    *oh = ah > 0 ? ah : content_height;
}

static void table_layout(VenomWidget* widget, VenomRectF bounds) {
    widget->bounds = bounds;
}

/* ============================================================================
 * DRAWING
 * ============================================================================ */

static void table_draw(VenomWidget* widget, VenomCanvas* canvas) {
    VenomTable* table = (VenomTable*)widget;
    VenomRectF bounds = { 0, 0, widget->bounds.width, widget->bounds.height };
    
    if (table->column_count == 0) return;
    
    /* Calculate column widths */
    VenomF32 total_flex = 0;
    VenomF32 fixed_width = 0;
    
    for (VenomU32 i = 0; i < table->column_count; i++) {
        if (table->columns[i].width > 0) {
            fixed_width += table->columns[i].width;
        } else {
            total_flex += table->columns[i].flex > 0 ? table->columns[i].flex : 1;
        }
    }
    
    VenomF32 flex_space = bounds.width - fixed_width;
    VenomF32* col_widths = venom_alloc(table->column_count * sizeof(VenomF32));
    if (!col_widths) return;
    
    for (VenomU32 i = 0; i < table->column_count; i++) {
        if (table->columns[i].width > 0) {
            col_widths[i] = table->columns[i].width;
        } else {
            VenomF32 flex = table->columns[i].flex > 0 ? table->columns[i].flex : 1;
            col_widths[i] = flex_space * (flex / total_flex);
        }
    }
    
    /* Draw header */
    VenomRectF header_rect = {bounds.x, bounds.y, bounds.width, TABLE_HEADER_HEIGHT};
    VenomPaint hbg = venom_paint_fill(table->header_bg);
    venom_canvas_draw_rect(canvas, header_rect, &hbg);
    
    /* Draw header cells */
    VenomF32 x = bounds.x;
    for (VenomU32 i = 0; i < table->column_count; i++) {
        VenomF32 text_x = x + TABLE_CELL_PADDING;
        VenomF32 text_y = bounds.y + TABLE_HEADER_HEIGHT / 2 + 5;
        
        VenomPaint tp = venom_paint_fill(table->header_text);
        if (table->columns[i].title) {
            venom_canvas_draw_text(canvas, table->columns[i].title, text_x, text_y, NULL, &tp);
        }
        
        /* Sort indicator */
        if ((VenomI32)i == table->sort_column && table->sort_direction != VENOM_SORT_NONE) {
            const char* arrow = table->sort_direction == VENOM_SORT_ASC ? "▲" : "▼";
            venom_canvas_draw_text(canvas, arrow, x + col_widths[i] - 20, text_y, NULL, &tp);
        }
        
        /* Border */
        if (table->bordered && i < table->column_count - 1) {
            VenomRectF border = {x + col_widths[i] - TABLE_BORDER_WIDTH, bounds.y, 
                                 TABLE_BORDER_WIDTH, TABLE_HEADER_HEIGHT};
            VenomPaint bp = venom_paint_fill(table->border_color);
            venom_canvas_draw_rect(canvas, border, &bp);
        }
        
        x += col_widths[i];
    }
    
    /* Header bottom border */
    if (table->bordered) {
        VenomRectF hborder = {bounds.x, bounds.y + TABLE_HEADER_HEIGHT - TABLE_BORDER_WIDTH,
                              bounds.width, TABLE_BORDER_WIDTH};
        VenomPaint hbp = venom_paint_fill(table->border_color);
        venom_canvas_draw_rect(canvas, hborder, &hbp);
    }
    
    /* Draw rows */
    VenomF32 y = bounds.y + TABLE_HEADER_HEIGHT - table->scroll_y;
    
    for (VenomU32 row_idx = 0; row_idx < table->row_count; row_idx++) {
        if (y + table->row_height < bounds.y + TABLE_HEADER_HEIGHT) {
            y += table->row_height;
            continue;  /* Above visible */
        }
        if (y > bounds.y + bounds.height) {
            break;  /* Below visible */
        }
        
        VenomTableRow* row = &table->rows[row_idx];
        
        /* Row background */
        VenomColor row_color;
        if (row->selected) {
            row_color = table->selected_bg;
        } else if ((VenomI32)row_idx == table->hovered_row) {
            row_color = table->hover_bg;
        } else if (table->striped && row_idx % 2 == 1) {
            row_color = table->alt_row_bg;
        } else {
            row_color = table->row_bg;
        }
        
        VenomRectF row_rect = {bounds.x, y, bounds.width, table->row_height};
        VenomPaint rbg = venom_paint_fill(row_color);
        venom_canvas_draw_rect(canvas, row_rect, &rbg);
        
        /* Draw cells */
        x = bounds.x;
        for (VenomU32 col_idx = 0; col_idx < table->column_count && col_idx < row->value_count; col_idx++) {
            VenomF32 text_x = x + TABLE_CELL_PADDING;
            VenomF32 text_y = y + table->row_height / 2 + 5;
            
            VenomPaint cp = venom_paint_fill(venom_color_rgb(60, 60, 70));
            if (row->values[col_idx]) {
                venom_canvas_draw_text(canvas, row->values[col_idx], text_x, text_y, NULL, &cp);
            }
            
            /* Border */
            if (table->bordered && col_idx < table->column_count - 1) {
                VenomRectF border = {x + col_widths[col_idx] - TABLE_BORDER_WIDTH, y,
                                     TABLE_BORDER_WIDTH, table->row_height};
                VenomPaint bp = venom_paint_fill(table->border_color);
                venom_canvas_draw_rect(canvas, border, &bp);
            }
            
            x += col_widths[col_idx];
        }
        
        /* Row bottom border */
        if (table->bordered) {
            VenomRectF rborder = {bounds.x, y + table->row_height - TABLE_BORDER_WIDTH,
                                  bounds.width, TABLE_BORDER_WIDTH};
            VenomPaint rbp = venom_paint_fill(table->border_color);
            venom_canvas_draw_rect(canvas, rborder, &rbp);
        }
        
        y += table->row_height;
    }
    
    venom_free(col_widths, table->column_count * sizeof(VenomF32));
}

/* ============================================================================
 * EVENTS
 * ============================================================================ */

static VenomBool table_on_event(VenomWidget* widget, const VenomEvent* event) {
    VenomTable* table = (VenomTable*)widget;
    VenomRectF bounds = widget->bounds;
    
    if (event->type == VENOM_EVENT_MOUSE_MOVE) {
        VenomF32 my = event->mouse.y;
        
        if (my > bounds.y + TABLE_HEADER_HEIGHT) {
            VenomI32 row_idx = (VenomI32)((my - bounds.y - TABLE_HEADER_HEIGHT + table->scroll_y) / table->row_height);
            if (row_idx >= 0 && row_idx < (VenomI32)table->row_count) {
                if (table->hovered_row != row_idx) {
                    table->hovered_row = row_idx;
                    venom_widget_invalidate(widget);
                }
            }
        } else {
            if (table->hovered_row != -1) {
                table->hovered_row = -1;
                venom_widget_invalidate(widget);
            }
        }
        return VENOM_TRUE;
    }
    
    if (event->type == VENOM_EVENT_MOUSE_BUTTON_DOWN) {
        VenomF32 mx = event->mouse.x;
        VenomF32 my = event->mouse.y;
        
        /* Header click (sorting) */
        if (my >= bounds.y && my < bounds.y + TABLE_HEADER_HEIGHT) {
            VenomF32 x = bounds.x;
            for (VenomU32 i = 0; i < table->column_count; i++) {
                VenomF32 w = table->columns[i].width > 0 ? 
                             table->columns[i].width : 100;  /* Simplified */
                
                if (mx >= x && mx < x + w && table->columns[i].sortable) {
                    if ((VenomI32)i == table->sort_column) {
                        table->sort_direction = table->sort_direction == VENOM_SORT_ASC ?
                                                VENOM_SORT_DESC : VENOM_SORT_ASC;
                    } else {
                        table->sort_column = i;
                        table->sort_direction = VENOM_SORT_ASC;
                    }
                    
                    if (table->on_sort) {
                        table->on_sort(table, i, table->sort_direction, table->user_data);
                    }
                    
                    venom_widget_invalidate(widget);
                    return VENOM_TRUE;
                }
                x += w;
            }
        }
        
        /* Row click */
        if (my > bounds.y + TABLE_HEADER_HEIGHT && table->selectable) {
            VenomI32 row_idx = (VenomI32)((my - bounds.y - TABLE_HEADER_HEIGHT + table->scroll_y) / table->row_height);
            
            if (row_idx >= 0 && row_idx < (VenomI32)table->row_count) {
                if (!table->multi_select) {
                    /* Deselect all first */
                    for (VenomU32 i = 0; i < table->row_count; i++) {
                        table->rows[i].selected = VENOM_FALSE;
                    }
                }
                
                table->rows[row_idx].selected = !table->rows[row_idx].selected;
                
                if (table->on_row_click) {
                    table->on_row_click(table, row_idx, table->user_data);
                }
                
                venom_widget_invalidate(widget);
                return VENOM_TRUE;
            }
        }
    }
    
    return VENOM_FALSE;
}

/* ============================================================================
 * PUBLIC API
 * ============================================================================ */

VenomResultPtr venom_table_create(const VenomTableColumn* columns, VenomU32 column_count) {
    if (!columns || column_count == 0) {
        return VENOM_ERR_PTR(VENOM_ERROR_NULL_POINTER);
    }
    
    VenomResultPtr result = venom_widget_create(&venom_table_class);
    if (!result.ok) return result;
    
    VenomTable* table = (VenomTable*)result.value;
    
    /* Copy columns */
    table->columns = venom_alloc(column_count * sizeof(VenomTableColumn));
    if (!table->columns) {
        venom_unref(table);
        return VENOM_ERR_PTR(VENOM_ERROR_OUT_OF_MEMORY);
    }
    
    memcpy(table->columns, columns, column_count * sizeof(VenomTableColumn));
    table->column_count = column_count;
    
    return result;
}

void venom_table_add_row(VenomTable* table, const char** values, VenomU32 value_count) {
    if (!table || !values) return;
    
    /* Grow array if needed */
    if (table->row_count >= table->row_capacity) {
        VenomU32 new_cap = table->row_capacity ? table->row_capacity * 2 : 16;
        VenomTableRow* new_rows = venom_realloc(table->rows,
            table->row_capacity * sizeof(VenomTableRow),
            new_cap * sizeof(VenomTableRow));
        
        if (!new_rows) return;
        
        table->rows = new_rows;
        table->row_capacity = new_cap;
    }
    
    VenomTableRow* row = &table->rows[table->row_count];
    row->values = venom_alloc(value_count * sizeof(char*));
    if (!row->values) return;
    
    row->value_count = value_count;
    row->selected = VENOM_FALSE;
    row->user_data = NULL;
    
    for (VenomU32 i = 0; i < value_count; i++) {
        if (values[i]) {
            VenomSize len = strlen(values[i]) + 1;
            row->values[i] = venom_alloc(len);
            if (row->values[i]) {
                memcpy(row->values[i], values[i], len);
            }
        } else {
            row->values[i] = NULL;
        }
    }
    
    table->row_count++;
    venom_widget_invalidate((VenomWidget*)table);
}

void venom_table_set_row(VenomTable* table, VenomU32 index, const char** values, VenomU32 value_count) {
    if (!table || index >= table->row_count) return;
    
    VenomTableRow* row = &table->rows[index];
    free_row(row);
    
    row->values = venom_alloc(value_count * sizeof(char*));
    if (!row->values) return;
    
    row->value_count = value_count;
    
    for (VenomU32 i = 0; i < value_count; i++) {
        if (values[i]) {
            VenomSize len = strlen(values[i]) + 1;
            row->values[i] = venom_alloc(len);
            if (row->values[i]) {
                memcpy(row->values[i], values[i], len);
            }
        } else {
            row->values[i] = NULL;
        }
    }
    
    venom_widget_invalidate((VenomWidget*)table);
}

void venom_table_remove_row(VenomTable* table, VenomU32 index) {
    if (!table || index >= table->row_count) return;
    
    free_row(&table->rows[index]);
    
    /* Shift remaining rows */
    for (VenomU32 i = index; i < table->row_count - 1; i++) {
        table->rows[i] = table->rows[i + 1];
    }
    table->row_count--;
    
    venom_widget_invalidate((VenomWidget*)table);
}

void venom_table_clear(VenomTable* table) {
    if (!table) return;
    
    for (VenomU32 i = 0; i < table->row_count; i++) {
        free_row(&table->rows[i]);
    }
    table->row_count = 0;
    
    venom_widget_invalidate((VenomWidget*)table);
}

VenomU32 venom_table_get_row_count(VenomTable* table) {
    return table ? table->row_count : 0;
}

void venom_table_select_row(VenomTable* table, VenomU32 index, VenomBool selected) {
    if (!table || index >= table->row_count) return;
    table->rows[index].selected = selected;
    venom_widget_invalidate((VenomWidget*)table);
}

void venom_table_select_all(VenomTable* table, VenomBool selected) {
    if (!table) return;
    for (VenomU32 i = 0; i < table->row_count; i++) {
        table->rows[i].selected = selected;
    }
    venom_widget_invalidate((VenomWidget*)table);
}

VenomBool venom_table_is_selected(VenomTable* table, VenomU32 index) {
    return table && index < table->row_count ? table->rows[index].selected : VENOM_FALSE;
}

void venom_table_set_selectable(VenomTable* table, VenomBool selectable, VenomBool multi) {
    if (!table) return;
    table->selectable = selectable;
    table->multi_select = multi;
}

void venom_table_set_striped(VenomTable* table, VenomBool striped) {
    if (!table) return;
    table->striped = striped;
    venom_widget_invalidate((VenomWidget*)table);
}

void venom_table_set_bordered(VenomTable* table, VenomBool bordered) {
    if (!table) return;
    table->bordered = bordered;
    venom_widget_invalidate((VenomWidget*)table);
}

void venom_table_set_row_height(VenomTable* table, VenomF32 height) {
    if (!table) return;
    table->row_height = height > 0 ? height : TABLE_DEFAULT_ROW_HEIGHT;
    venom_widget_invalidate_layout((VenomWidget*)table);
}

void venom_table_set_colors(VenomTable* table,
                             VenomColor header_bg,
                             VenomColor row_bg,
                             VenomColor alt_row_bg,
                             VenomColor selected_bg) {
    if (!table) return;
    table->header_bg = header_bg;
    table->row_bg = row_bg;
    table->alt_row_bg = alt_row_bg;
    table->selected_bg = selected_bg;
    venom_widget_invalidate((VenomWidget*)table);
}

void venom_table_set_callbacks(VenomTable* table,
                                void (*on_row_click)(VenomTable*, VenomU32, void*),
                                void (*on_sort)(VenomTable*, VenomU32, VenomSortDirection, void*),
                                void* user_data) {
    if (!table) return;
    table->on_row_click = on_row_click;
    table->on_sort = on_sort;
    table->user_data = user_data;
}
