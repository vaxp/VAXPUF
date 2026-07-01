/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_spacer.c - Spacer widget implementation
 */

#include "vaxp/widgets/vaxp_spacer.h"
#include "vaxp/core/vaxp_memory.h"

static void spacer_init(VaxpWidget* widget) {
    VaxpSpacer* spacer = (VaxpSpacer*)widget;
    spacer->flex = 1.0f;
}

static void spacer_measure(VaxpWidget* widget, VaxpF32 available_width, VaxpF32 available_height,
                           VaxpF32* out_width, VaxpF32* out_height) {
    (void)available_width; (void)available_height;
    
    /* Spacer has no intrinsic size - it expands to fill */
    *out_width = widget->layout.preferred_width > 0 ? widget->layout.preferred_width : 0;
    *out_height = widget->layout.preferred_height > 0 ? widget->layout.preferred_height : 0;
}

static void spacer_draw(VaxpWidget* widget, VaxpCanvas* canvas) {
    (void)widget; (void)canvas;
    /* Spacer draws nothing - it's just empty space */
}

const VaxpWidgetClass vaxp_spacer_class = {
    .class_name = "VaxpSpacer",
    .instance_size = sizeof(VaxpSpacer),
    .parent_class = &vaxp_widget_class,
    .init = spacer_init,
    .destroy = NULL,
    .measure = spacer_measure,
    .layout = NULL,
    .draw = spacer_draw,
    .on_event = NULL,
    .on_state_changed = NULL,
};

VaxpResultPtr vaxp_spacer_create(void) {
    return vaxp_widget_create(&vaxp_spacer_class);
}

void vaxp_spacer_set_flex(VaxpSpacer* spacer, VaxpF32 flex) {
    if (spacer) spacer->flex = flex;
}

VaxpWidget* _vaxp_spacer_build(VaxpF32 flex) {
    VaxpResultPtr result = vaxp_spacer_create();
    if (!result.ok) return NULL;
    
    VaxpSpacer* spacer = (VaxpSpacer*)result.value;
    spacer->flex = flex > 0 ? flex : 1.0f;
    return (VaxpWidget*)spacer;
}
