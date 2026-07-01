/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_spinner.h - Circular progress spinner
 */

#ifndef VAXP_SPINNER_H
#define VAXP_SPINNER_H

#include "vaxp/widgets/vaxp_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VaxpSpinner {
    VaxpWidget base;
    
    VaxpF32 size;
    VaxpF32 stroke_width;
    VaxpF32 angle;              /* Current rotation angle */
    VaxpColor color;
    VaxpColor track_color;
    VaxpBool show_track;
    
    /* Animation state (auto-animated) */
    VaxpF64 start_time;         /* Animation start time */
    VaxpF32 animation_duration; /* Full cycle duration in seconds */
    VaxpF32 arc_min;            /* Min arc length (fraction) */
    VaxpF32 arc_max;            /* Max arc length (fraction) */
    
} VaxpSpinner;

VaxpResultPtr vaxp_spinner_create(void);
void vaxp_spinner_set_size(VaxpSpinner* spinner, VaxpF32 size);
void vaxp_spinner_set_color(VaxpSpinner* spinner, VaxpColor color);
void vaxp_spinner_animate(VaxpSpinner* spinner, VaxpF32 delta_time);

extern const VaxpWidgetClass vaxp_spinner_class;

#define vaxp_spinner(...) _vaxp_spinner_build(&(VaxpSpinnerConfig){ __VA_ARGS__ })

typedef struct VaxpSpinnerConfig {
    VaxpF32 size;
    VaxpColor color;
} VaxpSpinnerConfig;

VaxpWidget* _vaxp_spinner_build(const VaxpSpinnerConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VAXP_SPINNER_H */
