/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_popover.h - Popover widget (anchored popup)
 * 
 * Features:
 * - Anchored to trigger element
 * - Multiple placement options
 * - Arrow indicator
 * - Auto-close on outside click
 * - Animation support
 */

#ifndef VENOM_POPOVER_H
#define VENOM_POPOVER_H

#include "venom/widgets/venom_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VenomPopover VenomPopover;

typedef enum VenomPopoverPlacement {
    VENOM_POPOVER_TOP,
    VENOM_POPOVER_TOP_START,
    VENOM_POPOVER_TOP_END,
    VENOM_POPOVER_BOTTOM,
    VENOM_POPOVER_BOTTOM_START,
    VENOM_POPOVER_BOTTOM_END,
    VENOM_POPOVER_LEFT,
    VENOM_POPOVER_LEFT_START,
    VENOM_POPOVER_LEFT_END,
    VENOM_POPOVER_RIGHT,
    VENOM_POPOVER_RIGHT_START,
    VENOM_POPOVER_RIGHT_END,
} VenomPopoverPlacement;

typedef enum VenomPopoverTrigger {
    VENOM_POPOVER_TRIGGER_CLICK,
    VENOM_POPOVER_TRIGGER_HOVER,
    VENOM_POPOVER_TRIGGER_FOCUS,
    VENOM_POPOVER_TRIGGER_MANUAL,
} VenomPopoverTrigger;

typedef void (*VenomPopoverCallback)(VenomPopover* popover, VenomBool open, void* data);

struct VenomPopover {
    VenomWidget base;
    
    /* Anchor element */
    VenomWidget* anchor;
    VenomWidget* content;
    
    /* State */
    VenomBool is_open;
    VenomPopoverPlacement placement;
    VenomPopoverTrigger trigger;
    VenomBool show_arrow;
    VenomBool close_on_outside;
    VenomBool close_on_escape;
    
    /* Positioning */
    VenomF32 offset;              /* Distance from anchor */
    VenomF32 arrow_size;
    VenomRectF anchor_rect;       /* Computed anchor bounds */
    VenomRectF content_rect;      /* Computed content bounds */
    
    /* Styling */
    VenomColor background_color;
    VenomColor border_color;
    VenomColor shadow_color;
    VenomF32 corner_radius;
    VenomF32 border_width;
    VenomF32 shadow_blur;
    VenomF32 padding;
    
    /* Callbacks */
    VenomPopoverCallback on_open;
    VenomPopoverCallback on_close;
    void* callback_data;
};

/* ============================================================================
 * API
 * ============================================================================ */

VenomResultPtr venom_popover_create(void);

void venom_popover_set_anchor(VenomPopover* popover, VenomWidget* anchor);
VenomResult venom_popover_set_content(VenomPopover* popover, VenomWidget* content);
void venom_popover_set_placement(VenomPopover* popover, VenomPopoverPlacement placement);
void venom_popover_set_trigger(VenomPopover* popover, VenomPopoverTrigger trigger);
void venom_popover_set_offset(VenomPopover* popover, VenomF32 offset);
void venom_popover_set_arrow(VenomPopover* popover, VenomBool show);

void venom_popover_open(VenomPopover* popover);
void venom_popover_close(VenomPopover* popover);
void venom_popover_toggle(VenomPopover* popover);
VenomBool venom_popover_is_open(const VenomPopover* popover);

void venom_popover_set_on_open(VenomPopover* popover, VenomPopoverCallback cb, void* data);
void venom_popover_set_on_close(VenomPopover* popover, VenomPopoverCallback cb, void* data);

extern const VenomWidgetClass venom_popover_class;

/* ============================================================================
 * CONVENIENCE MACRO
 * ============================================================================ */

#define venom_popover(...) _venom_popover_build(&(VenomPopoverConfig){ __VA_ARGS__ })

typedef struct VenomPopoverConfig {
    VenomWidget* anchor;
    VenomWidget* content;
    VenomPopoverPlacement placement;
    VenomPopoverTrigger trigger;
    VenomBool show_arrow;
    VenomF32 offset;
} VenomPopoverConfig;

VenomWidget* _venom_popover_build(const VenomPopoverConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VENOM_POPOVER_H */
