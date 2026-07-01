/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_container.h - Container widget with layout capabilities
 */

#ifndef VAXP_CONTAINER_H
#define VAXP_CONTAINER_H

#include "vaxp_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * CONTAINER TYPES
 * ============================================================================ */

typedef enum VaxpJustify {
    VAXP_JUSTIFY_START,
    VAXP_JUSTIFY_CENTER,
    VAXP_JUSTIFY_END,
    VAXP_JUSTIFY_SPACE_BETWEEN,
    VAXP_JUSTIFY_SPACE_AROUND,
    VAXP_JUSTIFY_SPACE_EVENLY,
} VaxpJustify;

typedef struct VaxpContainer {
    VaxpWidget base;
    
    VaxpFlexDirection direction;
    VaxpJustify justify;
    VaxpAlign align_items;
    VaxpF32 gap;               /* Space between items */
    
    /* Optional background */
    VaxpColor bg_color;
    VaxpF32 corner_radius;
    VaxpBool has_background;
} VaxpContainer;

/* ============================================================================
 * CONTAINER API
 * ============================================================================ */

/**
 * @brief Create a container
 */
VAXP_WARN_UNUSED
VaxpResultPtr vaxp_container_create(void);

/**
 * @brief Create a row container (flex-direction: row)
 */
VAXP_WARN_UNUSED
VaxpResultPtr vaxp_container_create_row(void);

/**
 * @brief Create a column container (flex-direction: column)
 */
VAXP_WARN_UNUSED
VaxpResultPtr vaxp_container_create_column(void);

/**
 * @brief Set flex direction
 */
void vaxp_container_set_direction(VaxpContainer* container, VaxpFlexDirection direction);

/**
 * @brief Set justify content
 */
void vaxp_container_set_justify(VaxpContainer* container, VaxpJustify justify);

/**
 * @brief Set align items
 */
void vaxp_container_set_align(VaxpContainer* container, VaxpAlign align);

/**
 * @brief Set gap between items
 */
void vaxp_container_set_gap(VaxpContainer* container, VaxpF32 gap);

/**
 * @brief Set background color
 */
void vaxp_container_set_background(VaxpContainer* container, VaxpColor color);

/**
 * @brief Set corner radius
 */
void vaxp_container_set_corner_radius(VaxpContainer* container, VaxpF32 radius);

/* Container class */
extern const VaxpWidgetClass vaxp_container_class;

#ifdef __cplusplus
}
#endif

#endif /* VAXP_CONTAINER_H */
