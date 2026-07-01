/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_divider.h - Divider widget (horizontal/vertical line)
 */

#ifndef VAXP_DIVIDER_H
#define VAXP_DIVIDER_H

#include "vaxp/widgets/vaxp_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum VaxpDividerDirection {
    VAXP_DIVIDER_HORIZONTAL,
    VAXP_DIVIDER_VERTICAL,
} VaxpDividerDirection;

typedef struct VaxpDivider {
    VaxpWidget base;
    VaxpDividerDirection direction;
    VaxpF32 thickness;
    VaxpColor color;
    VaxpF32 indent_start;
    VaxpF32 indent_end;
} VaxpDivider;

VaxpResultPtr vaxp_divider_create(void);
void vaxp_divider_set_direction(VaxpDivider* div, VaxpDividerDirection dir);
void vaxp_divider_set_thickness(VaxpDivider* div, VaxpF32 thickness);
void vaxp_divider_set_color(VaxpDivider* div, VaxpColor color);
void vaxp_divider_set_indent(VaxpDivider* div, VaxpF32 start, VaxpF32 end);

extern const VaxpWidgetClass vaxp_divider_class;

/* Convenience macro */
#define vaxp_divider(...) _vaxp_divider_build(&(VaxpDividerConfig){ __VA_ARGS__ })

typedef struct VaxpDividerConfig {
    VaxpDividerDirection direction;
    VaxpF32 thickness;
    VaxpColor color;
    VaxpF32 indent_start;
    VaxpF32 indent_end;
} VaxpDividerConfig;

VaxpWidget* _vaxp_divider_build(const VaxpDividerConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VAXP_DIVIDER_H */
