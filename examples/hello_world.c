/*
 * VAXPUI - Hello World Example
 * 
 * Demonstrates basic window creation, widgets, and event handling.
 */

#include <stdio.h>
#include <X11/Xlib.h>
#include <vaxp/vaxpui.h>

/* We need access to X11 internals for canvas creation */
/* This is a temporary solution - will be abstracted later */
extern VaxpResultPtr vaxp_canvas_create_opengl(Display* display, Window window, 
                                               VaxpU32 width, VaxpU32 height);

/* X11 display handle - from our display abstraction */
typedef struct {
    VaxpDisplay base;
    Display* xdisplay;
    int default_screen;
    Window root_window;
    /* ... other fields ... */
} VaxpX11Display;

/* Button click callback */
static void on_button_click(VaxpButton* button, void* user_data) {
    (void)user_data;
    printf("Button '%s' clicked!\n", vaxp_button_get_label(button));
}

int main(void) {
    /* Initialize VAXPUI */
    VaxpResult init_result = vaxp_init();
    if (!init_result.ok) {
        fprintf(stderr, "Failed to initialize VAXPUI: %s\n", 
                vaxp_error_string(init_result.error));
        return 1;
    }
    
    /* Open display */
    VaxpResultPtr display_result = vaxp_display_open(VAXP_BACKEND_X11, NULL);
    if (!display_result.ok) {
        fprintf(stderr, "Failed to open display: %s\n", 
                vaxp_error_string(display_result.error));
        return 1;
    }
    VaxpDisplay* display = (VaxpDisplay*)display_result.value;
    VaxpX11Display* x11display = (VaxpX11Display*)display;
    
    /* Create window via display */
    VaxpU32 win_width = 500;
    VaxpU32 win_height = 350;
    VaxpResultPtr window_result = display->ops->create_window(
        display, "VAXPUI - Hello World",
        100, 100, win_width, win_height
    );
    if (!window_result.ok) {
        fprintf(stderr, "Failed to create window: %s\n", 
                vaxp_error_string(window_result.error));
        vaxp_display_close(display);
        return 1;
    }
    Window xwindow = (Window)(uintptr_t)window_result.value;
    
    /* Create Cairo canvas for the window */
    VaxpResultPtr canvas_result = vaxp_canvas_create_opengl(
        x11display->xdisplay, xwindow, win_width, win_height
    );
    if (!canvas_result.ok) {
        fprintf(stderr, "Failed to create canvas: %s\n",
                vaxp_error_string(canvas_result.error));
        vaxp_display_close(display);
        return 1;
    }
    VaxpCanvas* canvas = (VaxpCanvas*)canvas_result.value;
    
    /* Create main container */
    VaxpResultPtr container_result = vaxp_container_create_column();
    if (!container_result.ok) {
        vaxp_unref(canvas);
        vaxp_display_close(display);
        return 1;
    }
    VaxpContainer* container = (VaxpContainer*)container_result.value;
    
    /* Style the container */
    vaxp_container_set_gap(container, 20);
    vaxp_container_set_justify(container, VAXP_JUSTIFY_CENTER);
    vaxp_container_set_align(container, VAXP_ALIGN_CENTER);
    vaxp_container_set_background(container, vaxp_color_rgb(245, 248, 255));
    ((VaxpWidget*)container)->layout.padding = (VaxpInsets){ 30, 30, 30, 30 };
    
    /* Create title label */
    VaxpResultPtr label_result = vaxp_label_create("Welcome to VAXPUI!");
    if (label_result.ok) {
        VaxpLabel* label = (VaxpLabel*)label_result.value;
        vaxp_label_set_font_size(label, 24.0f);
        vaxp_label_set_color(label, vaxp_color_rgb(30, 30, 80));
        vaxp_widget_add_child((VaxpWidget*)container, (VaxpWidget*)label);
        vaxp_unref(label);
    }
    
    /* Create subtitle */
    VaxpResultPtr subtitle_result = vaxp_label_create("A high performance GUI framework in C");
    if (subtitle_result.ok) {
        VaxpLabel* subtitle = (VaxpLabel*)subtitle_result.value;
        vaxp_label_set_color(subtitle, vaxp_color_rgb(100, 100, 120));
        vaxp_widget_add_child((VaxpWidget*)container, (VaxpWidget*)subtitle);
        vaxp_unref(subtitle);
    }
    
    /* Create button row */
    VaxpResultPtr row_result = vaxp_container_create_row();
    if (row_result.ok) {
        VaxpContainer* row = (VaxpContainer*)row_result.value;
        vaxp_container_set_gap(row, 15);
        
        /* Create buttons */
        const char* button_labels[] = { "Primary", "Success", "Danger" };
        VaxpColor colors[][3] = {
            { {60, 120, 220, 255}, {80, 140, 240, 255}, {40, 100, 200, 255} },  /* Blue */
            { {60, 180, 100, 255}, {80, 200, 120, 255}, {40, 160, 80, 255} },   /* Green */
            { {220, 70, 70, 255}, {240, 90, 90, 255}, {200, 50, 50, 255} },     /* Red */
        };
        
        for (int i = 0; i < 3; i++) {
            VaxpResultPtr btn_result = vaxp_button_create(button_labels[i]);
            if (btn_result.ok) {
                VaxpButton* btn = (VaxpButton*)btn_result.value;
                vaxp_button_set_colors(btn, colors[i][0], colors[i][1], 
                                        colors[i][2], VAXP_COLOR_WHITE);
                vaxp_button_set_corner_radius(btn, 8.0f);
                vaxp_button_set_on_click(btn, on_button_click, NULL);
                vaxp_widget_add_child((VaxpWidget*)row, (VaxpWidget*)btn);
                vaxp_unref(btn);
            }
        }
        
        vaxp_widget_add_child((VaxpWidget*)container, (VaxpWidget*)row);
        vaxp_unref(row);
    }
    
    printf("VAXPUI Hello World Example\n");
    printf("Close the window or press ESC to exit.\n");
    
    /* Initial layout */
    VaxpRectF root_bounds = { 0, 0, (VaxpF32)win_width, (VaxpF32)win_height };
    vaxp_widget_layout((VaxpWidget*)container, root_bounds);
    
    /* Event loop */
    VaxpBool running = VAXP_TRUE;
    VaxpBool needs_redraw = VAXP_TRUE;
    
    while (running) {
        VaxpEvent event;
        
        /* Draw if needed */
        if (needs_redraw) {
            vaxp_canvas_clear(canvas, vaxp_color_rgb(245, 248, 255));
            vaxp_widget_draw((VaxpWidget*)container, canvas);
            vaxp_canvas_flush(canvas);
            needs_redraw = VAXP_FALSE;
        }
        
        if (vaxp_display_poll_event(display, &event)) {
            switch (event.type) {
                case VAXP_EVENT_WINDOW_CLOSE:
                    printf("Window close requested.\n");
                    running = VAXP_FALSE;
                    break;
                    
                case VAXP_EVENT_KEY_DOWN:
                    if (event.key.key == VAXP_KEY_ESCAPE) {
                        running = VAXP_FALSE;
                    }
                    break;
                    
                case VAXP_EVENT_WINDOW_EXPOSE:
                    needs_redraw = VAXP_TRUE;
                    break;
                    
                default:
                    break;
            }
            
            /* Dispatch to widget tree */
            if (vaxp_widget_dispatch_event((VaxpWidget*)container, &event)) {
                needs_redraw = VAXP_TRUE;
            }
        }
        
        /* Small sleep to avoid busy wait */
        usleep(16000);  /* ~60fps */
    }
    
    /* Cleanup */
    vaxp_unref(container);
    vaxp_unref(canvas);
    vaxp_display_close(display);
    vaxp_shutdown();
    
    printf("Goodbye!\n");
    return 0;
}
