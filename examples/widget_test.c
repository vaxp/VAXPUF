/*
 * VAXPUI Widget Test Application
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
#include "vaxp/vaxpui.h"

/* X11 display handle */
typedef struct {
    VaxpDisplay base;
    Display* xdisplay;
    int default_screen;
    Window root_window;
} VaxpX11Display;

extern VaxpResultPtr vaxp_canvas_create_for_xlib(Display* display, Window window, 
                                                    Visual* visual, VaxpU32 width, VaxpU32 height);

static int current_batch = 1;

/* High precision time in microseconds */
static VaxpU64 get_time_us(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (VaxpU64)tv.tv_sec * 1000000 + (VaxpU64)tv.tv_usec;
}

/* ============================================================================
 * BATCH 1: Layout & Basic Widgets (7 widgets)
 * ============================================================================ */
static VaxpWidget* create_batch1(void) {
    printf("\n=== BATCH 1: Layout & Basic ===\n");
    
    VaxpResultPtr result = vaxp_container_create_column();
    if (!result.ok) return NULL;
    VaxpContainer* c = (VaxpContainer*)result.value;
    vaxp_container_set_gap(c, 12);
    vaxp_container_set_background(c, vaxp_color_rgb(245, 248, 255));
    ((VaxpWidget*)c)->layout.padding = (VaxpInsets){16,16,16,16};
    
    /* 1. Label */
    printf("  [1] VaxpLabel\n");
    VaxpResultPtr lr = vaxp_label_create("Batch 1: Layout & Basic");
    if (lr.ok) { vaxp_label_set_font_size((VaxpLabel*)lr.value, 20); vaxp_widget_add_child((VaxpWidget*)c, lr.value); }
    
    /* 2. Button */
    printf("  [2] VaxpButton\n");
    VaxpResultPtr br = vaxp_button_create("Click Me!");
    if (br.ok) { vaxp_widget_add_child((VaxpWidget*)c, br.value); }
    
    /* 3. Checkbox */
    printf("  [3] VaxpCheckbox\n");
    VaxpResultPtr cbr = vaxp_checkbox_create();
    if (cbr.ok) { vaxp_checkbox_set_label((VaxpCheckbox*)cbr.value, "Accept Terms"); }
    if (cbr.ok) { vaxp_widget_add_child((VaxpWidget*)c, cbr.value); }
    
    /* 4. Slider */
    printf("  [4] VaxpSlider\n");
    VaxpResultPtr sr = vaxp_slider_create();
    if (sr.ok) { vaxp_slider_set_value((VaxpSlider*)sr.value, 50); vaxp_widget_add_child((VaxpWidget*)c, sr.value); }
    
    /* 5. Divider */
    printf("  [5] VaxpDivider\n");
    VaxpResultPtr dr = vaxp_divider_create();
    if (dr.ok) { vaxp_widget_add_child((VaxpWidget*)c, dr.value); }
    
    /* 6. Progress */
    printf("  [6] VaxpProgressBar\n");
    VaxpResultPtr pr = vaxp_progress_create();
    if (pr.ok) { vaxp_progress_set_value((VaxpProgressBar*)pr.value, 0.65f); vaxp_widget_add_child((VaxpWidget*)c, pr.value); }
    
    /* 7. Switch */
    printf("  [7] VaxpSwitch\n");
    VaxpResultPtr swr = vaxp_switch_create();
    if (swr.ok) { vaxp_widget_add_child((VaxpWidget*)c, swr.value); }
    
    printf("  Batch 1 Ready!\n");
    return (VaxpWidget*)c;
}

/* ============================================================================
 * BATCH 2: Display Widgets (7 widgets)
 * ============================================================================ */
static VaxpWidget* create_batch2(void) {
    printf("\n=== BATCH 2: Display ===\n");
    
    VaxpResultPtr result = vaxp_container_create_column();
    if (!result.ok) return NULL;
    VaxpContainer* c = (VaxpContainer*)result.value;
    vaxp_container_set_gap(c, 12);
    vaxp_container_set_background(c, vaxp_color_rgb(240, 248, 245));
    ((VaxpWidget*)c)->layout.padding = (VaxpInsets){16,16,16,16};
    
    printf("  [8] VaxpLabel\n");
    VaxpResultPtr lr = vaxp_label_create("Batch 2: Display Widgets");
    if (lr.ok) { vaxp_label_set_font_size((VaxpLabel*)lr.value, 20); vaxp_widget_add_child((VaxpWidget*)c, lr.value); }
    
    printf("  [9] VaxpIcon\n");
    VaxpResultPtr ir = vaxp_icon_create();
    if (ir.ok) { vaxp_icon_set_icon((VaxpIcon*)ir.value, "⭐"); vaxp_widget_add_child((VaxpWidget*)c, ir.value); }
    
    printf("  [10] VaxpBadge\n");
    VaxpResultPtr bgr = vaxp_badge_create();
    if (bgr.ok) { vaxp_badge_set_count((VaxpBadge*)bgr.value, 5); vaxp_widget_add_child((VaxpWidget*)c, bgr.value); }
    
    printf("  [11] VaxpChip\n");
    VaxpResultPtr chr = vaxp_chip_create();
    if (chr.ok) { vaxp_chip_set_label((VaxpChip*)chr.value, "Technology"); vaxp_widget_add_child((VaxpWidget*)c, chr.value); }
    
    printf("  [12] VaxpRating\n");
    VaxpResultPtr rr = vaxp_rating_create();
    if (rr.ok) { vaxp_rating_set_value((VaxpRating*)rr.value, 3.5f); vaxp_widget_add_child((VaxpWidget*)c, rr.value); }
    
    printf("  [13] VaxpSkeleton\n");
    VaxpResultPtr skr = vaxp_skeleton_create();
    if (skr.ok) { vaxp_widget_add_child((VaxpWidget*)c, skr.value); }
    
    printf("  [14] VaxpSpinner\n");
    VaxpResultPtr spr = vaxp_spinner_create();
    if (spr.ok) { vaxp_widget_add_child((VaxpWidget*)c, spr.value); }
    
    printf("  Batch 2 Ready!\n");
    return (VaxpWidget*)c;
}

/* ============================================================================
 * BATCH 3: Pickers (6 widgets)
 * ============================================================================ */
static VaxpWidget* create_batch3(void) {
    printf("\n=== BATCH 3: Pickers ===\n");
    
    VaxpResultPtr result = vaxp_container_create_column();
    if (!result.ok) return NULL;
    VaxpContainer* c = (VaxpContainer*)result.value;
    vaxp_container_set_gap(c, 12);
    vaxp_container_set_background(c, vaxp_color_rgb(255, 248, 240));
    ((VaxpWidget*)c)->layout.padding = (VaxpInsets){16,16,16,16};
    
    printf("  [15] VaxpLabel\n");
    VaxpResultPtr lr = vaxp_label_create("Batch 3: Pickers");
    if (lr.ok) { vaxp_label_set_font_size((VaxpLabel*)lr.value, 20); vaxp_widget_add_child((VaxpWidget*)c, lr.value); }
    
    printf("  [16] VaxpSearchBar\n");
    VaxpResultPtr sr = vaxp_search_bar_create();
    if (sr.ok) { vaxp_widget_add_child((VaxpWidget*)c, sr.value); }
    
    printf("  [17] VaxpNumberInput\n");
    VaxpResultPtr nr = vaxp_number_input_create();
    if (nr.ok) { vaxp_widget_add_child((VaxpWidget*)c, nr.value); }
    
    printf("  [18] VaxpCalendar\n");
    VaxpResultPtr cr = vaxp_calendar_create();
    if (cr.ok) { vaxp_widget_add_child((VaxpWidget*)c, cr.value); }
    
    printf("  [19] VaxpColorPicker\n");
    VaxpResultPtr cpr = vaxp_color_picker_create();
    if (cpr.ok) { vaxp_widget_add_child((VaxpWidget*)c, cpr.value); }
    
    printf("  [20] VaxpToggleButton\n");
    VaxpResultPtr tbr = vaxp_toggle_button_create();
    if (tbr.ok) { vaxp_toggle_button_set_label((VaxpToggleButton*)tbr.value, "Toggle"); vaxp_widget_add_child((VaxpWidget*)c, tbr.value); }
    
    printf("  Batch 3 Ready!\n");
    return (VaxpWidget*)c;
}

/* ============================================================================
 * MAIN - Optimized Event Loop with Coalescing
 * ============================================================================ */
#define TARGET_FPS 144
#define FRAME_TIME_US (1000000 / TARGET_FPS)

int main(int argc, char** argv) {
    printf("╔═══════════════════════════════════════╗\n");
    printf("║  VAXPUI Widget Test (20 widgets)     ║\n");
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
    VaxpResult init = vaxp_init();
    if (!init.ok) { printf("Init failed\n"); return 1; }
    
    /* Open display */
    VaxpResultPtr disp = vaxp_display_open(VAXP_BACKEND_X11, NULL);
    if (!disp.ok) { printf("Display failed\n"); return 1; }
    VaxpDisplay* display = (VaxpDisplay*)disp.value;
    VaxpX11Display* x11 = (VaxpX11Display*)display;
    
    /* Create window */
    VaxpU32 w = 450, h = 650;
    VaxpResultPtr win = display->ops->create_window(display, "Widget Test", 100, 100, w, h);
    if (!win.ok) { printf("Window failed\n"); return 1; }
    Window xwin = (Window)(uintptr_t)win.value;
    
    /* Create canvas */
    Visual* vis = DefaultVisual(x11->xdisplay, x11->default_screen);
    VaxpResultPtr cvs = vaxp_canvas_create_for_xlib(x11->xdisplay, xwin, vis, w, h);
    if (!cvs.ok) { printf("Canvas failed\n"); return 1; }
    VaxpCanvas* canvas = (VaxpCanvas*)cvs.value;
    
    /* Create event queue for coalescing */
    VaxpEventQueue* event_queue = vaxp_event_queue_create(256);
    if (!event_queue) { printf("Event queue failed\n"); return 1; }
    
    /* Create batch content */
    VaxpWidget* content = NULL;
    switch (current_batch) {
        case 1: content = create_batch1(); break;
        case 2: content = create_batch2(); break;
        case 3: content = create_batch3(); break;
    }
    if (!content) { printf("Content failed\n"); return 1; }
    
    printf("\nInteract with widgets. Press ESC to exit.\n");
    printf("Event coalescing + Fixed 60 FPS enabled!\n\n");
    
    /* Layout */
    VaxpRectF bounds = {0, 0, (VaxpF32)w, (VaxpF32)h};
    vaxp_widget_layout(content, bounds);
    
    /* Optimized event loop */
    VaxpBool running = VAXP_TRUE;
    VaxpBool needs_redraw = VAXP_TRUE;
    VaxpBool continuous_animation = (current_batch == 2);  /* Spinner in Batch 2 */
    
    /* FPS Counter */
    VaxpU64 fps_last_time = get_time_us();
    VaxpU32 fps_frame_count = 0;
    VaxpF32 current_fps = 60.0f;
    
    while (running) {
        VaxpU64 frame_start = get_time_us();
        
        /* Calculate FPS every second */
        fps_frame_count++;
        VaxpU64 fps_elapsed = frame_start - fps_last_time;
        if (fps_elapsed >= 1000000) {  /* 1 second */
            current_fps = (VaxpF32)fps_frame_count * 1000000.0f / (VaxpF32)fps_elapsed;
            printf("\r FPS: %.1f    ", current_fps);
            fflush(stdout);
            fps_frame_count = 0;
            fps_last_time = frame_start;
        }
        
        /* 1. Drain ALL pending events into queue */
        VaxpEvent raw_event;
        while (vaxp_display_poll_event(display, &raw_event)) {
            /* Handle close/escape immediately */
            if (raw_event.type == VAXP_EVENT_WINDOW_CLOSE) {
                running = VAXP_FALSE;
            } else if (raw_event.type == VAXP_EVENT_KEY_DOWN && 
                       raw_event.key.key == VAXP_KEY_ESCAPE) {
                running = VAXP_FALSE;
            } else if (raw_event.type == VAXP_EVENT_WINDOW_EXPOSE) {
                needs_redraw = VAXP_TRUE;
            } else {
                /* Push to queue (auto-coalesces MOUSE_MOVE/SCROLL) */
                vaxp_event_queue_push(event_queue, &raw_event);
            }
        }
        
        /* 2. Flush coalesced events (MOUSE_MOVE, SCROLL) */
        vaxp_event_queue_flush_coalesced(event_queue);
        
        /* 3. Process all queued events */
        VaxpEvent ev;
        while (vaxp_event_queue_pop(event_queue, &ev)) {
            if (vaxp_widget_dispatch_event(content, &ev)) {
                needs_redraw = VAXP_TRUE;
            }
        }
        
        /* 4. Render frame if needed or continuous animation */
        if (needs_redraw || continuous_animation) {
            vaxp_canvas_clear(canvas, vaxp_color_rgb(240,240,245));
            vaxp_widget_draw(content, canvas);
            
            /* Draw FPS on screen */
            char fps_text[32];
            snprintf(fps_text, sizeof(fps_text), "FPS: %.1f", current_fps);
            VaxpPaint fps_paint = vaxp_paint_fill((VaxpColor){100, 100, 100, 255});
            vaxp_canvas_draw_text(canvas, fps_text, 10, 20, NULL, &fps_paint);
            
            vaxp_canvas_flush(canvas);
            needs_redraw = VAXP_FALSE;
        }
        
        /* 5. Frame limiting to 60 FPS */
        VaxpU64 frame_end = get_time_us();
        VaxpU64 elapsed = frame_end - frame_start;
        if (elapsed < FRAME_TIME_US) {
            usleep((useconds_t)(FRAME_TIME_US - elapsed));
        }
    }
    
    printf("\nCleaning up...\n");
    vaxp_event_queue_destroy(event_queue);
    vaxp_unref(content);
    vaxp_unref(canvas);
    vaxp_display_close(display);
    vaxp_shutdown();
    
    printf("Batch %d Complete!\n", current_batch);
    return 0;
}
