/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_spacer.h - Spacer widget (flexible space)
 */

#ifndef VENOM_SPACER_H
#define VENOM_SPACER_H

#include "venom/widgets/venom_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VenomSpacer {
    VenomWidget base;
    VenomF32 flex;  /* Flex factor for expanding */
} VenomSpacer;

VenomResultPtr venom_spacer_create(void);
void venom_spacer_set_flex(VenomSpacer* spacer, VenomF32 flex);

extern const VenomWidgetClass venom_spacer_class;

/* Convenience macro */
#define VENOM_SPACER(...) _venom_spacer_build((VenomF32[]){ __VA_ARGS__ + 0 }[0])
VenomWidget* _venom_spacer_build(VenomF32 flex);

#ifdef __cplusplus
}
#endif

#endif /* VENOM_SPACER_H */
