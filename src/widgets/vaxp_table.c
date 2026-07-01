/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_table.c - Data Table Widget implementation
 */

#include "vaxp/widgets/vaxp_table.h"
#include "vaxp/core/vaxp_memory.h"
#include "vaxp/graphics/vaxp_canvas.h"
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

static void table_init(VaxpWidget* widget);
static void table_destroy(VaxpWidget* widget);
static void table_measure(VaxpWidget* widget, VaxpF32 aw, VaxpF32 ah,
                           VaxpF32* ow, VaxpF32* oh);
static void table_layout(VaxpWidget* widget, VaxpRectF bounds);
static void table_draw(VaxpWidget* widget, VaxpCanvas* canvas);
static VaxpBool table_on_event(VaxpWidget* widget, const VaxpEvent* event);

/* ============================================================================
 * CLASS DEFINITION
 * ============================================================================ */

const VaxpWidgetClass vaxp_table_class = {
    .class_name = "VaxpTable",
    .instance_size = sizeof(VaxpTable),
    .parent_class = &vaxp_widget_class,
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

static void free_row(VaxpTableRow* row) {
    if (row->values) {
        for (VaxpU32 i = 0; i < row->value_count; i++) {
            if (row->values[i]) {
                vaxp_free(row->values[i], strlen(row->values[i]) + 1);
            }
        }
        vaxp_free(row->values, row->value_count * sizeof(char*));
        row->values = NULL;
    }
    row->value_count = 0;
}

/* ============================================================================
 * LIFECYCLE
 * ============================================================================ */

static void table_init(VaxpWidget* widget) {
    VaxpTable* table = (VaxpTable*)widget;
    
    table->columns = NULL;
    table->column_count = 0;
    table->rows = NULL;
    table->row_count = 0;
    table->row_capacity = 0;
    table->selectable = VAXP_TRUE;
    table->multi_select = VAXP_FALSE;
    table->hovered_row = -1;
    table->sort_column = -1;
    table->sort_direction = VAXP_SORT_NONE;
    table->scroll_y = 0;
    table->row_height = TABLE_DEFAULT_ROW_HEIGHT;
    
    /* Colors */
    table->header_bg = vaxp_color_rgb(248, 249, 250);
    table->header_text = vaxp_color_rgb(50, 50, 60);
    table->row_bg = VAXP_COLOR_WHITE;
    table->alt_row_bg = vaxp_color_rgb(250, 250, 252);
    table->selected_bg = vaxp_color_rgb(232, 240, 254);
    table->hover_bg = vaxp_color_rgb(245, 245, 248);
    table->border_color = vaxp_color_rgb(222, 226, 230);
    table->striped = VAXP_TRUE;
    table->bordered = VAXP_TRUE;
    
    table->on_row_click = NULL;
    table->on_sort = NULL;
    table->user_data = NULL;
}

static void table_destroy(VaxpWidget* widget) {
    VaxpTable* table = (VaxpTable*)widget;
    
    /* Free columns */
    if (table->columns) {
        vaxp_free(table->columns, table->column_count * sizeof(VaxpTableColumn));
        table->columns = NULL;
    }
    table->column_count = 0;
    
    /* Free rows */
    if (table->rows) {
        for (VaxpU32 i = 0; i < table->row_count; i++) {
            free_row(&table->rows[i]);
        }
        vaxp_free(table->rows, table->row_capacity * sizeof(VaxpTableRow));
        table->rows = NULL;
    }
    table->row_count = 0;
    table->row_capacity = 0;
    
    vaxp_widget_class.destroy(widget);
}

/* ============================================================================
 * LAYOUT
 * ============================================================================ */

static void table_measure(VaxpWidget* widget, VaxpF32 aw, VaxpF32 ah,
                           VaxpF32* ow, VaxpF32* oh) {
    VaxpTable* table = (VaxpTable*)widget;
    
    *ow = aw;
    VaxpF32 content_height = TABLE_HEADER_HEIGHT + table->row_count * table->row_height;
    *oh = ah > 0 ? ah : content_height;
}

static void table_layout(VaxpWidget* widget, VaxpRectF bounds) {
    widget->bounds = bounds;
}

/* ============================================================================
 * DRAWING
 * ============================================================================ */

static void table_draw(VaxpWidget* widget, VaxpCanvas* canvas) {
    VaxpTable* table = (VaxpTable*)widget;
    VaxpRectF bounds = { 0, 0, widget->bounds.width, widget->bounds.height };
    
    if (table->column_count == 0) return;
    
    /* Calculate column widths */
    VaxpF32 total_flex = 0;
    VaxpF32 fixed_width = 0;
    
    for (VaxpU32 i = 0; i < table->column_count; i++) {
        if (table->columns[i].width > 0) {
            fixed_width += table->columns[i].width;
        } else {
            total_flex += table->columns[i].flex > 0 ? table->columns[i].flex : 1;
        }
    }
    
    VaxpF32 flex_space = bounds.width - fixed_width;
    VaxpF32* col_widths = vaxp_alloc(table->column_count * sizeof(VaxpF32));
    if (!col_widths) return;
    
    for (VaxpU32 i = 0; i < table->column_count; i++) {
        if (table->columns[i].width > 0) {
            col_widths[i] = table->columns[i].width;
        } else {
            VaxpF32 flex = table->columns[i].flex > 0 ? table->columns[i].flex : 1;
            col_widths[i] = flex_space * (flex / total_flex);
        }
    }
    
    /* Draw header */
    VaxpRectF header_rect = {bounds.x, bounds.y, bounds.width, TABLE_HEADER_HEIGHT};
    VaxpPaint hbg = vaxp_paint_fill(table->header_bg);
    vaxp_canvas_draw_rect(canvas, header_rect, &hbg);
    
    /* Draw header cells */
    VaxpF32 x = bounds.x;
    for (VaxpU32 i = 0; i < table->column_count; i++) {
        VaxpF32 text_x = x + TABLE_CELL_PADDING;
        VaxpF32 text_y = bounds.y + TABLE_HEADER_HEIGHT / 2 + 5;
        
        VaxpPaint tp = vaxp_paint_fill(table->header_text);
        if (table->columns[i].title) {
            vaxp_canvas_draw_text(canvas, table->columns[i].title, text_x, text_y, NULL, &tp);
        }
        
        /* Sort indicator */
        if ((VaxpI32)i == table->sort_column && table->sort_direction != VAXP_SORT_NONE) {
            const char* arrow = table->sort_direction == VAXP_SORT_ASC ? "▲" : "▼";
            vaxp_canvas_draw_text(canvas, arrow, x + col_widths[i] - 20, text_y, NULL, &tp);
        }
        
        /* Border */
        if (table->bordered && i < table->column_count - 1) {
            VaxpRectF border = {x + col_widths[i] - TABLE_BORDER_WIDTH, bounds.y, 
                                 TABLE_BORDER_WIDTH, TABLE_HEADER_HEIGHT};
            VaxpPaint bp = vaxp_paint_fill(table->border_color);
            vaxp_canvas_draw_rect(canvas, border, &bp);
        }
        
        x += col_widths[i];
    }
    
    /* Header bottom border */
    if (table->bordered) {
        VaxpRectF hborder = {bounds.x, bounds.y + TABLE_HEADER_HEIGHT - TABLE_BORDER_WIDTH,
                              bounds.width, TABLE_BORDER_WIDTH};
        VaxpPaint hbp = vaxp_paint_fill(table->border_color);
        vaxp_canvas_draw_rect(canvas, hborder, &hbp);
    }
    
    /* Draw rows */
    VaxpF32 y = bounds.y + TABLE_HEADER_HEIGHT - table->scroll_y;
    
    for (VaxpU32 row_idx = 0; row_idx < table->row_count; row_idx++) {
        if (y + table->row_height < bounds.y + TABLE_HEADER_HEIGHT) {
            y += table->row_height;
            continue;  /* Above visible */
        }
        if (y > bounds.y + bounds.height) {
            break;  /* Below visible */
        }
        
        VaxpTableRow* row = &table->rows[row_idx];
        
        /* Row background */
        VaxpColor row_color;
        if (row->selected) {
            row_color = table->selected_bg;
        } else if ((VaxpI32)row_idx == table->hovered_row) {
            row_color = table->hover_bg;
        } else if (table->striped && row_idx % 2 == 1) {
            row_color = table->alt_row_bg;
        } else {
            row_color = table->row_bg;
        }
        
        VaxpRectF row_rect = {bounds.x, y, bounds.width, table->row_height};
        VaxpPaint rbg = vaxp_paint_fill(row_color);
        vaxp_canvas_draw_rect(canvas, row_rect, &rbg);
        
        /* Draw cells */
        x = bounds.x;
        for (VaxpU32 col_idx = 0; col_idx < table->column_count && col_idx < row->value_count; col_idx++) {
            VaxpF32 text_x = x + TABLE_CELL_PADDING;
            VaxpF32 text_y = y + table->row_height / 2 + 5;
            
            VaxpPaint cp = vaxp_paint_fill(vaxp_color_rgb(60, 60, 70));
            if (row->values[col_idx]) {
                vaxp_canvas_draw_text(canvas, row->values[col_idx], text_x, text_y, NULL, &cp);
            }
            
            /* Border */
            if (table->bordered && col_idx < table->column_count - 1) {
                VaxpRectF border = {x + col_widths[col_idx] - TABLE_BORDER_WIDTH, y,
                                     TABLE_BORDER_WIDTH, table->row_height};
                VaxpPaint bp = vaxp_paint_fill(table->border_color);
                vaxp_canvas_draw_rect(canvas, border, &bp);
            }
            
            x += col_widths[col_idx];
        }
        
        /* Row bottom border */
        if (table->bordered) {
            VaxpRectF rborder = {bounds.x, y + table->row_height - TABLE_BORDER_WIDTH,
                                  bounds.width, TABLE_BORDER_WIDTH};
            VaxpPaint rbp = vaxp_paint_fill(table->border_color);
            vaxp_canvas_draw_rect(canvas, rborder, &rbp);
        }
        
        y += table->row_height;
    }
    
    vaxp_free(col_widths, table->column_count * sizeof(VaxpF32));
}

/* ============================================================================
 * EVENTS
 * ============================================================================ */

static VaxpBool table_on_event(VaxpWidget* widget, const VaxpEvent* event) {
    VaxpTable* table = (VaxpTable*)widget;
    VaxpRectF bounds = widget->bounds;
    
    if (event->type == VAXP_EVENT_MOUSE_MOVE) {
        VaxpF32 my = event->mouse.y;
        
        if (my > bounds.y + TABLE_HEADER_HEIGHT) {
            VaxpI32 row_idx = (VaxpI32)((my - bounds.y - TABLE_HEADER_HEIGHT + table->scroll_y) / table->row_height);
            if (row_idx >= 0 && row_idx < (VaxpI32)table->row_count) {
                if (table->hovered_row != row_idx) {
                    table->hovered_row = row_idx;
                    vaxp_widget_invalidate(widget);
                }
            }
        } else {
            if (table->hovered_row != -1) {
                table->hovered_row = -1;
                vaxp_widget_invalidate(widget);
            }
        }
        return VAXP_TRUE;
    }
    
    if (event->type == VAXP_EVENT_MOUSE_BUTTON_DOWN) {
        VaxpF32 mx = event->mouse.x;
        VaxpF32 my = event->mouse.y;
        
        /* Header click (sorting) */
        if (my >= bounds.y && my < bounds.y + TABLE_HEADER_HEIGHT) {
            VaxpF32 x = bounds.x;
            for (VaxpU32 i = 0; i < table->column_count; i++) {
                VaxpF32 w = table->columns[i].width > 0 ? 
                             table->columns[i].width : 100;  /* Simplified */
                
                if (mx >= x && mx < x + w && table->columns[i].sortable) {
                    if ((VaxpI32)i == table->sort_column) {
                        table->sort_direction = table->sort_direction == VAXP_SORT_ASC ?
                                                VAXP_SORT_DESC : VAXP_SORT_ASC;
                    } else {
                        table->sort_column = i;
                        table->sort_direction = VAXP_SORT_ASC;
                    }
                    
                    if (table->on_sort) {
                        table->on_sort(table, i, table->sort_direction, table->user_data);
                    }
                    
                    vaxp_widget_invalidate(widget);
                    return VAXP_TRUE;
                }
                x += w;
            }
        }
        
        /* Row click */
        if (my > bounds.y + TABLE_HEADER_HEIGHT && table->selectable) {
            VaxpI32 row_idx = (VaxpI32)((my - bounds.y - TABLE_HEADER_HEIGHT + table->scroll_y) / table->row_height);
            
            if (row_idx >= 0 && row_idx < (VaxpI32)table->row_count) {
                if (!table->multi_select) {
                    /* Deselect all first */
                    for (VaxpU32 i = 0; i < table->row_count; i++) {
                        table->rows[i].selected = VAXP_FALSE;
                    }
                }
                
                table->rows[row_idx].selected = !table->rows[row_idx].selected;
                
                if (table->on_row_click) {
                    table->on_row_click(table, row_idx, table->user_data);
                }
                
                vaxp_widget_invalidate(widget);
                return VAXP_TRUE;
            }
        }
    }
    
    return VAXP_FALSE;
}

/* ============================================================================
 * PUBLIC API
 * ============================================================================ */

VaxpResultPtr vaxp_table_create(const VaxpTableColumn* columns, VaxpU32 column_count) {
    if (!columns || column_count == 0) {
        return VAXP_ERR_PTR(VAXP_ERROR_NULL_POINTER);
    }
    
    VaxpResultPtr result = vaxp_widget_create(&vaxp_table_class);
    if (!result.ok) return result;
    
    VaxpTable* table = (VaxpTable*)result.value;
    
    /* Copy columns */
    table->columns = vaxp_alloc(column_count * sizeof(VaxpTableColumn));
    if (!table->columns) {
        vaxp_unref(table);
        return VAXP_ERR_PTR(VAXP_ERROR_OUT_OF_MEMORY);
    }
    
    memcpy(table->columns, columns, column_count * sizeof(VaxpTableColumn));
    table->column_count = column_count;
    
    return result;
}

void vaxp_table_add_row(VaxpTable* table, const char** values, VaxpU32 value_count) {
    if (!table || !values) return;
    
    /* Grow array if needed */
    if (table->row_count >= table->row_capacity) {
        VaxpU32 new_cap = table->row_capacity ? table->row_capacity * 2 : 16;
        VaxpTableRow* new_rows = vaxp_realloc(table->rows,
            table->row_capacity * sizeof(VaxpTableRow),
            new_cap * sizeof(VaxpTableRow));
        
        if (!new_rows) return;
        
        table->rows = new_rows;
        table->row_capacity = new_cap;
    }
    
    VaxpTableRow* row = &table->rows[table->row_count];
    row->values = vaxp_alloc(value_count * sizeof(char*));
    if (!row->values) return;
    
    row->value_count = value_count;
    row->selected = VAXP_FALSE;
    row->user_data = NULL;
    
    for (VaxpU32 i = 0; i < value_count; i++) {
        if (values[i]) {
            VaxpSize len = strlen(values[i]) + 1;
            row->values[i] = vaxp_alloc(len);
            if (row->values[i]) {
                memcpy(row->values[i], values[i], len);
            }
        } else {
            row->values[i] = NULL;
        }
    }
    
    table->row_count++;
    vaxp_widget_invalidate((VaxpWidget*)table);
}

void vaxp_table_set_row(VaxpTable* table, VaxpU32 index, const char** values, VaxpU32 value_count) {
    if (!table || index >= table->row_count) return;
    
    VaxpTableRow* row = &table->rows[index];
    free_row(row);
    
    row->values = vaxp_alloc(value_count * sizeof(char*));
    if (!row->values) return;
    
    row->value_count = value_count;
    
    for (VaxpU32 i = 0; i < value_count; i++) {
        if (values[i]) {
            VaxpSize len = strlen(values[i]) + 1;
            row->values[i] = vaxp_alloc(len);
            if (row->values[i]) {
                memcpy(row->values[i], values[i], len);
            }
        } else {
            row->values[i] = NULL;
        }
    }
    
    vaxp_widget_invalidate((VaxpWidget*)table);
}

void vaxp_table_remove_row(VaxpTable* table, VaxpU32 index) {
    if (!table || index >= table->row_count) return;
    
    free_row(&table->rows[index]);
    
    /* Shift remaining rows */
    for (VaxpU32 i = index; i < table->row_count - 1; i++) {
        table->rows[i] = table->rows[i + 1];
    }
    table->row_count--;
    
    vaxp_widget_invalidate((VaxpWidget*)table);
}

void vaxp_table_clear(VaxpTable* table) {
    if (!table) return;
    
    for (VaxpU32 i = 0; i < table->row_count; i++) {
        free_row(&table->rows[i]);
    }
    table->row_count = 0;
    
    vaxp_widget_invalidate((VaxpWidget*)table);
}

VaxpU32 vaxp_table_get_row_count(VaxpTable* table) {
    return table ? table->row_count : 0;
}

void vaxp_table_select_row(VaxpTable* table, VaxpU32 index, VaxpBool selected) {
    if (!table || index >= table->row_count) return;
    table->rows[index].selected = selected;
    vaxp_widget_invalidate((VaxpWidget*)table);
}

void vaxp_table_select_all(VaxpTable* table, VaxpBool selected) {
    if (!table) return;
    for (VaxpU32 i = 0; i < table->row_count; i++) {
        table->rows[i].selected = selected;
    }
    vaxp_widget_invalidate((VaxpWidget*)table);
}

VaxpBool vaxp_table_is_selected(VaxpTable* table, VaxpU32 index) {
    return table && index < table->row_count ? table->rows[index].selected : VAXP_FALSE;
}

void vaxp_table_set_selectable(VaxpTable* table, VaxpBool selectable, VaxpBool multi) {
    if (!table) return;
    table->selectable = selectable;
    table->multi_select = multi;
}

void vaxp_table_set_striped(VaxpTable* table, VaxpBool striped) {
    if (!table) return;
    table->striped = striped;
    vaxp_widget_invalidate((VaxpWidget*)table);
}

void vaxp_table_set_bordered(VaxpTable* table, VaxpBool bordered) {
    if (!table) return;
    table->bordered = bordered;
    vaxp_widget_invalidate((VaxpWidget*)table);
}

void vaxp_table_set_row_height(VaxpTable* table, VaxpF32 height) {
    if (!table) return;
    table->row_height = height > 0 ? height : TABLE_DEFAULT_ROW_HEIGHT;
    vaxp_widget_invalidate_layout((VaxpWidget*)table);
}

void vaxp_table_set_colors(VaxpTable* table,
                             VaxpColor header_bg,
                             VaxpColor row_bg,
                             VaxpColor alt_row_bg,
                             VaxpColor selected_bg) {
    if (!table) return;
    table->header_bg = header_bg;
    table->row_bg = row_bg;
    table->alt_row_bg = alt_row_bg;
    table->selected_bg = selected_bg;
    vaxp_widget_invalidate((VaxpWidget*)table);
}

void vaxp_table_set_callbacks(VaxpTable* table,
                                void (*on_row_click)(VaxpTable*, VaxpU32, void*),
                                void (*on_sort)(VaxpTable*, VaxpU32, VaxpSortDirection, void*),
                                void* user_data) {
    if (!table) return;
    table->on_row_click = on_row_click;
    table->on_sort = on_sort;
    table->user_data = user_data;
}
