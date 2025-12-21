/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_table.h - Data Table Widget
 */

#ifndef VENOM_TABLE_H
#define VENOM_TABLE_H

#include "venom_widget.h"
#include "../core/venom_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * TYPES
 * ============================================================================ */

/**
 * @brief Table column definition
 */
typedef struct VenomTableColumn {
    const char* key;            /* Data key */
    const char* title;          /* Display title */
    VenomF32 width;             /* Fixed width (0 = flexible) */
    VenomF32 flex;              /* Flex factor */
    VenomBool sortable;         /* Is sortable */
    VenomBool resizable;        /* Is resizable */
    
    /* Custom cell renderer */
    VenomWidget* (*cell_builder)(const char* value, VenomU32 row, void* user_data);
    void* user_data;
} VenomTableColumn;

/**
 * @brief Sort direction
 */
typedef enum {
    VENOM_SORT_NONE,
    VENOM_SORT_ASC,
    VENOM_SORT_DESC
} VenomSortDirection;

/**
 * @brief Table row data
 */
typedef struct VenomTableRow {
    char** values;              /* Array of cell values */
    VenomU32 value_count;
    VenomBool selected;
    void* user_data;
} VenomTableRow;

/**
 * @brief Table structure
 */
typedef struct VenomTable {
    VenomWidget base;
    
    /* Columns */
    VenomTableColumn* columns;
    VenomU32 column_count;
    
    /* Data */
    VenomTableRow* rows;
    VenomU32 row_count;
    VenomU32 row_capacity;
    
    /* Selection */
    VenomBool selectable;
    VenomBool multi_select;
    VenomI32 hovered_row;
    
    /* Sorting */
    VenomI32 sort_column;
    VenomSortDirection sort_direction;
    
    /* Scroll */
    VenomF32 scroll_y;
    VenomF32 row_height;
    
    /* Appearance */
    VenomColor header_bg;
    VenomColor header_text;
    VenomColor row_bg;
    VenomColor alt_row_bg;
    VenomColor selected_bg;
    VenomColor hover_bg;
    VenomColor border_color;
    VenomBool striped;
    VenomBool bordered;
    
    /* Callbacks */
    void (*on_row_click)(struct VenomTable* table, VenomU32 row, void* user_data);
    void (*on_sort)(struct VenomTable* table, VenomU32 column, VenomSortDirection dir, void* user_data);
    void* user_data;
} VenomTable;

/* ============================================================================
 * CREATION
 * ============================================================================ */

/**
 * @brief Create a table
 */
VENOM_WARN_UNUSED
VenomResultPtr venom_table_create(const VenomTableColumn* columns, VenomU32 column_count);

/* ============================================================================
 * DATA
 * ============================================================================ */

/**
 * @brief Add a row
 */
void venom_table_add_row(VenomTable* table, const char** values, VenomU32 value_count);

/**
 * @brief Set row data at index
 */
void venom_table_set_row(VenomTable* table, VenomU32 index, const char** values, VenomU32 value_count);

/**
 * @brief Remove row at index
 */
void venom_table_remove_row(VenomTable* table, VenomU32 index);

/**
 * @brief Clear all rows
 */
void venom_table_clear(VenomTable* table);

/**
 * @brief Get row count
 */
VenomU32 venom_table_get_row_count(VenomTable* table);

/* ============================================================================
 * SELECTION
 * ============================================================================ */

/**
 * @brief Select row
 */
void venom_table_select_row(VenomTable* table, VenomU32 index, VenomBool selected);

/**
 * @brief Select all rows
 */
void venom_table_select_all(VenomTable* table, VenomBool selected);

/**
 * @brief Check if row is selected
 */
VenomBool venom_table_is_selected(VenomTable* table, VenomU32 index);

/* ============================================================================
 * CONFIGURATION
 * ============================================================================ */

/**
 * @brief Set selectable
 */
void venom_table_set_selectable(VenomTable* table, VenomBool selectable, VenomBool multi);

/**
 * @brief Set striped rows
 */
void venom_table_set_striped(VenomTable* table, VenomBool striped);

/**
 * @brief Set bordered
 */
void venom_table_set_bordered(VenomTable* table, VenomBool bordered);

/**
 * @brief Set row height
 */
void venom_table_set_row_height(VenomTable* table, VenomF32 height);

/**
 * @brief Set colors
 */
void venom_table_set_colors(VenomTable* table,
                             VenomColor header_bg,
                             VenomColor row_bg,
                             VenomColor alt_row_bg,
                             VenomColor selected_bg);

/**
 * @brief Set callbacks
 */
void venom_table_set_callbacks(VenomTable* table,
                                void (*on_row_click)(VenomTable*, VenomU32, void*),
                                void (*on_sort)(VenomTable*, VenomU32, VenomSortDirection, void*),
                                void* user_data);

/* ============================================================================
 * CLASS
 * ============================================================================ */

extern const VenomWidgetClass venom_table_class;

#ifdef __cplusplus
}
#endif

#endif /* VENOM_TABLE_H */
