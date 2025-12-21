/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_chart.h - Chart/Graph Widget
 */

#ifndef VENOM_CHART_H
#define VENOM_CHART_H

#include "venom_widget.h"
#include "../core/venom_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * TYPES
 * ============================================================================ */

/**
 * @brief Chart types
 */
typedef enum {
    VENOM_CHART_LINE,
    VENOM_CHART_BAR,
    VENOM_CHART_PIE,
    VENOM_CHART_DONUT,
    VENOM_CHART_AREA
} VenomChartType;

/**
 * @brief Data point
 */
typedef struct VenomDataPoint {
    VenomF32 value;
    const char* label;
} VenomDataPoint;

/**
 * @brief Dataset
 */
typedef struct VenomDataset {
    const char* label;
    VenomF32* values;
    VenomU32 value_count;
    VenomColor color;
    VenomColor fill_color;
    VenomF32 line_width;
    VenomBool show_points;
} VenomDataset;

/**
 * @brief Chart structure
 */
typedef struct VenomChart {
    VenomWidget base;
    
    /* Type */
    VenomChartType type;
    
    /* Data */
    const char** labels;
    VenomU32 label_count;
    VenomDataset* datasets;
    VenomU32 dataset_count;
    
    /* Appearance */
    VenomColor background;
    VenomColor grid_color;
    VenomColor text_color;
    VenomBool show_grid;
    VenomBool show_legend;
    VenomBool show_labels;
    VenomBool show_values;
    VenomBool animate;
    
    /* Animation state */
    VenomF32 anim_progress;
    
    /* Padding */
    VenomF32 padding;
    
    /* Title */
    const char* title;
    
    /* Range */
    VenomF32 min_value;
    VenomF32 max_value;
    VenomBool auto_scale;
} VenomChart;

/* ============================================================================
 * CREATION
 * ============================================================================ */

/**
 * @brief Create a chart
 */
VENOM_WARN_UNUSED
VenomResultPtr venom_chart_create(VenomChartType type);

/* ============================================================================
 * DATA
 * ============================================================================ */

/**
 * @brief Set labels (X-axis)
 */
void venom_chart_set_labels(VenomChart* chart, const char** labels, VenomU32 count);

/**
 * @brief Add dataset
 */
void venom_chart_add_dataset(VenomChart* chart, const VenomDataset* dataset);

/**
 * @brief Clear all data
 */
void venom_chart_clear(VenomChart* chart);

/**
 * @brief Update dataset values
 */
void venom_chart_update_dataset(VenomChart* chart, VenomU32 index, 
                                 const VenomF32* values, VenomU32 count);

/* ============================================================================
 * CONFIGURATION
 * ============================================================================ */

/**
 * @brief Set chart type
 */
void venom_chart_set_type(VenomChart* chart, VenomChartType type);

/**
 * @brief Set title
 */
void venom_chart_set_title(VenomChart* chart, const char* title);

/**
 * @brief Show/hide grid
 */
void venom_chart_set_show_grid(VenomChart* chart, VenomBool show);

/**
 * @brief Show/hide legend
 */
void venom_chart_set_show_legend(VenomChart* chart, VenomBool show);

/**
 * @brief Enable/disable animation
 */
void venom_chart_set_animate(VenomChart* chart, VenomBool animate);

/**
 * @brief Set value range
 */
void venom_chart_set_range(VenomChart* chart, VenomF32 min_val, VenomF32 max_val);

/**
 * @brief Enable auto-scaling
 */
void venom_chart_set_auto_scale(VenomChart* chart, VenomBool auto_scale);

/**
 * @brief Refresh animation
 */
void venom_chart_refresh(VenomChart* chart);

/* ============================================================================
 * CLASS
 * ============================================================================ */

extern const VenomWidgetClass venom_chart_class;

#ifdef __cplusplus
}
#endif

#endif /* VENOM_CHART_H */
