/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_container.h - Container widget with layout capabilities
 */

#ifndef VENOM_CONTAINER_H
#define VENOM_CONTAINER_H

#include "venom_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * CONTAINER TYPES
 * ============================================================================ */

typedef enum VenomJustify {
    VENOM_JUSTIFY_START,
    VENOM_JUSTIFY_CENTER,
    VENOM_JUSTIFY_END,
    VENOM_JUSTIFY_SPACE_BETWEEN,
    VENOM_JUSTIFY_SPACE_AROUND,
    VENOM_JUSTIFY_SPACE_EVENLY,
} VenomJustify;

typedef struct VenomContainer {
    VenomWidget base;
    
    VenomFlexDirection direction;
    VenomJustify justify;
    VenomAlign align_items;
    VenomF32 gap;               /* Space between items */
    
    /* Optional background */
    VenomColor bg_color;
    VenomF32 corner_radius;
    VenomBool has_background;
} VenomContainer;

/* ============================================================================
 * CONTAINER API
 * ============================================================================ */

/**
 * @brief Create a container
 */
VENOM_WARN_UNUSED
VenomResultPtr venom_container_create(void);

/**
 * @brief Create a row container (flex-direction: row)
 */
VENOM_WARN_UNUSED
VenomResultPtr venom_container_create_row(void);

/**
 * @brief Create a column container (flex-direction: column)
 */
VENOM_WARN_UNUSED
VenomResultPtr venom_container_create_column(void);

/**
 * @brief Set flex direction
 */
void venom_container_set_direction(VenomContainer* container, VenomFlexDirection direction);

/**
 * @brief Set justify content
 */
void venom_container_set_justify(VenomContainer* container, VenomJustify justify);

/**
 * @brief Set align items
 */
void venom_container_set_align(VenomContainer* container, VenomAlign align);

/**
 * @brief Set gap between items
 */
void venom_container_set_gap(VenomContainer* container, VenomF32 gap);

/**
 * @brief Set background color
 */
void venom_container_set_background(VenomContainer* container, VenomColor color);

/**
 * @brief Set corner radius
 */
void venom_container_set_corner_radius(VenomContainer* container, VenomF32 radius);

/* Container class */
extern const VenomWidgetClass venom_container_class;

#ifdef __cplusplus
}
#endif

#endif /* VENOM_CONTAINER_H */
