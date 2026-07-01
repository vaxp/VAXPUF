/*
 * VAXPUI - TextInput Example
 * 
 * Demonstrates text input widget with focus navigation.
 */

#include <stdio.h>
#include <vaxp/vaxpui.h>

/* ============================================================================
 * CALLBACKS
 * ============================================================================ */

static void on_name_change(VaxpTextInput* input, const char* text, void* data) {
    (void)input;
    (void)data;
    printf("Name changed: '%s'\n", text);
}

static void on_password_change(VaxpTextInput* input, const char* text, void* data) {
    (void)input;
    (void)data;
    printf("Password length: %zu\n", strlen(text));
}

static void on_submit(VaxpTextInput* input, const char* text, void* data) {
    (void)input;
    (void)data;
    printf("Form submitted! Name: '%s'\n", text);
}

static void on_login(VaxpButton* btn, void* data) {
    (void)btn;
    (void)data;
    printf("Login button clicked!\n");
}

/* ============================================================================
 * BUILD UI
 * ============================================================================ */

VaxpWidget* build_app(void* data) {
    (void)data;
    
    return vaxp_center(
        .gap = 20,
        .padding = { 40, 40, 40, 40 },
        .background = VAXP_LIGHT,
        .children = VAXP_CHILDREN(
            vaxp_text("Login Form"),
            
            /* Name input */
            vaxp_col(
                .gap = 5,
                .children = VAXP_CHILDREN(
                    vaxp_text("Username:"),
                    vaxp_input(.placeholder = "Enter your name", .on_change = on_name_change, .on_submit = on_submit)
                )
            ),
            
            /* Password input */
            vaxp_col(
                .gap = 5,
                .children = VAXP_CHILDREN(
                    vaxp_text("Password:"),
                    vaxp_input(.placeholder = "Enter password", .password = VAXP_TRUE, .on_change = on_password_change)
                )
            ),
            
            /* Login button */
            vaxp_btn("Login", .color = VAXP_PRIMARY, .on_click = on_login)
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
    
    return VAXP_APP(
        .title = "VAXPUI - TextInput Demo",
        .width = 400,
        .height = 350,
        .build = build_app,
        .debug = VAXP_TRUE
    );
}
