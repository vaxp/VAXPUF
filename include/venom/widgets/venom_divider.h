/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_divider.h - Divider widget (horizontal/vertical line)
 */

#ifndef VENOM_DIVIDER_H
#define VENOM_DIVIDER_H

#include "venom/widgets/venom_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum VenomDividerDirection {
    VENOM_DIVIDER_HORIZONTAL,
    VENOM_DIVIDER_VERTICAL,
} VenomDividerDirection;

typedef struct VenomDivider {
    VenomWidget base;
    VenomDividerDirection direction;
    VenomF32 thickness;
    VenomColor color;
    VenomF32 indent_start;
    VenomF32 indent_end;
} VenomDivider;

VenomResultPtr venom_divider_create(void);
void venom_divider_set_direction(VenomDivider* div, VenomDividerDirection dir);
void venom_divider_set_thickness(VenomDivider* div, VenomF32 thickness);
void venom_divider_set_color(VenomDivider* div, VenomColor color);
void venom_divider_set_indent(VenomDivider* div, VenomF32 start, VenomF32 end);

extern const VenomWidgetClass venom_divider_class;

/* Convenience macro */
#define venom_divider(...) _venom_divider_build(&(VenomDividerConfig){ __VA_ARGS__ })

typedef struct VenomDividerConfig {
    VenomDividerDirection direction;
    VenomF32 thickness;
    VenomColor color;
    VenomF32 indent_start;
    VenomF32 indent_end;
} VenomDividerConfig;

VenomWidget* _venom_divider_build(const VenomDividerConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VENOM_DIVIDER_H */
