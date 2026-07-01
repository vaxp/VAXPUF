/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_badge.h - Badge widget (notification indicator)
 */

#ifndef VAXP_BADGE_H
#define VAXP_BADGE_H

#include "vaxp/widgets/vaxp_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VaxpBadge {
    VaxpWidget base;
    
    VaxpWidget* child;          /* Widget to attach badge to */
    char* text;                  /* Badge text (e.g., "5", "99+") */
    int count;                   /* Numeric count (-1 if using text) */
    
    VaxpBool show_zero;         /* Show badge when count is 0 */
    VaxpBool dot_only;          /* Show only a dot, no number */
    
    /* Styling */
    VaxpColor bg_color;
    VaxpColor text_color;
    VaxpF32 size;               /* Badge min size */
    VaxpF32 offset_x;           /* Badge x offset from top-right */
    VaxpF32 offset_y;           /* Badge y offset from top-right */
    
} VaxpBadge;

VaxpResultPtr vaxp_badge_create(void);
VaxpResult vaxp_badge_set_child(VaxpBadge* badge, VaxpWidget* child);
void vaxp_badge_set_count(VaxpBadge* badge, int count);
void vaxp_badge_set_text(VaxpBadge* badge, const char* text);
void vaxp_badge_set_dot(VaxpBadge* badge, VaxpBool dot_only);
void vaxp_badge_set_visible(VaxpBadge* badge, VaxpBool visible);

extern const VaxpWidgetClass vaxp_badge_class;

#define vaxp_badge(...) _vaxp_badge_build(&(VaxpBadgeConfig){ __VA_ARGS__ })

typedef struct VaxpBadgeConfig {
    VaxpWidget* child;
    int count;
    const char* text;
    VaxpBool dot_only;
    VaxpColor color;
} VaxpBadgeConfig;

VaxpWidget* _vaxp_badge_build(const VaxpBadgeConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VAXP_BADGE_H */
