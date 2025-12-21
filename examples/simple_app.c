/*
 * VENOMUI - Simple Example
 * 
 * This demonstrates the Flutter-like simplified API.
 * Compare this with hello_world.c to see the difference!
 */

#include <stdio.h>
#include <venom/venomui.h>

/* Button click handlers */
static void on_primary_click(VenomButton* btn, void* data) {
    (void)btn; (void)data;
    printf("Primary button clicked!\n");
}

static void on_success_click(VenomButton* btn, void* data) {
    (void)btn; (void)data;
    printf("Success button clicked!\n");
}

static void on_danger_click(VenomButton* btn, void* data) {
    (void)btn; (void)data;
    printf("Danger button clicked!\n");
}

/* Build the UI - like Flutter's build() method */
VenomWidget* build_app(void* user_data) {
    (void)user_data;
    
    /* Create widgets individually then compose */
    VenomWidget* title = venom_text("Welcome to VENOMUI!");
    if (title) {
        venom_label_set_font_size((VenomLabel*)title, 28);
        venom_label_set_color((VenomLabel*)title, VENOM_DARK);
    }
    
    VenomWidget* subtitle = venom_text("A Flutter-like C GUI Framework");
    if (subtitle) {
        venom_label_set_font_size((VenomLabel*)subtitle, 16);
        venom_label_set_color((VenomLabel*)subtitle, VENOM_MUTED);
    }
    
    VenomWidget* spacer = venom_sized_box(0, 15);
    
    /* Create buttons */
    VenomWidget* btn1 = venom_btn("Primary", .color = VENOM_PRIMARY, .on_click = on_primary_click);
    VenomWidget* btn2 = venom_btn("Success", .color = VENOM_SUCCESS, .on_click = on_success_click);
    VenomWidget* btn3 = venom_btn("Danger", .color = VENOM_DANGER, .on_click = on_danger_click);
    
    /* Create button row */
    VenomWidget* button_row = venom_row(.gap = 12, .children = VENOM_CHILDREN(btn1, btn2, btn3));
    
    /* Create main container */
    return venom_center(
        .gap = 20,
        .padding = { 40, 40, 40, 40 },
        .background = VENOM_LIGHT,
        .corner_radius = 16,
        .children = VENOM_CHILDREN(title, subtitle, spacer, button_row)
    );
}

/* Ultra-simple main! */
int main(void) {
    return VENOM_APP(
        .title = "VENOMUI - Simple Example",
        .width = 550,
        .height = 400,
        .build = build_app,
        .debug = VENOM_TRUE
    );
}
