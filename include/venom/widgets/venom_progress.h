/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_progress.h - ProgressBar widget
 */

#ifndef VENOM_PROGRESS_H
#define VENOM_PROGRESS_H

#include "venom/widgets/venom_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum VenomProgressType {
    VENOM_PROGRESS_LINEAR,
    VENOM_PROGRESS_INDETERMINATE,
} VenomProgressType;

typedef struct VenomProgressBar {
    VenomWidget base;
    
    VenomF32 value;              /* 0.0 - 1.0 */
    VenomProgressType type;
    VenomF32 animation_offset;   /* For indeterminate animation */
    
    /* Styling */
    VenomF32 height;
    VenomF32 corner_radius;
    VenomColor track_color;
    VenomColor fill_color;
    
} VenomProgressBar;

VenomResultPtr venom_progress_create(void);
void venom_progress_set_value(VenomProgressBar* bar, VenomF32 value);
VenomF32 venom_progress_get_value(const VenomProgressBar* bar);
void venom_progress_set_indeterminate(VenomProgressBar* bar, VenomBool indeterminate);
void venom_progress_set_colors(VenomProgressBar* bar, VenomColor track, VenomColor fill);

extern const VenomWidgetClass venom_progress_class;

#define venom_progress(...) _venom_progress_build(&(VenomProgressConfig){ __VA_ARGS__ })

typedef struct VenomProgressConfig {
    VenomF32 value;
    VenomBool indeterminate;
    VenomColor fill_color;
} VenomProgressConfig;

VenomWidget* _venom_progress_build(const VenomProgressConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VENOM_PROGRESS_H */
