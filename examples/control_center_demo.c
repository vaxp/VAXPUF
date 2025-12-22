/*
 * VENOMUI - Control Center Demo
 * 
 * Popup window with system controls.
 */

#include <stdio.h>
#include <venom/venomui.h>

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

static void on_wifi_toggle(VenomButton* btn, void* data) {
    (void)btn; (void)data;
    g_wifi_on = !g_wifi_on;
    printf("WiFi: %s\n", g_wifi_on ? "ON" : "OFF");
    venom_rebuild();
}

static void on_bluetooth_toggle(VenomButton* btn, void* data) {
    (void)btn; (void)data;
    g_bluetooth_on = !g_bluetooth_on;
    printf("Bluetooth: %s\n", g_bluetooth_on ? "ON" : "OFF");
    venom_rebuild();
}

static void on_volume_up(VenomButton* btn, void* data) {
    (void)btn; (void)data;
    g_volume = (g_volume + 10 > 100) ? 100 : g_volume + 10;
    printf("Volume: %d%%\n", g_volume);
    venom_rebuild();
}

static void on_volume_down(VenomButton* btn, void* data) {
    (void)btn; (void)data;
    g_volume = (g_volume - 10 < 0) ? 0 : g_volume - 10;
    printf("Volume: %d%%\n", g_volume);
    venom_rebuild();
}

static void on_brightness_up(VenomButton* btn, void* data) {
    (void)btn; (void)data;
    g_brightness = (g_brightness + 10 > 100) ? 100 : g_brightness + 10;
    printf("Brightness: %d%%\n", g_brightness);
    venom_rebuild();
}

static void on_brightness_down(VenomButton* btn, void* data) {
    (void)btn; (void)data;
    g_brightness = (g_brightness - 10 < 0) ? 0 : g_brightness - 10;
    printf("Brightness: %d%%\n", g_brightness);
    venom_rebuild();
}

/* ============================================================================
 * BUILD UI
 * ============================================================================ */

static VenomWidget* build_control_center(void* user_data) {
    (void)user_data;
    
    char vol_str[16], bright_str[16];
    snprintf(vol_str, sizeof(vol_str), "%d%%", g_volume);
    snprintf(bright_str, sizeof(bright_str), "%d%%", g_brightness);
    
    return venom_col(
        .gap = 16,
        .padding = (VenomInsets){20, 20, 20, 20},
        .background = venom_color_rgba(30, 30, 40, 250),
        .corner_radius = 16,
        .children = VENOM_CHILDREN(
            
            /* Header */
            venom_text("Control Center",
                .size = 18,
                .color = venom_color_rgb(255, 255, 255)
            ),
            
            /* Quick Toggles Row */
            venom_row(
                .gap = 12,
                .children = VENOM_CHILDREN(
                    venom_btn(g_wifi_on ? "📶 WiFi" : "📴 WiFi",
                        .color = g_wifi_on ? VENOM_PRIMARY : venom_color_rgb(60, 60, 70),
                        .text_color = venom_color_rgb(255, 255, 255),
                        .corner_radius = 12,
                        .on_click = on_wifi_toggle
                    ),
                    venom_btn(g_bluetooth_on ? "🔵 BT" : "⚫ BT",
                        .color = g_bluetooth_on ? VENOM_PRIMARY : venom_color_rgb(60, 60, 70),
                        .text_color = venom_color_rgb(255, 255, 255),
                        .corner_radius = 12,
                        .on_click = on_bluetooth_toggle
                    )
                )
            ),
            
            /* Volume Control */
            venom_row(
                .gap = 8,
                .align = VENOM_ALIGN_CENTER,
                .children = VENOM_CHILDREN(
                    venom_text("🔊", .size = 16),
                    venom_btn("−",
                        .color = venom_color_rgb(60, 60, 70),
                        .text_color = venom_color_rgb(255, 255, 255),
                        .corner_radius = 6,
                        .on_click = on_volume_down
                    ),
                    venom_text(vol_str, .color = venom_color_rgb(255, 255, 255)),
                    venom_btn("+",
                        .color = venom_color_rgb(60, 60, 70),
                        .text_color = venom_color_rgb(255, 255, 255),
                        .corner_radius = 6,
                        .on_click = on_volume_up
                    )
                )
            ),
            
            /* Brightness Control */
            venom_row(
                .gap = 8,
                .align = VENOM_ALIGN_CENTER,
                .children = VENOM_CHILDREN(
                    venom_text("☀️", .size = 16),
                    venom_btn("−",
                        .color = venom_color_rgb(60, 60, 70),
                        .text_color = venom_color_rgb(255, 255, 255),
                        .corner_radius = 6,
                        .on_click = on_brightness_down
                    ),
                    venom_text(bright_str, .color = venom_color_rgb(255, 255, 255)),
                    venom_btn("+",
                        .color = venom_color_rgb(60, 60, 70),
                        .text_color = venom_color_rgb(255, 255, 255),
                        .corner_radius = 6,
                        .on_click = on_brightness_up
                    )
                )
            ),
            
            /* Close hint */
            venom_text("Press ESC to close",
                .size = 10,
                .color = venom_color_rgba(255, 255, 255, 100)
            )
        )
    );
}

/* ============================================================================
 * MAIN
 * ============================================================================ */

int main(void) {
    printf("Control Center opened\n");
    
    return VENOM_POPUP_APP(
        .title = "Control Center",
        .width = 280,
        .height = 260,
        .position = VENOM_POSITION_CENTER,
        .build = build_control_center,
        .debug = VENOM_TRUE
    );
}
