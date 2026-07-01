/*
 * VAXPUI - Multi-Window Example
 * 
 * Demonstrates creating and managing multiple windows.
 */

#include <stdio.h>
#include <vaxp/vaxpui.h>

/* Forward declarations */
static VaxpWindow* main_window = NULL;
static VaxpWindow* second_window = NULL;

/* ============================================================================
 * BUTTON CALLBACKS
 * ============================================================================ */

static void on_open_second(VaxpButton* btn, void* data) {
    (void)btn; (void)data;
    
    if (second_window) {
        printf("Second window already open!\n");
        vaxp_window_request_focus(second_window);
        return;
    }
    
    printf("Opening second window...\n");
    
    /* Create second window content */
    VaxpWidget* content = vaxp_center(
        .gap = 15,
        .background = vaxp_color_rgb(255, 240, 240),
        .padding = { 30, 30, 30, 30 },
        .children = VAXP_CHILDREN(
            vaxp_text("Second Window"),
            vaxp_text("This is a separate window!")
        )
    );
    
    /* Create second window */
    VaxpResultPtr result = VAXP_WINDOW(
        .title = "VAXPUI - Second Window",
        .width = 400,
        .height = 300,
        .root = content
    );
    
    if (result.ok) {
        second_window = (VaxpWindow*)result.value;
        vaxp_window_show(second_window);
    } else {
        printf("Failed to create second window\n");
    }
    
    vaxp_unref(content);  /* Window now owns it */
}

static void on_close_second(VaxpButton* btn, void* data) {
    (void)btn; (void)data;
    
    if (second_window) {
        printf("Closing second window...\n");
        vaxp_window_close(second_window);
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
    VaxpResult init_result = vaxp_init();
    if (!init_result.ok) {
        fprintf(stderr, "Failed to init: %s\n", vaxp_error_string(init_result.error));
        return 1;
    }
    
    VaxpResult wm_result = vaxp_window_manager_init();
    if (!wm_result.ok) {
        fprintf(stderr, "Failed to init window manager: %s\n", vaxp_error_string(wm_result.error));
        return 1;
    }
    
    /* Create main window content */
    VaxpWidget* main_content = vaxp_center(
        .gap = 20,
        .background = VAXP_LIGHT,
        .padding = { 40, 40, 40, 40 },
        .children = VAXP_CHILDREN(
            vaxp_text("Multi-Window Demo"),
            vaxp_btn("Open Second Window", .color = VAXP_PRIMARY, .on_click = on_open_second),
            vaxp_btn("Close Second Window", .color = VAXP_DANGER, .on_click = on_close_second)
        )
    );
    
    /* Create main window */
    VaxpResultPtr win_result = VAXP_WINDOW(
        .title = "VAXPUI - Main Window",
        .width = 500,
        .height = 400,
        .root = main_content
    );
    
    if (!win_result.ok) {
        fprintf(stderr, "Failed to create main window\n");
        return 1;
    }
    
    main_window = (VaxpWindow*)win_result.value;
    vaxp_window_show(main_window);
    vaxp_unref(main_content);
    
    printf("Running... Close main window to exit.\n");
    
    /* Run event loop */
    int result = vaxp_run();
    
    /* Cleanup */
    vaxp_window_manager_shutdown();
    vaxp_shutdown();
    
    printf("Goodbye!\n");
    return result;
}
