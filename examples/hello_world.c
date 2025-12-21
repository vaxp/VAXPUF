/*
 * VENOMUI - Hello World Example
 * 
 * Demonstrates basic window creation, widgets, and event handling.
 */

#include <stdio.h>
#include <X11/Xlib.h>
#include <venom/venomui.h>

/* We need access to X11 internals for canvas creation */
/* This is a temporary solution - will be abstracted later */
extern VenomResultPtr venom_canvas_create_for_xlib(Display* display, Window window, 
                                                    Visual* visual, VenomU32 width, VenomU32 height);

/* X11 display handle - from our display abstraction */
typedef struct {
    VenomDisplay base;
    Display* xdisplay;
    int default_screen;
    Window root_window;
    /* ... other fields ... */
} VenomX11Display;

/* Button click callback */
static void on_button_click(VenomButton* button, void* user_data) {
    (void)user_data;
    printf("Button '%s' clicked!\n", venom_button_get_label(button));
}

int main(void) {
    /* Initialize VENOMUI */
    VenomResult init_result = venom_init();
    if (!init_result.ok) {
        fprintf(stderr, "Failed to initialize VENOMUI: %s\n", 
                venom_error_string(init_result.error));
        return 1;
    }
    
    /* Open display */
    VenomResultPtr display_result = venom_display_open(VENOM_BACKEND_X11, NULL);
    if (!display_result.ok) {
        fprintf(stderr, "Failed to open display: %s\n", 
                venom_error_string(display_result.error));
        return 1;
    }
    VenomDisplay* display = (VenomDisplay*)display_result.value;
    VenomX11Display* x11display = (VenomX11Display*)display;
    
    /* Create window via display */
    VenomU32 win_width = 500;
    VenomU32 win_height = 350;
    VenomResultPtr window_result = display->ops->create_window(
        display, "VENOMUI - Hello World",
        100, 100, win_width, win_height
    );
    if (!window_result.ok) {
        fprintf(stderr, "Failed to create window: %s\n", 
                venom_error_string(window_result.error));
        venom_display_close(display);
        return 1;
    }
    Window xwindow = (Window)(uintptr_t)window_result.value;
    
    /* Create Cairo canvas for the window */
    Visual* visual = DefaultVisual(x11display->xdisplay, x11display->default_screen);
    VenomResultPtr canvas_result = venom_canvas_create_for_xlib(
        x11display->xdisplay, xwindow, visual, win_width, win_height
    );
    if (!canvas_result.ok) {
        fprintf(stderr, "Failed to create canvas: %s\n",
                venom_error_string(canvas_result.error));
        venom_display_close(display);
        return 1;
    }
    VenomCanvas* canvas = (VenomCanvas*)canvas_result.value;
    
    /* Create main container */
    VenomResultPtr container_result = venom_container_create_column();
    if (!container_result.ok) {
        venom_unref(canvas);
        venom_display_close(display);
        return 1;
    }
    VenomContainer* container = (VenomContainer*)container_result.value;
    
    /* Style the container */
    venom_container_set_gap(container, 20);
    venom_container_set_justify(container, VENOM_JUSTIFY_CENTER);
    venom_container_set_align(container, VENOM_ALIGN_CENTER);
    venom_container_set_background(container, venom_color_rgb(245, 248, 255));
    ((VenomWidget*)container)->layout.padding = (VenomInsets){ 30, 30, 30, 30 };
    
    /* Create title label */
    VenomResultPtr label_result = venom_label_create("Welcome to VENOMUI!");
    if (label_result.ok) {
        VenomLabel* label = (VenomLabel*)label_result.value;
        venom_label_set_font_size(label, 24.0f);
        venom_label_set_color(label, venom_color_rgb(30, 30, 80));
        venom_widget_add_child((VenomWidget*)container, (VenomWidget*)label);
        venom_unref(label);
    }
    
    /* Create subtitle */
    VenomResultPtr subtitle_result = venom_label_create("A high performance GUI framework in C");
    if (subtitle_result.ok) {
        VenomLabel* subtitle = (VenomLabel*)subtitle_result.value;
        venom_label_set_color(subtitle, venom_color_rgb(100, 100, 120));
        venom_widget_add_child((VenomWidget*)container, (VenomWidget*)subtitle);
        venom_unref(subtitle);
    }
    
    /* Create button row */
    VenomResultPtr row_result = venom_container_create_row();
    if (row_result.ok) {
        VenomContainer* row = (VenomContainer*)row_result.value;
        venom_container_set_gap(row, 15);
        
        /* Create buttons */
        const char* button_labels[] = { "Primary", "Success", "Danger" };
        VenomColor colors[][3] = {
            { {60, 120, 220, 255}, {80, 140, 240, 255}, {40, 100, 200, 255} },  /* Blue */
            { {60, 180, 100, 255}, {80, 200, 120, 255}, {40, 160, 80, 255} },   /* Green */
            { {220, 70, 70, 255}, {240, 90, 90, 255}, {200, 50, 50, 255} },     /* Red */
        };
        
        for (int i = 0; i < 3; i++) {
            VenomResultPtr btn_result = venom_button_create(button_labels[i]);
            if (btn_result.ok) {
                VenomButton* btn = (VenomButton*)btn_result.value;
                venom_button_set_colors(btn, colors[i][0], colors[i][1], 
                                        colors[i][2], VENOM_COLOR_WHITE);
                venom_button_set_corner_radius(btn, 8.0f);
                venom_button_set_on_click(btn, on_button_click, NULL);
                venom_widget_add_child((VenomWidget*)row, (VenomWidget*)btn);
                venom_unref(btn);
            }
        }
        
        venom_widget_add_child((VenomWidget*)container, (VenomWidget*)row);
        venom_unref(row);
    }
    
    printf("VENOMUI Hello World Example\n");
    printf("Close the window or press ESC to exit.\n");
    
    /* Initial layout */
    VenomRectF root_bounds = { 0, 0, (VenomF32)win_width, (VenomF32)win_height };
    venom_widget_layout((VenomWidget*)container, root_bounds);
    
    /* Event loop */
    VenomBool running = VENOM_TRUE;
    VenomBool needs_redraw = VENOM_TRUE;
    
    while (running) {
        VenomEvent event;
        
        /* Draw if needed */
        if (needs_redraw) {
            venom_canvas_clear(canvas, venom_color_rgb(245, 248, 255));
            venom_widget_draw((VenomWidget*)container, canvas);
            venom_canvas_flush(canvas);
            needs_redraw = VENOM_FALSE;
        }
        
        if (venom_display_poll_event(display, &event)) {
            switch (event.type) {
                case VENOM_EVENT_WINDOW_CLOSE:
                    printf("Window close requested.\n");
                    running = VENOM_FALSE;
                    break;
                    
                case VENOM_EVENT_KEY_DOWN:
                    if (event.key.key == VENOM_KEY_ESCAPE) {
                        running = VENOM_FALSE;
                    }
                    break;
                    
                case VENOM_EVENT_WINDOW_EXPOSE:
                    needs_redraw = VENOM_TRUE;
                    break;
                    
                default:
                    break;
            }
            
            /* Dispatch to widget tree */
            if (venom_widget_dispatch_event((VenomWidget*)container, &event)) {
                needs_redraw = VENOM_TRUE;
            }
        }
        
        /* Small sleep to avoid busy wait */
        usleep(16000);  /* ~60fps */
    }
    
    /* Cleanup */
    venom_unref(container);
    venom_unref(canvas);
    venom_display_close(display);
    venom_shutdown();
    
    printf("Goodbye!\n");
    return 0;
}
