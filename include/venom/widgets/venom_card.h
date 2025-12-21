/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_card.h - Card widget (elevated container)
 */

#ifndef VENOM_CARD_H
#define VENOM_CARD_H

#include "venom/widgets/venom_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VenomCard {
    VenomWidget base;
    
    VenomWidget* child;
    
    /* Elevation & Shadow */
    VenomF32 elevation;          /* 0-24, affects shadow */
    
    /* Styling */
    VenomColor background_color;
    VenomColor shadow_color;
    VenomF32 corner_radius;
    VenomF32 padding;
    VenomBool outlined;          /* Use border instead of shadow */
    VenomColor border_color;
    
} VenomCard;

VenomResultPtr venom_card_create(void);
VenomResult venom_card_set_child(VenomCard* card, VenomWidget* child);
void venom_card_set_elevation(VenomCard* card, VenomF32 elevation);
void venom_card_set_corner_radius(VenomCard* card, VenomF32 radius);
void venom_card_set_color(VenomCard* card, VenomColor color);
void venom_card_set_outlined(VenomCard* card, VenomBool outlined);

extern const VenomWidgetClass venom_card_class;

#define venom_card(...) _venom_card_build(&(VenomCardConfig){ __VA_ARGS__ })

typedef struct VenomCardConfig {
    VenomWidget* child;
    VenomF32 elevation;
    VenomF32 corner_radius;
    VenomF32 padding;
    VenomBool outlined;
    VenomColor color;
} VenomCardConfig;

VenomWidget* _venom_card_build(const VenomCardConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VENOM_CARD_H */
