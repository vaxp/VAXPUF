/*
 * VAXPUI - Simple Example
 * 
 * This demonstrates the Flutter-like simplified API.
 * Compare this with hello_world.c to see the difference!
 */

#include <stdio.h>
#include <vaxp/vaxpui.h>

/* Button click handlers */
static void on_primary_click(VaxpButton* btn, void* data) {
    (void)btn; (void)data;
    printf("Primary button clicked!\n");
}

static void on_success_click(VaxpButton* btn, void* data) {
    (void)btn; (void)data;
    printf("Success button clicked!\n");
}

static void on_danger_click(VaxpButton* btn, void* data) {
    (void)btn; (void)data;
    printf("Danger button clicked!\n");
}

/* Build the UI - like Flutter's build() method */
VaxpWidget* build_app(void* user_data) {
    (void)user_data;
    
    /* Create widgets individually then compose */
    VaxpWidget* title = vaxp_text("Welcome to VAXPUI!");
    if (title) {
        vaxp_label_set_font_size((VaxpLabel*)title, 28);
        vaxp_label_set_color((VaxpLabel*)title, VAXP_DARK);
    }
    
    VaxpWidget* subtitle = vaxp_text("A Flutter-like C GUI Framework");
    if (subtitle) {
        vaxp_label_set_font_size((VaxpLabel*)subtitle, 16);
        vaxp_label_set_color((VaxpLabel*)subtitle, VAXP_MUTED);
    }
    
    VaxpWidget* spacer = vaxp_sized_box(0, 15);
    
    /* Create buttons */
    VaxpWidget* btn1 = vaxp_btn("Primary", .color = VAXP_PRIMARY, .on_click = on_primary_click);
    VaxpWidget* btn2 = vaxp_btn("Success", .color = VAXP_SUCCESS, .on_click = on_success_click);
    VaxpWidget* btn3 = vaxp_btn("Danger", .color = VAXP_DANGER, .on_click = on_danger_click);
    
    /* Create button row */
    VaxpWidget* button_row = vaxp_row(.gap = 12, .children = VAXP_CHILDREN(btn1, btn2, btn3));
    
    /* Create main container */
    return vaxp_center(
        .gap = 20,
        .padding = { 40, 40, 40, 40 },
        .background = VAXP_LIGHT,
        .corner_radius = 16,
        .children = VAXP_CHILDREN(title, subtitle, spacer, button_row)
    );
}

/* Ultra-simple main! */
int main(void) {
    return VAXP_APP(
        .title = "VAXPUI - Simple Example",
        .width = 550,
        .height = 400,
        .build = build_app,
        .debug = VAXP_TRUE
    );
}
