/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_tooltip.h - Tooltip widget (hover information)
 */

#ifndef VAXP_TOOLTIP_H
#define VAXP_TOOLTIP_H

#include "vaxp/widgets/vaxp_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum VaxpTooltipPosition {
    VAXP_TOOLTIP_TOP,
    VAXP_TOOLTIP_BOTTOM,
    VAXP_TOOLTIP_LEFT,
    VAXP_TOOLTIP_RIGHT,
    VAXP_TOOLTIP_AUTO,
} VaxpTooltipPosition;

typedef struct VaxpTooltip {
    VaxpWidget base;
    
    VaxpWidget* child;          /* The widget to show tooltip for */
    char* message;               /* Tooltip text */
    VaxpTooltipPosition position;
    
    /* Timing */
    VaxpU32 show_delay_ms;      /* Delay before showing (default 500ms) */
    VaxpU32 hide_delay_ms;      /* Delay before hiding after mouse leave */
    
    /* State */
    VaxpBool showing;
    VaxpF32 popup_x;
    VaxpF32 popup_y;
    
    /* Styling */
    VaxpColor bg_color;
    VaxpColor text_color;
    VaxpF32 padding;
    VaxpF32 corner_radius;
    
} VaxpTooltip;

VaxpResultPtr vaxp_tooltip_create(void);
VaxpResult vaxp_tooltip_set_child(VaxpTooltip* tip, VaxpWidget* child);
void vaxp_tooltip_set_message(VaxpTooltip* tip, const char* message);
void vaxp_tooltip_set_position(VaxpTooltip* tip, VaxpTooltipPosition pos);
void vaxp_tooltip_show(VaxpTooltip* tip);
void vaxp_tooltip_hide(VaxpTooltip* tip);

extern const VaxpWidgetClass vaxp_tooltip_class;

#define vaxp_tooltip(...) _vaxp_tooltip_build(&(VaxpTooltipConfig){ __VA_ARGS__ })

typedef struct VaxpTooltipConfig {
    VaxpWidget* child;
    const char* message;
    VaxpTooltipPosition position;
} VaxpTooltipConfig;

VaxpWidget* _vaxp_tooltip_build(const VaxpTooltipConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VAXP_TOOLTIP_H */
