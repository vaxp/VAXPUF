/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_skeleton.h - Skeleton loading placeholder
 */

#ifndef VENOM_SKELETON_H
#define VENOM_SKELETON_H

#include "venom/widgets/venom_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum VenomSkeletonVariant {
    VENOM_SKELETON_TEXT,
    VENOM_SKELETON_CIRCULAR,
    VENOM_SKELETON_RECTANGULAR,
} VenomSkeletonVariant;

typedef struct VenomSkeleton {
    VenomWidget base;
    
    VenomSkeletonVariant variant;
    VenomF32 width;
    VenomF32 height;
    VenomF32 corner_radius;
    VenomBool animate;
    VenomF32 animation_offset;
    
    VenomColor base_color;
    VenomColor highlight_color;
} VenomSkeleton;

VenomResultPtr venom_skeleton_create(void);
void venom_skeleton_set_variant(VenomSkeleton* skel, VenomSkeletonVariant variant);
void venom_skeleton_set_size(VenomSkeleton* skel, VenomF32 width, VenomF32 height);
void venom_skeleton_animate(VenomSkeleton* skel, VenomF32 delta_time);

extern const VenomWidgetClass venom_skeleton_class;

#define venom_skeleton(...) _venom_skeleton_build(&(VenomSkeletonConfig){ __VA_ARGS__ })

typedef struct VenomSkeletonConfig {
    VenomSkeletonVariant variant;
    VenomF32 width;
    VenomF32 height;
} VenomSkeletonConfig;

VenomWidget* _venom_skeleton_build(const VenomSkeletonConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VENOM_SKELETON_H */
