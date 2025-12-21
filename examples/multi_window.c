/*
 * VENOMUI - Multi-Window Example
 * 
 * Demonstrates creating and managing multiple windows.
 */

#include <stdio.h>
#include <venom/venomui.h>

/* Forward declarations */
static VenomWindow* main_window = NULL;
static VenomWindow* second_window = NULL;

/* ============================================================================
 * BUTTON CALLBACKS
 * ============================================================================ */

static void on_open_second(VenomButton* btn, void* data) {
    (void)btn; (void)data;
    
    if (second_window) {
        printf("Second window already open!\n");
        venom_window_request_focus(second_window);
        return;
    }
    
    printf("Opening second window...\n");
    
    /* Create second window content */
    VenomWidget* content = venom_center(
        .gap = 15,
        .background = venom_color_rgb(255, 240, 240),
        .padding = { 30, 30, 30, 30 },
        .children = VENOM_CHILDREN(
            venom_text("Second Window"),
            venom_text("This is a separate window!")
        )
    );
    
    /* Create second window */
    VenomResultPtr result = VENOM_WINDOW(
        .title = "VENOMUI - Second Window",
        .width = 400,
        .height = 300,
        .root = content
    );
    
    if (result.ok) {
        second_window = (VenomWindow*)result.value;
        venom_window_show(second_window);
    } else {
        printf("Failed to create second window\n");
    }
    
    venom_unref(content);  /* Window now owns it */
}

static void on_close_second(VenomButton* btn, void* data) {
    (void)btn; (void)data;
    
    if (second_window) {
        printf("Closing second window...\n");
        venom_window_close(second_window);
        second_window = NULL;
    } else {
        printf("Second window not open!\n");
    }
}

/* ============================================================================
 * MAIN
 * ============================================================================ */

int main(void) {
    printf("Multi-Window Demo\n");
    
    /* Initialize */
    VenomResult init_result = venom_init();
    if (!init_result.ok) {
        fprintf(stderr, "Failed to init: %s\n", venom_error_string(init_result.error));
        return 1;
    }
    
    VenomResult wm_result = venom_window_manager_init();
    if (!wm_result.ok) {
        fprintf(stderr, "Failed to init window manager: %s\n", venom_error_string(wm_result.error));
        return 1;
    }
    
    /* Create main window content */
    VenomWidget* main_content = venom_center(
        .gap = 20,
        .background = VENOM_LIGHT,
        .padding = { 40, 40, 40, 40 },
        .children = VENOM_CHILDREN(
            venom_text("Multi-Window Demo"),
            venom_btn("Open Second Window", .color = VENOM_PRIMARY, .on_click = on_open_second),
            venom_btn("Close Second Window", .color = VENOM_DANGER, .on_click = on_close_second)
        )
    );
    
    /* Create main window */
    VenomResultPtr win_result = VENOM_WINDOW(
        .title = "VENOMUI - Main Window",
        .width = 500,
        .height = 400,
        .root = main_content
    );
    
    if (!win_result.ok) {
        fprintf(stderr, "Failed to create main window\n");
        return 1;
    }
    
    main_window = (VenomWindow*)win_result.value;
    venom_window_show(main_window);
    venom_unref(main_content);
    
    printf("Running... Close main window to exit.\n");
    
    /* Run event loop */
    int result = venom_run();
    
    /* Cleanup */
    venom_window_manager_shutdown();
    venom_shutdown();
    
    printf("Goodbye!\n");
    return result;
}
