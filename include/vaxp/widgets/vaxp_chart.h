/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_chart.h - Chart/Graph Widget
 */

#ifndef VAXP_CHART_H
#define VAXP_CHART_H

#include "vaxp_widget.h"
#include "../core/vaxp_types.h"

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
    VAXP_CHART_LINE,
    VAXP_CHART_BAR,
    VAXP_CHART_PIE,
    VAXP_CHART_DONUT,
    VAXP_CHART_AREA
} VaxpChartType;

/**
 * @brief Data point
 */
typedef struct VaxpDataPoint {
    VaxpF32 value;
    const char* label;
} VaxpDataPoint;

/**
 * @brief Dataset
 */
typedef struct VaxpDataset {
    const char* label;
    VaxpF32* values;
    VaxpU32 value_count;
    VaxpColor color;
    VaxpColor fill_color;
    VaxpF32 line_width;
    VaxpBool show_points;
} VaxpDataset;

/**
 * @brief Chart structure
 */
typedef struct VaxpChart {
    VaxpWidget base;
    
    /* Type */
    VaxpChartType type;
    
    /* Data */
    const char** labels;
    VaxpU32 label_count;
    VaxpDataset* datasets;
    VaxpU32 dataset_count;
    
    /* Appearance */
    VaxpColor background;
    VaxpColor grid_color;
    VaxpColor text_color;
    VaxpBool show_grid;
    VaxpBool show_legend;
    VaxpBool show_labels;
    VaxpBool show_values;
    VaxpBool animate;
    
    /* Animation state */
    VaxpF32 anim_progress;
    
    /* Padding */
    VaxpF32 padding;
    
    /* Title */
    const char* title;
    
    /* Range */
    VaxpF32 min_value;
    VaxpF32 max_value;
    VaxpBool auto_scale;
} VaxpChart;

/* ============================================================================
 * CREATION
 * ============================================================================ */

/**
 * @brief Create a chart
 */
VAXP_WARN_UNUSED
VaxpResultPtr vaxp_chart_create(VaxpChartType type);

/* ============================================================================
 * DATA
 * ============================================================================ */

/**
 * @brief Set labels (X-axis)
 */
void vaxp_chart_set_labels(VaxpChart* chart, const char** labels, VaxpU32 count);

/**
 * @brief Add dataset
 */
void vaxp_chart_add_dataset(VaxpChart* chart, const VaxpDataset* dataset);

/**
 * @brief Clear all data
 */
void vaxp_chart_clear(VaxpChart* chart);

/**
 * @brief Update dataset values
 */
void vaxp_chart_update_dataset(VaxpChart* chart, VaxpU32 index, 
                                 const VaxpF32* values, VaxpU32 count);

/* ============================================================================
 * CONFIGURATION
 * ============================================================================ */

/**
 * @brief Set chart type
 */
void vaxp_chart_set_type(VaxpChart* chart, VaxpChartType type);

/**
 * @brief Set title
 */
void vaxp_chart_set_title(VaxpChart* chart, const char* title);

/**
 * @brief Show/hide grid
 */
void vaxp_chart_set_show_grid(VaxpChart* chart, VaxpBool show);

/**
 * @brief Show/hide legend
 */
void vaxp_chart_set_show_legend(VaxpChart* chart, VaxpBool show);

/**
 * @brief Enable/disable animation
 */
void vaxp_chart_set_animate(VaxpChart* chart, VaxpBool animate);

/**
 * @brief Set value range
 */
void vaxp_chart_set_range(VaxpChart* chart, VaxpF32 min_val, VaxpF32 max_val);

/**
 * @brief Enable auto-scaling
 */
void vaxp_chart_set_auto_scale(VaxpChart* chart, VaxpBool auto_scale);

/**
 * @brief Refresh animation
 */
void vaxp_chart_refresh(VaxpChart* chart);

/* ============================================================================
 * CLASS
 * ============================================================================ */

extern const VaxpWidgetClass vaxp_chart_class;

#ifdef __cplusplus
}
#endif

#endif /* VAXP_CHART_H */
