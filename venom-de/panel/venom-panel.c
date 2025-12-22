/*
 * VENOMUI - Desktop Panel Demo
 * 
 * A demo of a desktop panel shell component:
 * - Top panel with clock and battery indicator
 * - Control center popup window when clicking the icon
 * 
 * This demonstrates OpenGL can build desktop environments!
*/

#define _POSIX_C_SOURCE 199309L
#define _DEFAULT_SOURCE
#define _USE_MATH_DEFINES

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include <GL/gl.h>
#include <GL/glx.h>

#include "venom/core/venom_types.h"
#include "venom/core/venom_result.h"
#include "venom/core/venom_ref.h"
#include "venom/graphics/venom_canvas.h"
#include "venom/graphics/venom_canvas_opengl.h"

/* Audio D-Bus client */
#include "audio_client.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* ============================================================================
 * PANEL STATE
 * ============================================================================ */

static struct {
    int control_center_open;
    int battery_percent;
    int battery_charging;
    int mouse_x, mouse_y;
    int icon_hovered;
    
    /* Control Center popup window */
    Window cc_window;
    VenomCanvas* cc_canvas;
    int cc_visible;
    
    /* Audio state */
    VenomAudioState audio;
    int audio_connected;
    
    /* Slider dragging */
    int volume_dragging;
} g_state = {0};

/* GLX context for sharing */
static GLXContext g_glx_context = NULL;

/* ============================================================================
 * BATTERY READING (Linux)
 * ============================================================================ */

static void update_battery(void) {
    FILE* f = fopen("/sys/class/power_supply/BAT0/capacity", "r");
    if (f) {
        if (fscanf(f, "%d", &g_state.battery_percent) != 1) g_state.battery_percent = 75;
        fclose(f);
    } else {
        g_state.battery_percent = 75;
    }
    
    f = fopen("/sys/class/power_supply/BAT0/status", "r");
    if (f) {
        char status[32];
        if (fscanf(f, "%31s", status) == 1)
            g_state.battery_charging = (strcmp(status, "Charging") == 0);
        fclose(f);
    }
}

/* ============================================================================
 * TIME STRING
 * ============================================================================ */

static void get_time_string(char* buf, size_t size) {
    time_t now = time(NULL);
    struct tm* tm = localtime(&now);
    strftime(buf, size, "%H:%M", tm);
}

/* ============================================================================
 * DRAWING HELPERS
 * ============================================================================ */

static void draw_battery_icon(VenomCanvas* canvas, float x, float y, int percent, int charging) {
    VenomPaint outline = venom_paint_stroke(venom_color_rgba(255, 255, 255, 200), 1.5f);
    VenomRectF body = { x, y, 20, 10 };
    venom_canvas_draw_rounded_rect(canvas, body, 2.0f, &outline);
    
    VenomPaint tip_paint = venom_paint_fill(venom_color_rgba(255, 255, 255, 200));
    VenomRectF tip = { x + 20, y + 2.5f, 2, 5 };
    venom_canvas_draw_rect(canvas, tip, &tip_paint);
    
    VenomColor fill_color;
    if (percent > 50) fill_color = venom_color_rgba(80, 200, 120, 255);
    else if (percent > 20) fill_color = venom_color_rgba(255, 180, 50, 255);
    else fill_color = venom_color_rgba(255, 80, 80, 255);
    
    VenomPaint fill = venom_paint_fill(fill_color);
    float fill_width = (percent / 100.0f) * 16.0f;
    VenomRectF fill_rect = { x + 2, y + 2, fill_width, 6 };
    venom_canvas_draw_rect(canvas, fill_rect, &fill);
    
    if (charging) {
        VenomPaint bolt = venom_paint_fill(venom_color_rgba(255, 255, 255, 255));
        venom_canvas_draw_circle(canvas, x + 10, y + 5, 2.5f, &bolt);
    }
}

static void draw_control_center_icon(VenomCanvas* canvas, float x, float y, int hovered) {
    VenomU8 alpha = hovered ? 255 : 180;
    VenomPaint paint = venom_paint_fill(venom_color_rgba(255, 255, 255, alpha));
    
    for (int i = 0; i < 3; i++) {
        VenomRectF line = { x, y + i * 4, 12, 2 };
        venom_canvas_draw_rounded_rect(canvas, line, 1.0f, &paint);
    }
}

static void draw_control_center_content(VenomCanvas* canvas, int width, int height) {
    /* Background */
    VenomPaint bg = venom_paint_fill(venom_color_rgba(30, 30, 40, 250));
    VenomRectF bg_rect = { 0, 0, (float)width, (float)height };
    venom_canvas_draw_rounded_rect(canvas, bg_rect, 16.0f, &bg);
    
    /* Border */
    VenomPaint border = venom_paint_stroke(venom_color_rgba(100, 100, 120, 100), 1.0f);
    venom_canvas_draw_rounded_rect(canvas, bg_rect, 16.0f, &border);
    
    /* Title placeholder */
    VenomPaint title_paint = venom_paint_fill(venom_color_rgba(255, 255, 255, 255));
    VenomRectF title_rect = { 20, 15, 120, 16 };
    venom_canvas_draw_rounded_rect(canvas, title_rect, 4.0f, &title_paint);
    
    /* Quick toggles row */
    float toggle_y = 50;
    float toggle_size = 55;
    float toggle_gap = 12;
    int toggle_states[] = {1, 1, 0, 0};
    
    for (int i = 0; i < 4; i++) {
        float tx = 15 + i * (toggle_size + toggle_gap);
        
        VenomColor toggle_color = toggle_states[i] 
            ? venom_color_rgba(100, 150, 255, 255) 
            : venom_color_rgba(60, 60, 70, 255);
        
        VenomPaint toggle_paint = venom_paint_fill(toggle_color);
        VenomRectF toggle_rect = { tx, toggle_y, toggle_size, toggle_size };
        venom_canvas_draw_rounded_rect(canvas, toggle_rect, 12.0f, &toggle_paint);
        
        /* Icon */
        VenomPaint icon_paint = venom_paint_fill(venom_color_rgba(255, 255, 255, 200));
        venom_canvas_draw_circle(canvas, tx + toggle_size/2, toggle_y + 22, 10, &icon_paint);
        
        /* Label placeholder */
        VenomPaint label = venom_paint_fill(venom_color_rgba(255, 255, 255, 150));
        VenomRectF label_rect = { tx + 10, toggle_y + toggle_size - 14, toggle_size - 20, 8 };
        venom_canvas_draw_rounded_rect(canvas, label_rect, 2.0f, &label);
    }
    
    /* Brightness slider */
    float slider_y = toggle_y + toggle_size + 20;
    VenomPaint slider_bg = venom_paint_fill(venom_color_rgba(60, 60, 70, 255));
    VenomRectF slider_track = { 15, slider_y, width - 30, 35 };
    venom_canvas_draw_rounded_rect(canvas, slider_track, 10.0f, &slider_bg);
    
    VenomPaint slider_fill = venom_paint_fill(venom_color_rgba(255, 200, 100, 255));
    VenomRectF slider_value = { 15, slider_y, (width - 30) * 0.7f, 35 };
    venom_canvas_draw_rounded_rect(canvas, slider_value, 10.0f, &slider_fill);
    
    VenomPaint sun = venom_paint_fill(venom_color_rgba(255, 255, 255, 255));
    venom_canvas_draw_circle(canvas, 35, slider_y + 17, 7, &sun);
    
    /* Volume slider - connected to audio D-Bus */
    float vol_y = slider_y + 50;
    venom_canvas_draw_rounded_rect(canvas, (VenomRectF){ 15, vol_y, width - 30, 35 }, 10.0f, &slider_bg);
    
    /* Volume fill based on real audio volume */
    float vol_percent = g_state.audio.volume / 100.0f;
    VenomColor vol_color = g_state.audio.muted 
        ? venom_color_rgba(100, 100, 100, 255) 
        : venom_color_rgba(100, 200, 150, 255);
    VenomPaint vol_fill = venom_paint_fill(vol_color);
    venom_canvas_draw_rounded_rect(canvas, (VenomRectF){ 15, vol_y, (width - 30) * vol_percent, 35 }, 10.0f, &vol_fill);
    
    /* Speaker icon */
    VenomPaint speaker_p = venom_paint_fill(venom_color_rgba(255, 255, 255, 255));
    /* Speaker body (small rectangle) */
    venom_canvas_draw_rect(canvas, (VenomRectF){ 26, vol_y + 13, 6, 9 }, &speaker_p);
    /* Speaker cone (larger part) */
    venom_canvas_draw_rect(canvas, (VenomRectF){ 32, vol_y + 10, 4, 15 }, &speaker_p);
    /* Sound waves (if not muted) */
    if (!g_state.audio.muted && g_state.audio.volume > 0) {
        VenomPaint wave = venom_paint_stroke(venom_color_rgba(255, 255, 255, 180), 1.5f);
        venom_canvas_draw_circle(canvas, 40, vol_y + 17, 5, &wave);
        if (g_state.audio.volume > 50) {
            venom_canvas_draw_circle(canvas, 40, vol_y + 17, 9, &wave);
        }
    }
    /* Mute line (if muted) */
    if (g_state.audio.muted) {
        VenomPaint mute = venom_paint_stroke(venom_color_rgba(255, 80, 80, 255), 2.0f);
        venom_canvas_draw_line(canvas, 25, vol_y + 8, 45, vol_y + 27, &mute);
    }
    
    /* Volume percentage text */
    char vol_str[8];
    snprintf(vol_str, sizeof(vol_str), "%d%%", g_state.audio.volume);
    VenomPaint vol_text = venom_paint_fill(venom_color_rgba(255, 255, 255, 200));
    venom_canvas_draw_text(canvas, vol_str, width - 60, vol_y + 24, NULL, &vol_text);
    
    /* Music player area */
    float music_y = vol_y + 55;
    VenomPaint music_bg = venom_paint_fill(venom_color_rgba(50, 50, 60, 255));
    VenomRectF music_rect = { 15, music_y, width - 30, 90 };
    venom_canvas_draw_rounded_rect(canvas, music_rect, 12.0f, &music_bg);
    
    /* Album art */
    VenomPaint album = venom_paint_fill(venom_color_rgba(100, 80, 120, 255));
    venom_canvas_draw_rounded_rect(canvas, (VenomRectF){ 25, music_y + 10, 70, 70 }, 8.0f, &album);
    
    /* Song title placeholder */
    VenomPaint song_title = venom_paint_fill(venom_color_rgba(255, 255, 255, 255));
    venom_canvas_draw_rounded_rect(canvas, (VenomRectF){ 105, music_y + 20, 100, 12 }, 3.0f, &song_title);
    
    /* Artist placeholder */
    VenomPaint artist = venom_paint_fill(venom_color_rgba(180, 180, 180, 255));
    venom_canvas_draw_rounded_rect(canvas, (VenomRectF){ 105, music_y + 38, 70, 10 }, 3.0f, &artist);
    
    /* Play button */
    VenomPaint play = venom_paint_fill(venom_color_rgba(255, 255, 255, 255));
    venom_canvas_draw_circle(canvas, width - 60, music_y + 45, 18, &play);
}

/* ============================================================================
 * CONTROL CENTER WINDOW
 * ============================================================================ */

static Window create_popup_window(Display* display, int screen, XVisualInfo* vi, 
                                   int x, int y, int width, int height) {
    Colormap colormap = XCreateColormap(display, RootWindow(display, screen), vi->visual, AllocNone);
    
    XSetWindowAttributes swa = {0};
    swa.colormap = colormap;
    swa.event_mask = ExposureMask | ButtonPressMask | ButtonReleaseMask | Button1MotionMask | LeaveWindowMask;
    swa.override_redirect = True;  /* No window decorations */
    swa.background_pixel = 0;
    swa.border_pixel = 0;
    
    Window window = XCreateWindow(display, RootWindow(display, screen),
                                   x, y, width, height,
                                   0, vi->depth, InputOutput, vi->visual,
                                   CWColormap | CWEventMask | CWOverrideRedirect | CWBackPixel | CWBorderPixel,
                                   &swa);
    
    /* Set window type to popup */
    Atom type_atom = XInternAtom(display, "_NET_WM_WINDOW_TYPE", False);
    Atom popup_atom = XInternAtom(display, "_NET_WM_WINDOW_TYPE_POPUP_MENU", False);
    XChangeProperty(display, window, type_atom, XA_ATOM, 32, PropModeReplace, (unsigned char*)&popup_atom, 1);
    
    return window;
}

static void show_control_center(Display* display, int screen_width, int panel_height) {
    if (g_state.cc_visible) return;
    
    XMapWindow(display, g_state.cc_window);
    XMoveWindow(display, g_state.cc_window, screen_width - 310, panel_height + 5);
    XRaiseWindow(display, g_state.cc_window);
    XFlush(display);
    
    g_state.cc_visible = 1;
    g_state.control_center_open = 1;
}

static void hide_control_center(Display* display) {
    if (!g_state.cc_visible) return;
    
    XUnmapWindow(display, g_state.cc_window);
    XFlush(display);
    
    g_state.cc_visible = 0;
    g_state.control_center_open = 0;
}

/* ============================================================================
 * MAIN
 * ============================================================================ */

int main(int argc, char** argv) {
    (void)argc; (void)argv;
    
    printf("===========================================\n");
    printf("   VENOMUI Desktop Panel Demo\n");
    printf("   OpenGL-powered Desktop Shell\n");
    printf("===========================================\n\n");
    
    Display* display = XOpenDisplay(NULL);
    if (!display) {
        fprintf(stderr, "Failed to open X display\n");
        return 1;
    }
    
    int screen = DefaultScreen(display);
    int screen_width = DisplayWidth(display, screen);
    int panel_height = 26;
    int cc_width = 300;
    int cc_height = 350;
    
    /* GLX setup */
    static int visual_attribs[] = {
        GLX_X_RENDERABLE, True,
        GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
        GLX_RENDER_TYPE, GLX_RGBA_BIT,
        GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
        GLX_RED_SIZE, 8, GLX_GREEN_SIZE, 8, GLX_BLUE_SIZE, 8, GLX_ALPHA_SIZE, 8,
        GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, True,
        None
    };
    
    int fbcount;
    GLXFBConfig* fbc = glXChooseFBConfig(display, screen, visual_attribs, &fbcount);
    if (!fbc) { fprintf(stderr, "No FB config\n"); return 1; }
    
    XVisualInfo* vi = glXGetVisualFromFBConfig(display, fbc[0]);
    XFree(fbc);
    
    Colormap colormap = XCreateColormap(display, RootWindow(display, screen), vi->visual, AllocNone);
    
    XSetWindowAttributes swa = {0};
    swa.colormap = colormap;
    swa.event_mask = ExposureMask | KeyPressMask | ButtonPressMask | PointerMotionMask;
    swa.background_pixel = 0;
    swa.border_pixel = 0;
    
    /* Create panel window */
    Window panel_window = XCreateWindow(display, RootWindow(display, screen),
                                   0, 0, screen_width, panel_height,
                                   0, vi->depth, InputOutput, vi->visual,
                                   CWColormap | CWEventMask | CWBackPixel | CWBorderPixel,
                                   &swa);
    
    /* Set window type to dock */
    Atom type_atom = XInternAtom(display, "_NET_WM_WINDOW_TYPE", False);
    Atom dock_atom = XInternAtom(display, "_NET_WM_WINDOW_TYPE_DOCK", False);
    XChangeProperty(display, panel_window, type_atom, XA_ATOM, 32, PropModeReplace, (unsigned char*)&dock_atom, 1);
    
    /* Reserve space at top */
    Atom strut = XInternAtom(display, "_NET_WM_STRUT_PARTIAL", False);
    long strut_values[12] = {0, 0, panel_height, 0, 0, 0, 0, 0, 0, screen_width - 1, 0, 0};
    XChangeProperty(display, panel_window, strut, XA_CARDINAL, 32, PropModeReplace, (unsigned char*)strut_values, 12);
    
    XStoreName(display, panel_window, "VENOMUI Panel");
    XMapWindow(display, panel_window);
    
    /* Create control center popup window */
    g_state.cc_window = create_popup_window(display, screen, vi, screen_width - cc_width - 10, panel_height + 5, cc_width, cc_height);
    
    XFree(vi);
    XFlush(display);
    
    /* Wait for expose */
    XEvent event;
    while (1) { XNextEvent(display, &event); if (event.type == Expose) break; }
    
    /* Create OpenGL canvas for panel */
    VenomResultPtr canvas_result = venom_canvas_create_opengl(display, panel_window, screen_width, panel_height);
    if (!canvas_result.ok) { fprintf(stderr, "Panel canvas failed\n"); return 1; }
    VenomCanvas* panel_canvas = (VenomCanvas*)canvas_result.value;
    
    /* Create OpenGL canvas for control center */
    canvas_result = venom_canvas_create_opengl(display, g_state.cc_window, cc_width, cc_height);
    if (!canvas_result.ok) { fprintf(stderr, "CC canvas failed\n"); return 1; }
    g_state.cc_canvas = (VenomCanvas*)canvas_result.value;
    
    printf("Panel created: %dx%d\n", screen_width, panel_height);
    printf("Control Center: %dx%d\n", cc_width, cc_height);
    printf("Click the ≡ icon on the right to toggle Control Center\n");
    printf("Press ESC to exit\n\n");
    
    /* Disable VSync for both */
    typedef int (*SwapIntervalProc)(Display*, GLXDrawable, int);
    SwapIntervalProc swapInterval = (SwapIntervalProc)glXGetProcAddress((unsigned char*)"glXSwapIntervalEXT");
    if (swapInterval) {
        swapInterval(display, panel_window, 0);
        swapInterval(display, g_state.cc_window, 0);
    }
    
    Atom wm_delete = XInternAtom(display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display, panel_window, &wm_delete, 1);
    
    update_battery();
    time_t last_battery_update = time(NULL);
    
    /* Initialize audio D-Bus connection */
    g_state.audio_connected = venom_audio_init();
    if (g_state.audio_connected) {
        venom_audio_get_state(&g_state.audio);
        printf("Audio: Volume=%d%%, Muted=%s\n", g_state.audio.volume, g_state.audio.muted ? "yes" : "no");
    } else {
        printf("Audio: Not connected (daemon not running?)\n");
        g_state.audio.volume = 50;
        g_state.audio.muted = false;
    }
    
    /* FPS counter */
    struct timespec ts_start, ts_now;
    clock_gettime(CLOCK_MONOTONIC, &ts_start);
    double last_fps_time = ts_start.tv_sec + ts_start.tv_nsec / 1e9;
    int frame_count = 0;
    double fps = 0.0;
    
    int running = 1;
    while (running) {
        while (XPending(display)) {
            XNextEvent(display, &event);
            
            switch (event.type) {
                case KeyPress:
                    if (XLookupKeysym(&event.xkey, 0) == XK_Escape) running = 0;
                    break;
                    
                case ButtonPress:
                    if (event.xbutton.window == panel_window) {
                        /* Check if control center icon clicked */
                        if (event.xbutton.x >= screen_width - 35 && event.xbutton.x <= screen_width - 10 &&
                            event.xbutton.y >= 5 && event.xbutton.y <= 21) {
                            if (g_state.cc_visible) {
                                hide_control_center(display);
                            } else {
                                show_control_center(display, screen_width, panel_height);
                            }
                        } else if (g_state.cc_visible) {
                            hide_control_center(display);
                        }
                    }
                    else if (event.xbutton.window == g_state.cc_window) {
                        int mx = event.xbutton.x;
                        int my = event.xbutton.y;
                        
                        /* Volume slider area: x=15 to cc_width-15, y=175 to 210 (approx) */
                        if (mx >= 15 && mx <= cc_width - 15 && my >= 175 && my <= 210) {
                            g_state.volume_dragging = 1;  /* Enable dragging */
                            
                            /* Calculate volume from X position */
                            float slider_width = cc_width - 30;
                            float click_pos = mx - 15;
                            int new_volume = (int)((click_pos / slider_width) * 100);
                            if (new_volume < 0) new_volume = 0;
                            if (new_volume > 100) new_volume = 100;
                            
                            if (g_state.audio_connected) {
                                venom_audio_set_volume(new_volume);
                                g_state.audio.volume = new_volume;
                            }
                        }
                        
                        /* Brightness slider: y=125 to 160 (approx) */
                        /* TODO: Add brightness control */
                    }
                    break;
                    
                case ButtonRelease:
                    g_state.volume_dragging = 0;
                    break;
                    
                case LeaveNotify:
                    if (event.xcrossing.window == g_state.cc_window) {
                        g_state.volume_dragging = 0;
                    }
                    break;
                    
                case MotionNotify:
                    if (event.xmotion.window == panel_window) {
                        g_state.mouse_x = event.xmotion.x;
                        g_state.mouse_y = event.xmotion.y;
                        g_state.icon_hovered = (event.xmotion.x >= screen_width - 35 && 
                                                event.xmotion.x <= screen_width - 10 &&
                                                event.xmotion.y >= 5 && event.xmotion.y <= 21);
                    }
                    else if (event.xmotion.window == g_state.cc_window && g_state.volume_dragging) {
                        /* Dragging volume slider */
                        int mx = event.xmotion.x;
                        float slider_width = cc_width - 30;
                        float click_pos = mx - 15;
                        int new_volume = (int)((click_pos / slider_width) * 100);
                        if (new_volume < 0) new_volume = 0;
                        if (new_volume > 100) new_volume = 100;
                        
                        if (g_state.audio_connected) {
                            venom_audio_set_volume(new_volume);
                            g_state.audio.volume = new_volume;
                        }
                    }
                    break;
                    
                case ClientMessage:
                    if ((Atom)event.xclient.data.l[0] == wm_delete) running = 0;
                    break;
            }
        }
        
        /* Update battery every 30 seconds */
        if (time(NULL) - last_battery_update > 30) {
            update_battery();
            last_battery_update = time(NULL);
        }
        
        /* ===== Draw Panel ===== */
        
        VenomColor panel_bg = venom_color_rgba(20, 20, 30, 245);
        venom_canvas_clear(panel_canvas, panel_bg);
        
        /* Clock background */
        VenomPaint clock_bg = venom_paint_fill(venom_color_rgba(255, 255, 255, 30));
        float clock_x = screen_width / 2.0f - 35;
        VenomRectF clock_rect = { clock_x, 3, 70, 20 };
        venom_canvas_draw_rounded_rect(panel_canvas, clock_rect, 10.0f, &clock_bg);
        
        /* Clock text - actual time */
        time_t now = time(NULL);
        struct tm* tm_info = localtime(&now);
        char time_str[16];
        strftime(time_str, sizeof(time_str), "%H:%M", tm_info);
        
        VenomPaint text_paint = venom_paint_fill(venom_color_rgba(255, 255, 255, 255));
        venom_canvas_draw_text(panel_canvas, time_str, clock_x + 15, 18, NULL, &text_paint);
        
        /* Battery */
        draw_battery_icon(panel_canvas, screen_width - 100, 8, g_state.battery_percent, g_state.battery_charging);
        
        /* Battery percentage text */
        char batt_str[8];
        snprintf(batt_str, sizeof(batt_str), "%d%%", g_state.battery_percent);
        venom_canvas_draw_text(panel_canvas, batt_str, screen_width - 70, 18, NULL, &text_paint);
        
        /* Control center icon */
        draw_control_center_icon(panel_canvas, screen_width - 30, 7, g_state.icon_hovered);
        
        venom_canvas_flush(panel_canvas);
        
        /* ===== Draw Control Center if visible ===== */
        if (g_state.cc_visible) {
            venom_canvas_clear(g_state.cc_canvas, venom_color_rgba(0, 0, 0, 0));
            draw_control_center_content(g_state.cc_canvas, cc_width, cc_height);
            venom_canvas_flush(g_state.cc_canvas);
        }
        
        /* FPS counter */
        frame_count++;
        clock_gettime(CLOCK_MONOTONIC, &ts_now);
        double current_time = ts_now.tv_sec + ts_now.tv_nsec / 1e9;
        if (current_time - last_fps_time >= 1.0) {
            fps = frame_count / (current_time - last_fps_time);
            frame_count = 0;
            last_fps_time = current_time;
            printf("\rFPS: %.1f  ", fps);
            fflush(stdout);
        }
        
        /* Update audio state periodically */
        static int audio_update_counter = 0;
        if (++audio_update_counter >= 60) {  /* Every ~0.5s */
            audio_update_counter = 0;
            if (g_state.audio_connected) {
                venom_audio_get_state(&g_state.audio);
            }
        }
        
        usleep(8000); /* ~120 FPS */
    }
    
    /* Cleanup audio */
    venom_audio_cleanup();
    
    printf("\n\nPanel demo finished!\n");
    
    venom_unref(panel_canvas);
    venom_unref(g_state.cc_canvas);
    XDestroyWindow(display, g_state.cc_window);
    XDestroyWindow(display, panel_window);
    XFreeColormap(display, colormap);
    XCloseDisplay(display);
    
    return 0;
}
