/*
 * VAXPUI - Control Center Demo
 * 
 * Popup window with system controls.
 */

#include <stdio.h>
#include <vaxp/vaxpui.h>

/* ============================================================================
 * STATE
 * ============================================================================ */

static int g_wifi_on = 1;
static int g_bluetooth_on = 1;
static int g_volume = 50;
static int g_brightness = 70;

/* ============================================================================
 * CALLBACKS
 * ============================================================================ */

static void on_wifi_toggle(VaxpButton* btn, void* data) {
    (void)btn; (void)data;
    g_wifi_on = !g_wifi_on;
    printf("WiFi: %s\n", g_wifi_on ? "ON" : "OFF");
    vaxp_rebuild();
}

static void on_bluetooth_toggle(VaxpButton* btn, void* data) {
    (void)btn; (void)data;
    g_bluetooth_on = !g_bluetooth_on;
    printf("Bluetooth: %s\n", g_bluetooth_on ? "ON" : "OFF");
    vaxp_rebuild();
}

static void on_volume_up(VaxpButton* btn, void* data) {
    (void)btn; (void)data;
    g_volume = (g_volume + 10 > 100) ? 100 : g_volume + 10;
    printf("Volume: %d%%\n", g_volume);
    vaxp_rebuild();
}

static void on_volume_down(VaxpButton* btn, void* data) {
    (void)btn; (void)data;
    g_volume = (g_volume - 10 < 0) ? 0 : g_volume - 10;
    printf("Volume: %d%%\n", g_volume);
    vaxp_rebuild();
}

static void on_brightness_up(VaxpButton* btn, void* data) {
    (void)btn; (void)data;
    g_brightness = (g_brightness + 10 > 100) ? 100 : g_brightness + 10;
    printf("Brightness: %d%%\n", g_brightness);
    vaxp_rebuild();
}

static void on_brightness_down(VaxpButton* btn, void* data) {
    (void)btn; (void)data;
    g_brightness = (g_brightness - 10 < 0) ? 0 : g_brightness - 10;
    printf("Brightness: %d%%\n", g_brightness);
    vaxp_rebuild();
}

/* ============================================================================
 * BUILD UI
 * ============================================================================ */

static VaxpWidget* build_control_center(void* user_data) {
    (void)user_data;
    
    char vol_str[16], bright_str[16];
    snprintf(vol_str, sizeof(vol_str), "%d%%", g_volume);
    snprintf(bright_str, sizeof(bright_str), "%d%%", g_brightness);
    
    return vaxp_col(
        .gap = 16,
        .padding = (VaxpInsets){20, 20, 20, 20},
        .background = vaxp_color_rgba(30, 30, 40, 250),
        .corner_radius = 16,
        .children = VAXP_CHILDREN(
            
            /* Header */
            vaxp_text("Control Center",
                .size = 18,
                .color = vaxp_color_rgb(255, 255, 255)
            ),
            
            /* Quick Toggles Row */
            vaxp_row(
                .gap = 12,
                .children = VAXP_CHILDREN(
                    vaxp_btn(g_wifi_on ? "📶 WiFi" : "📴 WiFi",
                        .color = g_wifi_on ? VAXP_PRIMARY : vaxp_color_rgb(60, 60, 70),
                        .text_color = vaxp_color_rgb(255, 255, 255),
                        .corner_radius = 12,
                        .on_click = on_wifi_toggle
                    ),
                    vaxp_btn(g_bluetooth_on ? "🔵 BT" : "⚫ BT",
                        .color = g_bluetooth_on ? VAXP_PRIMARY : vaxp_color_rgb(60, 60, 70),
                        .text_color = vaxp_color_rgb(255, 255, 255),
                        .corner_radius = 12,
                        .on_click = on_bluetooth_toggle
                    )
                )
            ),
            
            /* Volume Control */
            vaxp_row(
                .gap = 8,
                .align = VAXP_ALIGN_CENTER,
                .children = VAXP_CHILDREN(
                    vaxp_text("🔊", .size = 16),
                    vaxp_btn("−",
                        .color = vaxp_color_rgb(60, 60, 70),
                        .text_color = vaxp_color_rgb(255, 255, 255),
                        .corner_radius = 6,
                        .on_click = on_volume_down
                    ),
                    vaxp_text(vol_str, .color = vaxp_color_rgb(255, 255, 255)),
                    vaxp_btn("+",
                        .color = vaxp_color_rgb(60, 60, 70),
                        .text_color = vaxp_color_rgb(255, 255, 255),
                        .corner_radius = 6,
                        .on_click = on_volume_up
                    )
                )
            ),
            
            /* Brightness Control */
            vaxp_row(
                .gap = 8,
                .align = VAXP_ALIGN_CENTER,
                .children = VAXP_CHILDREN(
                    vaxp_text("☀️", .size = 16),
                    vaxp_btn("−",
                        .color = vaxp_color_rgb(60, 60, 70),
                        .text_color = vaxp_color_rgb(255, 255, 255),
                        .corner_radius = 6,
                        .on_click = on_brightness_down
                    ),
                    vaxp_text(bright_str, .color = vaxp_color_rgb(255, 255, 255)),
                    vaxp_btn("+",
                        .color = vaxp_color_rgb(60, 60, 70),
                        .text_color = vaxp_color_rgb(255, 255, 255),
                        .corner_radius = 6,
                        .on_click = on_brightness_up
                    )
                )
            ),
            
            /* Close hint */
            vaxp_text("Press ESC to close",
                .size = 10,
                .color = vaxp_color_rgba(255, 255, 255, 100)
            )
        )
    );
}

/* ============================================================================
 * MAIN
 * ============================================================================ */

int main(void) {
    printf("Control Center opened\n");
    
    return VAXP_POPUP_APP(
        .title = "Control Center",
        .width = 280,
        .height = 260,
        .position = VAXP_POSITION_CENTER,
        .build = build_control_center,
        .debug = VAXP_TRUE
    );
}
