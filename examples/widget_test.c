/*
 * VENOMUI Widget Test Application
 * Tests 20 widgets across 3 batches
 * 
 * Features:
 * - Event coalescing (MOUSE_MOVE → single event)
 * - Fixed 60 FPS frame rate
 * - Smooth Spinner animation
 */

#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <unistd.h>
#include <sys/time.h>
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

/* High precision time in microseconds */
static VenomU64 get_time_us(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (VenomU64)tv.tv_sec * 1000000 + (VenomU64)tv.tv_usec;
}

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
 * MAIN - Optimized Event Loop with Coalescing
 * ============================================================================ */
#define TARGET_FPS 144
#define FRAME_TIME_US (1000000 / TARGET_FPS)

int main(int argc, char** argv) {
    printf("╔═══════════════════════════════════════╗\n");
    printf("║  VENOMUI Widget Test (20 widgets)     ║\n");
    printf("║  Optimized: Event Coalescing + 60 FPS ║\n");
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
    
    /* Create event queue for coalescing */
    VenomEventQueue* event_queue = venom_event_queue_create(256);
    if (!event_queue) { printf("Event queue failed\n"); return 1; }
    
    /* Create batch content */
    VenomWidget* content = NULL;
    switch (current_batch) {
        case 1: content = create_batch1(); break;
        case 2: content = create_batch2(); break;
        case 3: content = create_batch3(); break;
    }
    if (!content) { printf("Content failed\n"); return 1; }
    
    printf("\nInteract with widgets. Press ESC to exit.\n");
    printf("Event coalescing + Fixed 60 FPS enabled!\n\n");
    
    /* Layout */
    VenomRectF bounds = {0, 0, (VenomF32)w, (VenomF32)h};
    venom_widget_layout(content, bounds);
    
    /* Optimized event loop */
    VenomBool running = VENOM_TRUE;
    VenomBool needs_redraw = VENOM_TRUE;
    VenomBool continuous_animation = (current_batch == 2);  /* Spinner in Batch 2 */
    
    /* FPS Counter */
    VenomU64 fps_last_time = get_time_us();
    VenomU32 fps_frame_count = 0;
    VenomF32 current_fps = 60.0f;
    
    while (running) {
        VenomU64 frame_start = get_time_us();
        
        /* Calculate FPS every second */
        fps_frame_count++;
        VenomU64 fps_elapsed = frame_start - fps_last_time;
        if (fps_elapsed >= 1000000) {  /* 1 second */
            current_fps = (VenomF32)fps_frame_count * 1000000.0f / (VenomF32)fps_elapsed;
            printf("\r FPS: %.1f    ", current_fps);
            fflush(stdout);
            fps_frame_count = 0;
            fps_last_time = frame_start;
        }
        
        /* 1. Drain ALL pending events into queue */
        VenomEvent raw_event;
        while (venom_display_poll_event(display, &raw_event)) {
            /* Handle close/escape immediately */
            if (raw_event.type == VENOM_EVENT_WINDOW_CLOSE) {
                running = VENOM_FALSE;
            } else if (raw_event.type == VENOM_EVENT_KEY_DOWN && 
                       raw_event.key.key == VENOM_KEY_ESCAPE) {
                running = VENOM_FALSE;
            } else if (raw_event.type == VENOM_EVENT_WINDOW_EXPOSE) {
                needs_redraw = VENOM_TRUE;
            } else {
                /* Push to queue (auto-coalesces MOUSE_MOVE/SCROLL) */
                venom_event_queue_push(event_queue, &raw_event);
            }
        }
        
        /* 2. Flush coalesced events (MOUSE_MOVE, SCROLL) */
        venom_event_queue_flush_coalesced(event_queue);
        
        /* 3. Process all queued events */
        VenomEvent ev;
        while (venom_event_queue_pop(event_queue, &ev)) {
            if (venom_widget_dispatch_event(content, &ev)) {
                needs_redraw = VENOM_TRUE;
            }
        }
        
        /* 4. Render frame if needed or continuous animation */
        if (needs_redraw || continuous_animation) {
            venom_canvas_clear(canvas, venom_color_rgb(240,240,245));
            venom_widget_draw(content, canvas);
            
            /* Draw FPS on screen */
            char fps_text[32];
            snprintf(fps_text, sizeof(fps_text), "FPS: %.1f", current_fps);
            VenomPaint fps_paint = venom_paint_fill((VenomColor){100, 100, 100, 255});
            venom_canvas_draw_text(canvas, fps_text, 10, 20, NULL, &fps_paint);
            
            venom_canvas_flush(canvas);
            needs_redraw = VENOM_FALSE;
        }
        
        /* 5. Frame limiting to 60 FPS */
        VenomU64 frame_end = get_time_us();
        VenomU64 elapsed = frame_end - frame_start;
        if (elapsed < FRAME_TIME_US) {
            usleep((useconds_t)(FRAME_TIME_US - elapsed));
        }
    }
    
    printf("\nCleaning up...\n");
    venom_event_queue_destroy(event_queue);
    venom_unref(content);
    venom_unref(canvas);
    venom_display_close(display);
    venom_shutdown();
    
    printf("Batch %d Complete!\n", current_batch);
    return 0;
}
