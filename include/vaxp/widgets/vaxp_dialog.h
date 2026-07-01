/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_dialog.h - Modal dialog widget
 */

#ifndef VAXP_DIALOG_H
#define VAXP_DIALOG_H

#include "vaxp/widgets/vaxp_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VaxpDialog VaxpDialog;

typedef void (*VaxpDialogCallback)(VaxpDialog* dialog, int result, void* user_data);

/* Standard dialog results */
#define VAXP_DIALOG_OK     1
#define VAXP_DIALOG_CANCEL 0
#define VAXP_DIALOG_YES    2
#define VAXP_DIALOG_NO     3

struct VaxpDialog {
    VaxpWidget base;
    
    char* title;
    VaxpWidget* content;
    VaxpWidget* actions;        /* Container for action buttons */
    
    VaxpBool is_open;
    VaxpBool modal;             /* Block interaction with parent */
    VaxpBool dismissible;       /* Can be dismissed by clicking outside */
    
    /* Sizing */
    VaxpF32 min_width;
    VaxpF32 max_width;
    VaxpF32 min_height;
    
    /* Styling */
    VaxpColor backdrop_color;
    VaxpColor background_color;
    VaxpF32 corner_radius;
    VaxpF32 padding;
    VaxpF32 title_padding;
    
    /* Callback */
    VaxpDialogCallback on_close;
    void* callback_data;
    int result;
    
};

VaxpResultPtr vaxp_dialog_create(void);
void vaxp_dialog_set_title(VaxpDialog* dialog, const char* title);
VaxpResult vaxp_dialog_set_content(VaxpDialog* dialog, VaxpWidget* content);
VaxpResult vaxp_dialog_add_action(VaxpDialog* dialog, VaxpWidget* button, int result);
void vaxp_dialog_open(VaxpDialog* dialog);
void vaxp_dialog_close(VaxpDialog* dialog, int result);
VaxpBool vaxp_dialog_is_open(const VaxpDialog* dialog);
void vaxp_dialog_set_on_close(VaxpDialog* dialog, VaxpDialogCallback callback, void* data);

extern const VaxpWidgetClass vaxp_dialog_class;

#define vaxp_dialog(...) _vaxp_dialog_build(&(VaxpDialogConfig){ __VA_ARGS__ })

typedef struct VaxpDialogConfig {
    const char* title;
    VaxpWidget* content;
    VaxpBool modal;
    VaxpDialogCallback on_close;
    void* data;
} VaxpDialogConfig;

VaxpWidget* _vaxp_dialog_build(const VaxpDialogConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VAXP_DIALOG_H */
