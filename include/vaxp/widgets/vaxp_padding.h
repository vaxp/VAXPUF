/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_padding.h - Padding widget (add padding around child)
 */

#ifndef VAXP_PADDING_H
#define VAXP_PADDING_H

#include "vaxp/widgets/vaxp_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VaxpPadding {
    VaxpWidget base;
    VaxpWidget* child;
    VaxpInsets padding;
} VaxpPadding;

VaxpResultPtr vaxp_padding_create(void);
VaxpResult vaxp_padding_set_child(VaxpPadding* pad, VaxpWidget* child);
void vaxp_padding_set_padding(VaxpPadding* pad, VaxpInsets padding);
void vaxp_padding_set_all(VaxpPadding* pad, VaxpF32 value);

extern const VaxpWidgetClass vaxp_padding_class;

/* Convenience macro */
#define vaxp_padding(...) _vaxp_padding_build(&(VaxpPaddingConfig){ __VA_ARGS__ })

typedef struct VaxpPaddingConfig {
    VaxpWidget* child;
    VaxpInsets padding;
    VaxpF32 all;  /* Set all sides at once */
} VaxpPaddingConfig;

VaxpWidget* _vaxp_padding_build(const VaxpPaddingConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VAXP_PADDING_H */
