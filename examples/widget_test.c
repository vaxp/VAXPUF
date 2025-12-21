/*
 * VENOMUI Widget Test Application
 * Tests 20 widgets across 3 batches
 */

#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <unistd.h>
#include "venom/venomui.h"

/* X11 display handle */
typedef struct {
    VenomDisplay base;
    Display* xdisplay;
    int default_screen;
    Window root_window;
} VenomX11Display;

extern VenomResultPtr venom_canvas_create_for_xlib(Display* display, Window window, 
                                                    Visual* visual, VenomU32 width, VenomU32 height);

static int current_batch = 1;

/* ============================================================================
 * BATCH 1: Layout & Basic Widgets (7 widgets)
 * ============================================================================ */
static VenomWidget* create_batch1(void) {
    printf("\n=== BATCH 1: Layout & Basic ===\n");
    
    VenomResultPtr result = venom_container_create_column();
    if (!result.ok) return NULL;
    VenomContainer* c = (VenomContainer*)result.value;
    venom_container_set_gap(c, 12);
    venom_container_set_background(c, venom_color_rgb(245, 248, 255));
    ((VenomWidget*)c)->layout.padding = (VenomInsets){16,16,16,16};
    
    /* 1. Label */
    printf("  [1] VenomLabel\n");
    VenomResultPtr lr = venom_label_create("Batch 1: Layout & Basic");
    if (lr.ok) { venom_label_set_font_size((VenomLabel*)lr.value, 20); venom_widget_add_child((VenomWidget*)c, lr.value); }
    
    /* 2. Button */
    printf("  [2] VenomButton\n");
    VenomResultPtr br = venom_button_create("Click Me!");
    if (br.ok) { venom_widget_add_child((VenomWidget*)c, br.value); }
    
    /* 3. Checkbox */
    printf("  [3] VenomCheckbox\n");
    VenomResultPtr cbr = venom_checkbox_create();
    if (cbr.ok) { venom_checkbox_set_label((VenomCheckbox*)cbr.value, "Accept Terms"); }
    if (cbr.ok) { venom_widget_add_child((VenomWidget*)c, cbr.value); }
    
    /* 4. Slider */
    printf("  [4] VenomSlider\n");
    VenomResultPtr sr = venom_slider_create();
    if (sr.ok) { venom_slider_set_value((VenomSlider*)sr.value, 50); venom_widget_add_child((VenomWidget*)c, sr.value); }
    
    /* 5. Divider */
    printf("  [5] VenomDivider\n");
    VenomResultPtr dr = venom_divider_create();
    if (dr.ok) { venom_widget_add_child((VenomWidget*)c, dr.value); }
    
    /* 6. Progress */
    printf("  [6] VenomProgressBar\n");
    VenomResultPtr pr = venom_progress_create();
    if (pr.ok) { venom_progress_set_value((VenomProgressBar*)pr.value, 0.65f); venom_widget_add_child((VenomWidget*)c, pr.value); }
    
    /* 7. Switch */
    printf("  [7] VenomSwitch\n");
    VenomResultPtr swr = venom_switch_create();
    if (swr.ok) { venom_widget_add_child((VenomWidget*)c, swr.value); }
    
    printf("  Batch 1 Ready!\n");
    return (VenomWidget*)c;
}

/* ============================================================================
 * BATCH 2: Display Widgets (7 widgets)
 * ============================================================================ */
static VenomWidget* create_batch2(void) {
    printf("\n=== BATCH 2: Display ===\n");
    
    VenomResultPtr result = venom_container_create_column();
    if (!result.ok) return NULL;
    VenomContainer* c = (VenomContainer*)result.value;
    venom_container_set_gap(c, 12);
    venom_container_set_background(c, venom_color_rgb(240, 248, 245));
    ((VenomWidget*)c)->layout.padding = (VenomInsets){16,16,16,16};
    
    printf("  [8] VenomLabel\n");
    VenomResultPtr lr = venom_label_create("Batch 2: Display Widgets");
    if (lr.ok) { venom_label_set_font_size((VenomLabel*)lr.value, 20); venom_widget_add_child((VenomWidget*)c, lr.value); }
    
    printf("  [9] VenomIcon\n");
    VenomResultPtr ir = venom_icon_create();
    if (ir.ok) { venom_icon_set_icon((VenomIcon*)ir.value, "⭐"); venom_widget_add_child((VenomWidget*)c, ir.value); }
    
    printf("  [10] VenomBadge\n");
    VenomResultPtr bgr = venom_badge_create();
    if (bgr.ok) { venom_badge_set_count((VenomBadge*)bgr.value, 5); venom_widget_add_child((VenomWidget*)c, bgr.value); }
    
    printf("  [11] VenomChip\n");
    VenomResultPtr chr = venom_chip_create();
    if (chr.ok) { venom_chip_set_label((VenomChip*)chr.value, "Technology"); venom_widget_add_child((VenomWidget*)c, chr.value); }
    
    printf("  [12] VenomRating\n");
    VenomResultPtr rr = venom_rating_create();
    if (rr.ok) { venom_rating_set_value((VenomRating*)rr.value, 3.5f); venom_widget_add_child((VenomWidget*)c, rr.value); }
    
    printf("  [13] VenomSkeleton\n");
    VenomResultPtr skr = venom_skeleton_create();
    if (skr.ok) { venom_widget_add_child((VenomWidget*)c, skr.value); }
    
    printf("  [14] VenomSpinner\n");
    VenomResultPtr spr = venom_spinner_create();
    if (spr.ok) { venom_widget_add_child((VenomWidget*)c, spr.value); }
    
    printf("  Batch 2 Ready!\n");
    return (VenomWidget*)c;
}

/* ============================================================================
 * BATCH 3: Pickers (6 widgets)
 * ============================================================================ */
static VenomWidget* create_batch3(void) {
    printf("\n=== BATCH 3: Pickers ===\n");
    
    VenomResultPtr result = venom_container_create_column();
    if (!result.ok) return NULL;
    VenomContainer* c = (VenomContainer*)result.value;
    venom_container_set_gap(c, 12);
    venom_container_set_background(c, venom_color_rgb(255, 248, 240));
    ((VenomWidget*)c)->layout.padding = (VenomInsets){16,16,16,16};
    
    printf("  [15] VenomLabel\n");
    VenomResultPtr lr = venom_label_create("Batch 3: Pickers");
    if (lr.ok) { venom_label_set_font_size((VenomLabel*)lr.value, 20); venom_widget_add_child((VenomWidget*)c, lr.value); }
    
    printf("  [16] VenomSearchBar\n");
    VenomResultPtr sr = venom_search_bar_create();
    if (sr.ok) { venom_widget_add_child((VenomWidget*)c, sr.value); }
    
    printf("  [17] VenomNumberInput\n");
    VenomResultPtr nr = venom_number_input_create();
    if (nr.ok) { venom_widget_add_child((VenomWidget*)c, nr.value); }
    
    printf("  [18] VenomCalendar\n");
    VenomResultPtr cr = venom_calendar_create();
    if (cr.ok) { venom_widget_add_child((VenomWidget*)c, cr.value); }
    
    printf("  [19] VenomColorPicker\n");
    VenomResultPtr cpr = venom_color_picker_create();
    if (cpr.ok) { venom_widget_add_child((VenomWidget*)c, cpr.value); }
    
    printf("  [20] VenomToggleButton\n");
    VenomResultPtr tbr = venom_toggle_button_create();
    if (tbr.ok) { venom_toggle_button_set_label((VenomToggleButton*)tbr.value, "Toggle"); venom_widget_add_child((VenomWidget*)c, tbr.value); }
    
    printf("  Batch 3 Ready!\n");
    return (VenomWidget*)c;
}

/* ============================================================================
 * MAIN
 * ============================================================================ */
int main(int argc, char** argv) {
    printf("╔═══════════════════════════════════════╗\n");
    printf("║  VENOMUI Widget Test (20 widgets)     ║\n");
    printf("╚═══════════════════════════════════════╝\n");
    
    if (argc > 1) {
        current_batch = atoi(argv[1]);
        if (current_batch < 1 || current_batch > 3) {
            printf("Usage: %s [1|2|3]\n", argv[0]);
            return 1;
        }
    }
    
    printf("Testing Batch %d...\n", current_batch);
    
    /* Init */
    VenomResult init = venom_init();
    if (!init.ok) { printf("Init failed\n"); return 1; }
    
    /* Open display */
    VenomResultPtr disp = venom_display_open(VENOM_BACKEND_X11, NULL);
    if (!disp.ok) { printf("Display failed\n"); return 1; }
    VenomDisplay* display = (VenomDisplay*)disp.value;
    VenomX11Display* x11 = (VenomX11Display*)display;
    
    /* Create window */
    VenomU32 w = 450, h = 650;
    VenomResultPtr win = display->ops->create_window(display, "Widget Test", 100, 100, w, h);
    if (!win.ok) { printf("Window failed\n"); return 1; }
    Window xwin = (Window)(uintptr_t)win.value;
    
    /* Create canvas */
    Visual* vis = DefaultVisual(x11->xdisplay, x11->default_screen);
    VenomResultPtr cvs = venom_canvas_create_for_xlib(x11->xdisplay, xwin, vis, w, h);
    if (!cvs.ok) { printf("Canvas failed\n"); return 1; }
    VenomCanvas* canvas = (VenomCanvas*)cvs.value;
    
    /* Create batch content */
    VenomWidget* content = NULL;
    switch (current_batch) {
        case 1: content = create_batch1(); break;
        case 2: content = create_batch2(); break;
        case 3: content = create_batch3(); break;
    }
    if (!content) { printf("Content failed\n"); return 1; }
    
    printf("\nInteract with widgets. Press ESC to exit.\n\n");
    
    /* Layout */
    VenomRectF bounds = {0, 0, (VenomF32)w, (VenomF32)h};
    venom_widget_layout(content, bounds);
    
    /* Event loop with continuous animation for Batch 2 (Spinner) */
    VenomBool running = VENOM_TRUE;
    VenomBool redraw = VENOM_TRUE;
    VenomBool continuous_animation = (current_batch == 2);  /* Batch 2 has Spinner */
    
    while (running) {
        /* Always redraw for continuous animation (Spinner), otherwise only on demand */
        if (redraw || continuous_animation) {
            venom_canvas_clear(canvas, venom_color_rgb(240,240,245));
            venom_widget_draw(content, canvas);
            venom_canvas_flush(canvas);
            redraw = VENOM_FALSE;
        }
        
        VenomEvent ev;
        if (venom_display_poll_event(display, &ev)) {
            switch (ev.type) {
                case VENOM_EVENT_WINDOW_CLOSE:
                    running = VENOM_FALSE;
                    break;
                case VENOM_EVENT_KEY_DOWN:
                    if (ev.key.key == VENOM_KEY_ESCAPE) running = VENOM_FALSE;
                    break;
                case VENOM_EVENT_WINDOW_EXPOSE:
                    redraw = VENOM_TRUE;
                    break;
                default:
                    break;
            }
            if (venom_widget_dispatch_event(content, &ev)) redraw = VENOM_TRUE;
        }
        
        /* Frame limiting: 60 FPS */
        usleep(16667);
    }
    
    printf("\nCleaning up...\n");
    venom_unref(content);
    venom_unref(canvas);
    venom_display_close(display);
    venom_shutdown();
    
    printf("Batch %d Complete!\n", current_batch);
    return 0;
}
