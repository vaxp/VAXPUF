/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_popover.h - Popover widget (anchored popup)
 * 
 * Features:
 * - Anchored to trigger element
 * - Multiple placement options
 * - Arrow indicator
 * - Auto-close on outside click
 * - Animation support
 */

#ifndef VAXP_POPOVER_H
#define VAXP_POPOVER_H

#include "vaxp/widgets/vaxp_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VaxpPopover VaxpPopover;

typedef enum VaxpPopoverPlacement {
    VAXP_POPOVER_TOP,
    VAXP_POPOVER_TOP_START,
    VAXP_POPOVER_TOP_END,
    VAXP_POPOVER_BOTTOM,
    VAXP_POPOVER_BOTTOM_START,
    VAXP_POPOVER_BOTTOM_END,
    VAXP_POPOVER_LEFT,
    VAXP_POPOVER_LEFT_START,
    VAXP_POPOVER_LEFT_END,
    VAXP_POPOVER_RIGHT,
    VAXP_POPOVER_RIGHT_START,
    VAXP_POPOVER_RIGHT_END,
} VaxpPopoverPlacement;

typedef enum VaxpPopoverTrigger {
    VAXP_POPOVER_TRIGGER_CLICK,
    VAXP_POPOVER_TRIGGER_HOVER,
    VAXP_POPOVER_TRIGGER_FOCUS,
    VAXP_POPOVER_TRIGGER_MANUAL,
} VaxpPopoverTrigger;

typedef void (*VaxpPopoverCallback)(VaxpPopover* popover, VaxpBool open, void* data);

struct VaxpPopover {
    VaxpWidget base;
    
    /* Anchor element */
    VaxpWidget* anchor;
    VaxpWidget* content;
    
    /* State */
    VaxpBool is_open;
    VaxpPopoverPlacement placement;
    VaxpPopoverTrigger trigger;
    VaxpBool show_arrow;
    VaxpBool close_on_outside;
    VaxpBool close_on_escape;
    
    /* Positioning */
    VaxpF32 offset;              /* Distance from anchor */
    VaxpF32 arrow_size;
    VaxpRectF anchor_rect;       /* Computed anchor bounds */
    VaxpRectF content_rect;      /* Computed content bounds */
    
    /* Styling */
    VaxpColor background_color;
    VaxpColor border_color;
    VaxpColor shadow_color;
    VaxpF32 corner_radius;
    VaxpF32 border_width;
    VaxpF32 shadow_blur;
    VaxpF32 padding;
    
    /* Callbacks */
    VaxpPopoverCallback on_open;
    VaxpPopoverCallback on_close;
    void* callback_data;
};

/* ============================================================================
 * API
 * ============================================================================ */

VaxpResultPtr vaxp_popover_create(void);

void vaxp_popover_set_anchor(VaxpPopover* popover, VaxpWidget* anchor);
VaxpResult vaxp_popover_set_content(VaxpPopover* popover, VaxpWidget* content);
void vaxp_popover_set_placement(VaxpPopover* popover, VaxpPopoverPlacement placement);
void vaxp_popover_set_trigger(VaxpPopover* popover, VaxpPopoverTrigger trigger);
void vaxp_popover_set_offset(VaxpPopover* popover, VaxpF32 offset);
void vaxp_popover_set_arrow(VaxpPopover* popover, VaxpBool show);

void vaxp_popover_open(VaxpPopover* popover);
void vaxp_popover_close(VaxpPopover* popover);
void vaxp_popover_toggle(VaxpPopover* popover);
VaxpBool vaxp_popover_is_open(const VaxpPopover* popover);

void vaxp_popover_set_on_open(VaxpPopover* popover, VaxpPopoverCallback cb, void* data);
void vaxp_popover_set_on_close(VaxpPopover* popover, VaxpPopoverCallback cb, void* data);

extern const VaxpWidgetClass vaxp_popover_class;

/* ============================================================================
 * CONVENIENCE MACRO
 * ============================================================================ */

#define vaxp_popover(...) _vaxp_popover_build(&(VaxpPopoverConfig){ __VA_ARGS__ })

typedef struct VaxpPopoverConfig {
    VaxpWidget* anchor;
    VaxpWidget* content;
    VaxpPopoverPlacement placement;
    VaxpPopoverTrigger trigger;
    VaxpBool show_arrow;
    VaxpF32 offset;
} VaxpPopoverConfig;

VaxpWidget* _vaxp_popover_build(const VaxpPopoverConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VAXP_POPOVER_H */
