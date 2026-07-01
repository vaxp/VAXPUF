/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_stepper.h - Step indicator widget
 */

#ifndef VAXP_STEPPER_H
#define VAXP_STEPPER_H

#include "vaxp/widgets/vaxp_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VaxpStepper VaxpStepper;
typedef void (*VaxpStepperCallback)(VaxpStepper* stepper, VaxpU32 step, void* data);

typedef struct VaxpStep {
    char* label;
    VaxpBool completed;
    VaxpBool error;
} VaxpStep;

struct VaxpStepper {
    VaxpWidget base;
    
    VaxpStep* steps;
    VaxpU32 step_count;
    VaxpU32 step_capacity;
    VaxpU32 current_step;
    
    VaxpBool vertical;
    VaxpBool clickable;
    
    VaxpStepperCallback on_step_click;
    void* callback_data;
    
    VaxpColor active_color;
    VaxpColor completed_color;
    VaxpColor inactive_color;
    VaxpColor error_color;
    VaxpColor connector_color;
    VaxpF32 step_size;
};

VaxpResultPtr vaxp_stepper_create(void);
VaxpResult vaxp_stepper_add_step(VaxpStepper* stepper, const char* label);
void vaxp_stepper_set_current(VaxpStepper* stepper, VaxpU32 step);
void vaxp_stepper_next(VaxpStepper* stepper);
void vaxp_stepper_prev(VaxpStepper* stepper);
void vaxp_stepper_complete_step(VaxpStepper* stepper, VaxpU32 step);
void vaxp_stepper_set_error(VaxpStepper* stepper, VaxpU32 step, VaxpBool error);

extern const VaxpWidgetClass vaxp_stepper_class;

#ifdef __cplusplus
}
#endif

#endif /* VAXP_STEPPER_H */
