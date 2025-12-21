/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_stack.h - Stack widget (overlay children)
 */

#ifndef VENOM_STACK_H
#define VENOM_STACK_H

#include "venom/widgets/venom_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * STACK ALIGNMENT
 * ============================================================================ */

typedef enum VenomStackAlignment {
    VENOM_STACK_TOP_LEFT,
    VENOM_STACK_TOP_CENTER,
    VENOM_STACK_TOP_RIGHT,
    VENOM_STACK_CENTER_LEFT,
    VENOM_STACK_CENTER,
    VENOM_STACK_CENTER_RIGHT,
    VENOM_STACK_BOTTOM_LEFT,
    VENOM_STACK_BOTTOM_CENTER,
    VENOM_STACK_BOTTOM_RIGHT,
} VenomStackAlignment;

/* ============================================================================
 * STACK WIDGET
 * ============================================================================ */

typedef struct VenomStack {
    VenomWidget base;
    
    VenomWidget** children;
    VenomU32 child_count;
    VenomU32 child_capacity;
    
    VenomStackAlignment alignment;
    
} VenomStack;

/* ============================================================================
 * API
 * ============================================================================ */

VenomResultPtr venom_stack_create(void);
VenomResult venom_stack_add_child(VenomStack* stack, VenomWidget* child);
VenomResult venom_stack_remove_child(VenomStack* stack, VenomWidget* child);
void venom_stack_set_alignment(VenomStack* stack, VenomStackAlignment alignment);

extern const VenomWidgetClass venom_stack_class;

/* Macro */
#define venom_stack(...) \
    _venom_stack_build(&(VenomStackConfig){ __VA_ARGS__ })

typedef struct VenomStackConfig {
    VenomStackAlignment alignment;
    VenomWidget** children;
    VenomU32 child_count;
} VenomStackConfig;

VenomWidget* _venom_stack_build(const VenomStackConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VENOM_STACK_H */
