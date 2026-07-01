/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_table.h - Data Table Widget
 */

#ifndef VAXP_TABLE_H
#define VAXP_TABLE_H

#include "vaxp_widget.h"
#include "../core/vaxp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * TYPES
 * ============================================================================ */

/**
 * @brief Table column definition
 */
typedef struct VaxpTableColumn {
    const char* key;            /* Data key */
    const char* title;          /* Display title */
    VaxpF32 width;             /* Fixed width (0 = flexible) */
    VaxpF32 flex;              /* Flex factor */
    VaxpBool sortable;         /* Is sortable */
    VaxpBool resizable;        /* Is resizable */
    
    /* Custom cell renderer */
    VaxpWidget* (*cell_builder)(const char* value, VaxpU32 row, void* user_data);
    void* user_data;
} VaxpTableColumn;

/**
 * @brief Sort direction
 */
typedef enum {
    VAXP_SORT_NONE,
    VAXP_SORT_ASC,
    VAXP_SORT_DESC
} VaxpSortDirection;

/**
 * @brief Table row data
 */
typedef struct VaxpTableRow {
    char** values;              /* Array of cell values */
    VaxpU32 value_count;
    VaxpBool selected;
    void* user_data;
} VaxpTableRow;

/**
 * @brief Table structure
 */
typedef struct VaxpTable {
    VaxpWidget base;
    
    /* Columns */
    VaxpTableColumn* columns;
    VaxpU32 column_count;
    
    /* Data */
    VaxpTableRow* rows;
    VaxpU32 row_count;
    VaxpU32 row_capacity;
    
    /* Selection */
    VaxpBool selectable;
    VaxpBool multi_select;
    VaxpI32 hovered_row;
    
    /* Sorting */
    VaxpI32 sort_column;
    VaxpSortDirection sort_direction;
    
    /* Scroll */
    VaxpF32 scroll_y;
    VaxpF32 row_height;
    
    /* Appearance */
    VaxpColor header_bg;
    VaxpColor header_text;
    VaxpColor row_bg;
    VaxpColor alt_row_bg;
    VaxpColor selected_bg;
    VaxpColor hover_bg;
    VaxpColor border_color;
    VaxpBool striped;
    VaxpBool bordered;
    
    /* Callbacks */
    void (*on_row_click)(struct VaxpTable* table, VaxpU32 row, void* user_data);
    void (*on_sort)(struct VaxpTable* table, VaxpU32 column, VaxpSortDirection dir, void* user_data);
    void* user_data;
} VaxpTable;

/* ============================================================================
 * CREATION
 * ============================================================================ */

/**
 * @brief Create a table
 */
VAXP_WARN_UNUSED
VaxpResultPtr vaxp_table_create(const VaxpTableColumn* columns, VaxpU32 column_count);

/* ============================================================================
 * DATA
 * ============================================================================ */

/**
 * @brief Add a row
 */
void vaxp_table_add_row(VaxpTable* table, const char** values, VaxpU32 value_count);

/**
 * @brief Set row data at index
 */
void vaxp_table_set_row(VaxpTable* table, VaxpU32 index, const char** values, VaxpU32 value_count);

/**
 * @brief Remove row at index
 */
void vaxp_table_remove_row(VaxpTable* table, VaxpU32 index);

/**
 * @brief Clear all rows
 */
void vaxp_table_clear(VaxpTable* table);

/**
 * @brief Get row count
 */
VaxpU32 vaxp_table_get_row_count(VaxpTable* table);

/* ============================================================================
 * SELECTION
 * ============================================================================ */

/**
 * @brief Select row
 */
void vaxp_table_select_row(VaxpTable* table, VaxpU32 index, VaxpBool selected);

/**
 * @brief Select all rows
 */
void vaxp_table_select_all(VaxpTable* table, VaxpBool selected);

/**
 * @brief Check if row is selected
 */
VaxpBool vaxp_table_is_selected(VaxpTable* table, VaxpU32 index);

/* ============================================================================
 * CONFIGURATION
 * ============================================================================ */

/**
 * @brief Set selectable
 */
void vaxp_table_set_selectable(VaxpTable* table, VaxpBool selectable, VaxpBool multi);

/**
 * @brief Set striped rows
 */
void vaxp_table_set_striped(VaxpTable* table, VaxpBool striped);

/**
 * @brief Set bordered
 */
void vaxp_table_set_bordered(VaxpTable* table, VaxpBool bordered);

/**
 * @brief Set row height
 */
void vaxp_table_set_row_height(VaxpTable* table, VaxpF32 height);

/**
 * @brief Set colors
 */
void vaxp_table_set_colors(VaxpTable* table,
                             VaxpColor header_bg,
                             VaxpColor row_bg,
                             VaxpColor alt_row_bg,
                             VaxpColor selected_bg);

/**
 * @brief Set callbacks
 */
void vaxp_table_set_callbacks(VaxpTable* table,
                                void (*on_row_click)(VaxpTable*, VaxpU32, void*),
                                void (*on_sort)(VaxpTable*, VaxpU32, VaxpSortDirection, void*),
                                void* user_data);

/* ============================================================================
 * CLASS
 * ============================================================================ */

extern const VaxpWidgetClass vaxp_table_class;

#ifdef __cplusplus
}
#endif

#endif /* VAXP_TABLE_H */
