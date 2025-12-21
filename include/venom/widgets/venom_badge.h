/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_badge.h - Badge widget (notification indicator)
 */

#ifndef VENOM_BADGE_H
#define VENOM_BADGE_H

#include "venom/widgets/venom_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VenomBadge {
    VenomWidget base;
    
    VenomWidget* child;          /* Widget to attach badge to */
    char* text;                  /* Badge text (e.g., "5", "99+") */
    int count;                   /* Numeric count (-1 if using text) */
    
    VenomBool show_zero;         /* Show badge when count is 0 */
    VenomBool dot_only;          /* Show only a dot, no number */
    
    /* Styling */
    VenomColor bg_color;
    VenomColor text_color;
    VenomF32 size;               /* Badge min size */
    VenomF32 offset_x;           /* Badge x offset from top-right */
    VenomF32 offset_y;           /* Badge y offset from top-right */
    
} VenomBadge;

VenomResultPtr venom_badge_create(void);
VenomResult venom_badge_set_child(VenomBadge* badge, VenomWidget* child);
void venom_badge_set_count(VenomBadge* badge, int count);
void venom_badge_set_text(VenomBadge* badge, const char* text);
void venom_badge_set_dot(VenomBadge* badge, VenomBool dot_only);
void venom_badge_set_visible(VenomBadge* badge, VenomBool visible);

extern const VenomWidgetClass venom_badge_class;

#define venom_badge(...) _venom_badge_build(&(VenomBadgeConfig){ __VA_ARGS__ })

typedef struct VenomBadgeConfig {
    VenomWidget* child;
    int count;
    const char* text;
    VenomBool dot_only;
    VenomColor color;
} VenomBadgeConfig;

VenomWidget* _venom_badge_build(const VenomBadgeConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VENOM_BADGE_H */
