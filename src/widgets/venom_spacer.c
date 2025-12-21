/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_spacer.c - Spacer widget implementation
 */

#include "venom/widgets/venom_spacer.h"
#include "venom/core/venom_memory.h"

static void spacer_init(VenomWidget* widget) {
    VenomSpacer* spacer = (VenomSpacer*)widget;
    spacer->flex = 1.0f;
}

static void spacer_measure(VenomWidget* widget, VenomF32 available_width, VenomF32 available_height,
                           VenomF32* out_width, VenomF32* out_height) {
    (void)available_width; (void)available_height;
    
    /* Spacer has no intrinsic size - it expands to fill */
    *out_width = widget->layout.preferred_width > 0 ? widget->layout.preferred_width : 0;
    *out_height = widget->layout.preferred_height > 0 ? widget->layout.preferred_height : 0;
}

static void spacer_draw(VenomWidget* widget, VenomCanvas* canvas) {
    (void)widget; (void)canvas;
    /* Spacer draws nothing - it's just empty space */
}

const VenomWidgetClass venom_spacer_class = {
    .class_name = "VenomSpacer",
    .instance_size = sizeof(VenomSpacer),
    .parent_class = &venom_widget_class,
    .init = spacer_init,
    .destroy = NULL,
    .measure = spacer_measure,
    .layout = NULL,
    .draw = spacer_draw,
    .on_event = NULL,
    .on_state_changed = NULL,
};

VenomResultPtr venom_spacer_create(void) {
    return venom_widget_create(&venom_spacer_class);
}

void venom_spacer_set_flex(VenomSpacer* spacer, VenomF32 flex) {
    if (spacer) spacer->flex = flex;
}

VenomWidget* _venom_spacer_build(VenomF32 flex) {
    VenomResultPtr result = venom_spacer_create();
    if (!result.ok) return NULL;
    
    VenomSpacer* spacer = (VenomSpacer*)result.value;
    spacer->flex = flex > 0 ? flex : 1.0f;
    return (VenomWidget*)spacer;
}
