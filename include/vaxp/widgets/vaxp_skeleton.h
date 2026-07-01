/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_skeleton.h - Skeleton loading placeholder
 */

#ifndef VAXP_SKELETON_H
#define VAXP_SKELETON_H

#include "vaxp/widgets/vaxp_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum VaxpSkeletonVariant {
    VAXP_SKELETON_TEXT,
    VAXP_SKELETON_CIRCULAR,
    VAXP_SKELETON_RECTANGULAR,
} VaxpSkeletonVariant;

typedef struct VaxpSkeleton {
    VaxpWidget base;
    
    VaxpSkeletonVariant variant;
    VaxpF32 width;
    VaxpF32 height;
    VaxpF32 corner_radius;
    VaxpBool animate;
    VaxpF32 animation_offset;
    
    VaxpColor base_color;
    VaxpColor highlight_color;
} VaxpSkeleton;

VaxpResultPtr vaxp_skeleton_create(void);
void vaxp_skeleton_set_variant(VaxpSkeleton* skel, VaxpSkeletonVariant variant);
void vaxp_skeleton_set_size(VaxpSkeleton* skel, VaxpF32 width, VaxpF32 height);
void vaxp_skeleton_animate(VaxpSkeleton* skel, VaxpF32 delta_time);

extern const VaxpWidgetClass vaxp_skeleton_class;

#define vaxp_skeleton(...) _vaxp_skeleton_build(&(VaxpSkeletonConfig){ __VA_ARGS__ })

typedef struct VaxpSkeletonConfig {
    VaxpSkeletonVariant variant;
    VaxpF32 width;
    VaxpF32 height;
} VaxpSkeletonConfig;

VaxpWidget* _vaxp_skeleton_build(const VaxpSkeletonConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VAXP_SKELETON_H */
