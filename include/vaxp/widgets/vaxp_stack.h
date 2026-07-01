/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_stack.h - Stack widget (overlay children)
 */

#ifndef VAXP_STACK_H
#define VAXP_STACK_H

#include "vaxp/widgets/vaxp_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * STACK ALIGNMENT
 * ============================================================================ */

typedef enum VaxpStackAlignment {
    VAXP_STACK_TOP_LEFT,
    VAXP_STACK_TOP_CENTER,
    VAXP_STACK_TOP_RIGHT,
    VAXP_STACK_CENTER_LEFT,
    VAXP_STACK_CENTER,
    VAXP_STACK_CENTER_RIGHT,
    VAXP_STACK_BOTTOM_LEFT,
    VAXP_STACK_BOTTOM_CENTER,
    VAXP_STACK_BOTTOM_RIGHT,
} VaxpStackAlignment;

/* ============================================================================
 * STACK WIDGET
 * ============================================================================ */

typedef struct VaxpStack {
    VaxpWidget base;
    
    VaxpWidget** children;
    VaxpU32 child_count;
    VaxpU32 child_capacity;
    
    VaxpStackAlignment alignment;
    
} VaxpStack;

/* ============================================================================
 * API
 * ============================================================================ */

VaxpResultPtr vaxp_stack_create(void);
VaxpResult vaxp_stack_add_child(VaxpStack* stack, VaxpWidget* child);
VaxpResult vaxp_stack_remove_child(VaxpStack* stack, VaxpWidget* child);
void vaxp_stack_set_alignment(VaxpStack* stack, VaxpStackAlignment alignment);

extern const VaxpWidgetClass vaxp_stack_class;

/* Macro */
#define vaxp_stack(...) \
    _vaxp_stack_build(&(VaxpStackConfig){ __VA_ARGS__ })

typedef struct VaxpStackConfig {
    VaxpStackAlignment alignment;
    VaxpWidget** children;
    VaxpU32 child_count;
} VaxpStackConfig;

VaxpWidget* _vaxp_stack_build(const VaxpStackConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VAXP_STACK_H */
