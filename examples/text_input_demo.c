/*
 * VENOMUI - TextInput Example
 * 
 * Demonstrates text input widget with focus navigation.
 */

#include <stdio.h>
#include <venom/venomui.h>

/* ============================================================================
 * CALLBACKS
 * ============================================================================ */

static void on_name_change(VenomTextInput* input, const char* text, void* data) {
    (void)input;
    (void)data;
    printf("Name changed: '%s'\n", text);
}

static void on_password_change(VenomTextInput* input, const char* text, void* data) {
    (void)input;
    (void)data;
    printf("Password length: %zu\n", strlen(text));
}

static void on_submit(VenomTextInput* input, const char* text, void* data) {
    (void)input;
    (void)data;
    printf("Form submitted! Name: '%s'\n", text);
}

static void on_login(VenomButton* btn, void* data) {
    (void)btn;
    (void)data;
    printf("Login button clicked!\n");
}

/* ============================================================================
 * BUILD UI
 * ============================================================================ */

VenomWidget* build_app(void* data) {
    (void)data;
    
    return venom_center(
        .gap = 20,
        .padding = { 40, 40, 40, 40 },
        .background = VENOM_LIGHT,
        .children = VENOM_CHILDREN(
            venom_text("Login Form"),
            
            /* Name input */
            venom_col(
                .gap = 5,
                .children = VENOM_CHILDREN(
                    venom_text("Username:"),
                    venom_input(.placeholder = "Enter your name", .on_change = on_name_change, .on_submit = on_submit)
                )
            ),
            
            /* Password input */
            venom_col(
                .gap = 5,
                .children = VENOM_CHILDREN(
                    venom_text("Password:"),
                    venom_input(.placeholder = "Enter password", .password = VENOM_TRUE, .on_change = on_password_change)
                )
            ),
            
            /* Login button */
            venom_btn("Login", .color = VENOM_PRIMARY, .on_click = on_login)
        )
    );
}

/* ============================================================================
 * MAIN
 * ============================================================================ */

int main(void) {
    printf("TextInput Demo\n");
    printf("Use Tab to navigate, type to enter text\n");
    printf("Arrow keys to move cursor, Backspace/Delete to remove\n\n");
    
    return VENOM_APP(
        .title = "VENOMUI - TextInput Demo",
        .width = 400,
        .height = 350,
        .build = build_app,
        .debug = VENOM_TRUE
    );
}
