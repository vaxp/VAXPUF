/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_padding.h - Padding widget (add padding around child)
 */

#ifndef VENOM_PADDING_H
#define VENOM_PADDING_H

#include "venom/widgets/venom_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VenomPadding {
    VenomWidget base;
    VenomWidget* child;
    VenomInsets padding;
} VenomPadding;

VenomResultPtr venom_padding_create(void);
VenomResult venom_padding_set_child(VenomPadding* pad, VenomWidget* child);
void venom_padding_set_padding(VenomPadding* pad, VenomInsets padding);
void venom_padding_set_all(VenomPadding* pad, VenomF32 value);

extern const VenomWidgetClass venom_padding_class;

/* Convenience macro */
#define venom_padding(...) _venom_padding_build(&(VenomPaddingConfig){ __VA_ARGS__ })

typedef struct VenomPaddingConfig {
    VenomWidget* child;
    VenomInsets padding;
    VenomF32 all;  /* Set all sides at once */
} VenomPaddingConfig;

VenomWidget* _venom_padding_build(const VenomPaddingConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VENOM_PADDING_H */
