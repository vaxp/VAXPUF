/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_snackbar.h - Snackbar widget (bottom message bar)
 */

#ifndef VENOM_SNACKBAR_H
#define VENOM_SNACKBAR_H

#include "venom/widgets/venom_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VenomSnackbar VenomSnackbar;
typedef void (*VenomSnackbarAction)(VenomSnackbar* bar, void* data);

struct VenomSnackbar {
    VenomWidget base;
    
    char* message;
    char* action_label;
    VenomSnackbarAction on_action;
    void* action_data;
    
    VenomBool is_showing;
    VenomU32 duration_ms;
    
    VenomColor background_color;
    VenomColor text_color;
    VenomColor action_color;
    VenomF32 corner_radius;
    VenomF32 max_width;
};

VenomResultPtr venom_snackbar_create(void);
void venom_snackbar_set_message(VenomSnackbar* bar, const char* message);
void venom_snackbar_set_action(VenomSnackbar* bar, const char* label, VenomSnackbarAction action, void* data);
void venom_snackbar_show(VenomSnackbar* bar);
void venom_snackbar_hide(VenomSnackbar* bar);

extern const VenomWidgetClass venom_snackbar_class;

#define venom_snackbar(...) _venom_snackbar_build(&(VenomSnackbarConfig){ __VA_ARGS__ })

typedef struct VenomSnackbarConfig {
    const char* message;
    const char* action;
    VenomSnackbarAction on_action;
    void* data;
    VenomU32 duration;
} VenomSnackbarConfig;

VenomWidget* _venom_snackbar_build(const VenomSnackbarConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VENOM_SNACKBAR_H */
