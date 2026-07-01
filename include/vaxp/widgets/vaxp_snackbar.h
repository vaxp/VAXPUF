/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_snackbar.h - Snackbar widget (bottom message bar)
 */

#ifndef VAXP_SNACKBAR_H
#define VAXP_SNACKBAR_H

#include "vaxp/widgets/vaxp_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VaxpSnackbar VaxpSnackbar;
typedef void (*VaxpSnackbarAction)(VaxpSnackbar* bar, void* data);

struct VaxpSnackbar {
    VaxpWidget base;
    
    char* message;
    char* action_label;
    VaxpSnackbarAction on_action;
    void* action_data;
    
    VaxpBool is_showing;
    VaxpU32 duration_ms;
    
    VaxpColor background_color;
    VaxpColor text_color;
    VaxpColor action_color;
    VaxpF32 corner_radius;
    VaxpF32 max_width;
};

VaxpResultPtr vaxp_snackbar_create(void);
void vaxp_snackbar_set_message(VaxpSnackbar* bar, const char* message);
void vaxp_snackbar_set_action(VaxpSnackbar* bar, const char* label, VaxpSnackbarAction action, void* data);
void vaxp_snackbar_show(VaxpSnackbar* bar);
void vaxp_snackbar_hide(VaxpSnackbar* bar);

extern const VaxpWidgetClass vaxp_snackbar_class;

#define vaxp_snackbar(...) _vaxp_snackbar_build(&(VaxpSnackbarConfig){ __VA_ARGS__ })

typedef struct VaxpSnackbarConfig {
    const char* message;
    const char* action;
    VaxpSnackbarAction on_action;
    void* data;
    VaxpU32 duration;
} VaxpSnackbarConfig;

VaxpWidget* _vaxp_snackbar_build(const VaxpSnackbarConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VAXP_SNACKBAR_H */
