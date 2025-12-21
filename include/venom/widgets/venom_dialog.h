/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_dialog.h - Modal dialog widget
 */

#ifndef VENOM_DIALOG_H
#define VENOM_DIALOG_H

#include "venom/widgets/venom_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VenomDialog VenomDialog;

typedef void (*VenomDialogCallback)(VenomDialog* dialog, int result, void* user_data);

/* Standard dialog results */
#define VENOM_DIALOG_OK     1
#define VENOM_DIALOG_CANCEL 0
#define VENOM_DIALOG_YES    2
#define VENOM_DIALOG_NO     3

struct VenomDialog {
    VenomWidget base;
    
    char* title;
    VenomWidget* content;
    VenomWidget* actions;        /* Container for action buttons */
    
    VenomBool is_open;
    VenomBool modal;             /* Block interaction with parent */
    VenomBool dismissible;       /* Can be dismissed by clicking outside */
    
    /* Sizing */
    VenomF32 min_width;
    VenomF32 max_width;
    VenomF32 min_height;
    
    /* Styling */
    VenomColor backdrop_color;
    VenomColor background_color;
    VenomF32 corner_radius;
    VenomF32 padding;
    VenomF32 title_padding;
    
    /* Callback */
    VenomDialogCallback on_close;
    void* callback_data;
    int result;
    
};

VenomResultPtr venom_dialog_create(void);
void venom_dialog_set_title(VenomDialog* dialog, const char* title);
VenomResult venom_dialog_set_content(VenomDialog* dialog, VenomWidget* content);
VenomResult venom_dialog_add_action(VenomDialog* dialog, VenomWidget* button, int result);
void venom_dialog_open(VenomDialog* dialog);
void venom_dialog_close(VenomDialog* dialog, int result);
VenomBool venom_dialog_is_open(const VenomDialog* dialog);
void venom_dialog_set_on_close(VenomDialog* dialog, VenomDialogCallback callback, void* data);

extern const VenomWidgetClass venom_dialog_class;

#define venom_dialog(...) _venom_dialog_build(&(VenomDialogConfig){ __VA_ARGS__ })

typedef struct VenomDialogConfig {
    const char* title;
    VenomWidget* content;
    VenomBool modal;
    VenomDialogCallback on_close;
    void* data;
} VenomDialogConfig;

VenomWidget* _venom_dialog_build(const VenomDialogConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VENOM_DIALOG_H */
