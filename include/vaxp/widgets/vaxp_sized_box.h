/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_sized_box.h - SizedBox widget (fixed size container)
 */

#ifndef VAXP_SIZED_BOX_H
#define VAXP_SIZED_BOX_H

#include "vaxp/widgets/vaxp_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VaxpSizedBox {
    VaxpWidget base;
    VaxpWidget* child;
    VaxpF32 width;
    VaxpF32 height;
} VaxpSizedBox;

VaxpResultPtr vaxp_sized_box_create(void);
VaxpResult vaxp_sized_box_set_child(VaxpSizedBox* box, VaxpWidget* child);
void vaxp_sized_box_set_size(VaxpSizedBox* box, VaxpF32 width, VaxpF32 height);

extern const VaxpWidgetClass vaxp_sized_box_class;

/* Convenience macro */
#define VAXP_SIZED_BOX(...) _vaxp_sized_box_build(&(VaxpSizedBoxConfig){ __VA_ARGS__ })

typedef struct VaxpSizedBoxConfig {
    VaxpWidget* child;
    VaxpF32 width;
    VaxpF32 height;
} VaxpSizedBoxConfig;

VaxpWidget* _vaxp_sized_box_build(const VaxpSizedBoxConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VAXP_SIZED_BOX_H */
