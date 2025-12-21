/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_tooltip.h - Tooltip widget (hover information)
 */

#ifndef VENOM_TOOLTIP_H
#define VENOM_TOOLTIP_H

#include "venom/widgets/venom_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum VenomTooltipPosition {
    VENOM_TOOLTIP_TOP,
    VENOM_TOOLTIP_BOTTOM,
    VENOM_TOOLTIP_LEFT,
    VENOM_TOOLTIP_RIGHT,
    VENOM_TOOLTIP_AUTO,
} VenomTooltipPosition;

typedef struct VenomTooltip {
    VenomWidget base;
    
    VenomWidget* child;          /* The widget to show tooltip for */
    char* message;               /* Tooltip text */
    VenomTooltipPosition position;
    
    /* Timing */
    VenomU32 show_delay_ms;      /* Delay before showing (default 500ms) */
    VenomU32 hide_delay_ms;      /* Delay before hiding after mouse leave */
    
    /* State */
    VenomBool showing;
    VenomF32 popup_x;
    VenomF32 popup_y;
    
    /* Styling */
    VenomColor bg_color;
    VenomColor text_color;
    VenomF32 padding;
    VenomF32 corner_radius;
    
} VenomTooltip;

VenomResultPtr venom_tooltip_create(void);
VenomResult venom_tooltip_set_child(VenomTooltip* tip, VenomWidget* child);
void venom_tooltip_set_message(VenomTooltip* tip, const char* message);
void venom_tooltip_set_position(VenomTooltip* tip, VenomTooltipPosition pos);
void venom_tooltip_show(VenomTooltip* tip);
void venom_tooltip_hide(VenomTooltip* tip);

extern const VenomWidgetClass venom_tooltip_class;

#define venom_tooltip(...) _venom_tooltip_build(&(VenomTooltipConfig){ __VA_ARGS__ })

typedef struct VenomTooltipConfig {
    VenomWidget* child;
    const char* message;
    VenomTooltipPosition position;
} VenomTooltipConfig;

VenomWidget* _venom_tooltip_build(const VenomTooltipConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VENOM_TOOLTIP_H */
