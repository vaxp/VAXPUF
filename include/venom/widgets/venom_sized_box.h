/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_sized_box.h - SizedBox widget (fixed size container)
 */

#ifndef VENOM_SIZED_BOX_H
#define VENOM_SIZED_BOX_H

#include "venom/widgets/venom_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VenomSizedBox {
    VenomWidget base;
    VenomWidget* child;
    VenomF32 width;
    VenomF32 height;
} VenomSizedBox;

VenomResultPtr venom_sized_box_create(void);
VenomResult venom_sized_box_set_child(VenomSizedBox* box, VenomWidget* child);
void venom_sized_box_set_size(VenomSizedBox* box, VenomF32 width, VenomF32 height);

extern const VenomWidgetClass venom_sized_box_class;

/* Convenience macro */
#define VENOM_SIZED_BOX(...) _venom_sized_box_build(&(VenomSizedBoxConfig){ __VA_ARGS__ })

typedef struct VenomSizedBoxConfig {
    VenomWidget* child;
    VenomF32 width;
    VenomF32 height;
} VenomSizedBoxConfig;

VenomWidget* _venom_sized_box_build(const VenomSizedBoxConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VENOM_SIZED_BOX_H */
