/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_card.h - Card widget (elevated container)
 */

#ifndef VAXP_CARD_H
#define VAXP_CARD_H

#include "vaxp/widgets/vaxp_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VaxpCard {
    VaxpWidget base;
    
    VaxpWidget* child;
    
    /* Elevation & Shadow */
    VaxpF32 elevation;          /* 0-24, affects shadow */
    
    /* Styling */
    VaxpColor background_color;
    VaxpColor shadow_color;
    VaxpF32 corner_radius;
    VaxpF32 padding;
    VaxpBool outlined;          /* Use border instead of shadow */
    VaxpColor border_color;
    
} VaxpCard;

VaxpResultPtr vaxp_card_create(void);
VaxpResult vaxp_card_set_child(VaxpCard* card, VaxpWidget* child);
void vaxp_card_set_elevation(VaxpCard* card, VaxpF32 elevation);
void vaxp_card_set_corner_radius(VaxpCard* card, VaxpF32 radius);
void vaxp_card_set_color(VaxpCard* card, VaxpColor color);
void vaxp_card_set_outlined(VaxpCard* card, VaxpBool outlined);

extern const VaxpWidgetClass vaxp_card_class;

#define vaxp_card(...) _vaxp_card_build(&(VaxpCardConfig){ __VA_ARGS__ })

typedef struct VaxpCardConfig {
    VaxpWidget* child;
    VaxpF32 elevation;
    VaxpF32 corner_radius;
    VaxpF32 padding;
    VaxpBool outlined;
    VaxpColor color;
} VaxpCardConfig;

VaxpWidget* _vaxp_card_build(const VaxpCardConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VAXP_CARD_H */
