/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_spinner.h - Circular progress spinner
 */

#ifndef VENOM_SPINNER_H
#define VENOM_SPINNER_H

#include "venom/widgets/venom_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VenomSpinner {
    VenomWidget base;
    
    VenomF32 size;
    VenomF32 stroke_width;
    VenomF32 angle;              /* Current rotation angle */
    VenomColor color;
    VenomColor track_color;
    VenomBool show_track;
    
    /* Animation state (auto-animated) */
    VenomF64 start_time;         /* Animation start time */
    VenomF32 animation_duration; /* Full cycle duration in seconds */
    VenomF32 arc_min;            /* Min arc length (fraction) */
    VenomF32 arc_max;            /* Max arc length (fraction) */
    
} VenomSpinner;

VenomResultPtr venom_spinner_create(void);
void venom_spinner_set_size(VenomSpinner* spinner, VenomF32 size);
void venom_spinner_set_color(VenomSpinner* spinner, VenomColor color);
void venom_spinner_animate(VenomSpinner* spinner, VenomF32 delta_time);

extern const VenomWidgetClass venom_spinner_class;

#define venom_spinner(...) _venom_spinner_build(&(VenomSpinnerConfig){ __VA_ARGS__ })

typedef struct VenomSpinnerConfig {
    VenomF32 size;
    VenomColor color;
} VenomSpinnerConfig;

VenomWidget* _venom_spinner_build(const VenomSpinnerConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VENOM_SPINNER_H */
