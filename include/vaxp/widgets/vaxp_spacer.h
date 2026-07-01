/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_spacer.h - Spacer widget (flexible space)
 */

#ifndef VAXP_SPACER_H
#define VAXP_SPACER_H

#include "vaxp/widgets/vaxp_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VaxpSpacer {
    VaxpWidget base;
    VaxpF32 flex;  /* Flex factor for expanding */
} VaxpSpacer;

VaxpResultPtr vaxp_spacer_create(void);
void vaxp_spacer_set_flex(VaxpSpacer* spacer, VaxpF32 flex);

extern const VaxpWidgetClass vaxp_spacer_class;

/* Convenience macro */
#define VAXP_SPACER(...) _vaxp_spacer_build((VaxpF32[]){ __VA_ARGS__ + 0 }[0])
VaxpWidget* _vaxp_spacer_build(VaxpF32 flex);

#ifdef __cplusplus
}
#endif

#endif /* VAXP_SPACER_H */
