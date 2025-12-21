/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_stepper.h - Step indicator widget
 */

#ifndef VENOM_STEPPER_H
#define VENOM_STEPPER_H

#include "venom/widgets/venom_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VenomStepper VenomStepper;
typedef void (*VenomStepperCallback)(VenomStepper* stepper, VenomU32 step, void* data);

typedef struct VenomStep {
    char* label;
    VenomBool completed;
    VenomBool error;
} VenomStep;

struct VenomStepper {
    VenomWidget base;
    
    VenomStep* steps;
    VenomU32 step_count;
    VenomU32 step_capacity;
    VenomU32 current_step;
    
    VenomBool vertical;
    VenomBool clickable;
    
    VenomStepperCallback on_step_click;
    void* callback_data;
    
    VenomColor active_color;
    VenomColor completed_color;
    VenomColor inactive_color;
    VenomColor error_color;
    VenomColor connector_color;
    VenomF32 step_size;
};

VenomResultPtr venom_stepper_create(void);
VenomResult venom_stepper_add_step(VenomStepper* stepper, const char* label);
void venom_stepper_set_current(VenomStepper* stepper, VenomU32 step);
void venom_stepper_next(VenomStepper* stepper);
void venom_stepper_prev(VenomStepper* stepper);
void venom_stepper_complete_step(VenomStepper* stepper, VenomU32 step);
void venom_stepper_set_error(VenomStepper* stepper, VenomU32 step, VenomBool error);

extern const VenomWidgetClass venom_stepper_class;

#ifdef __cplusplus
}
#endif

#endif /* VENOM_STEPPER_H */
