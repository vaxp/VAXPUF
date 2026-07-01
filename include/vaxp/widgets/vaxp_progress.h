/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_progress.h - ProgressBar widget
 */

#ifndef VAXP_PROGRESS_H
#define VAXP_PROGRESS_H

#include "vaxp/widgets/vaxp_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum VaxpProgressType {
    VAXP_PROGRESS_LINEAR,
    VAXP_PROGRESS_INDETERMINATE,
} VaxpProgressType;

typedef struct VaxpProgressBar {
    VaxpWidget base;
    
    VaxpF32 value;              /* 0.0 - 1.0 */
    VaxpProgressType type;
    VaxpF32 animation_offset;   /* For indeterminate animation */
    
    /* Styling */
    VaxpF32 height;
    VaxpF32 corner_radius;
    VaxpColor track_color;
    VaxpColor fill_color;
    
} VaxpProgressBar;

VaxpResultPtr vaxp_progress_create(void);
void vaxp_progress_set_value(VaxpProgressBar* bar, VaxpF32 value);
VaxpF32 vaxp_progress_get_value(const VaxpProgressBar* bar);
void vaxp_progress_set_indeterminate(VaxpProgressBar* bar, VaxpBool indeterminate);
void vaxp_progress_set_colors(VaxpProgressBar* bar, VaxpColor track, VaxpColor fill);

extern const VaxpWidgetClass vaxp_progress_class;

#define vaxp_progress(...) _vaxp_progress_build(&(VaxpProgressConfig){ __VA_ARGS__ })

typedef struct VaxpProgressConfig {
    VaxpF32 value;
    VaxpBool indeterminate;
    VaxpColor fill_color;
} VaxpProgressConfig;

VaxpWidget* _vaxp_progress_build(const VaxpProgressConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VAXP_PROGRESS_H */
