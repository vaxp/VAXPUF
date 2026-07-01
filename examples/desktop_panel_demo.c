/*
 * VAXPUI - Desktop Panel Demo
 * 
 * Real top panel with popup control center.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <vaxp/vaxpui.h>

/* ============================================================================
 * STATE
 * ============================================================================ */

static int g_battery = 85;
static int g_volume = 50;
static int g_wifi_on = 1;
static int g_bluetooth_on = 1;
static char g_time_str[16] = "12:00";

/* Control Center process tracking */
static int g_cc_open = 0;

/* ============================================================================
 * UPDATE TIME
 * ============================================================================ */

static void update_time(void) {
    time_t now = time(NULL);
    struct tm* tm = localtime(&now);
    strftime(g_time_str, sizeof(g_time_str), "%H:%M", tm);
}

/* ============================================================================
 * CALLBACKS
 * ============================================================================ */

static void on_control_center_click(VaxpButton* btn, void* data) {
    (void)btn; (void)data;
    
    if (g_cc_open) {
        /* Close control center */
        printf("Closing Control Center...\n");
        system("pkill -f control_center_demo");
        g_cc_open = 0;
    } else {
        /* Open control center */
        printf("Opening Control Center...\n");
        system("./build_new/examples/control_center_demo &");
        g_cc_open = 1;
    }
}

/* ============================================================================
 * BUILD PANEL UI (Simple horizontal row)
 * ============================================================================ */

static VaxpWidget* build_panel(void* user_data) {
    (void)user_data;
    
    update_time();
    
    /* Build strings */
    char vol_str[16];
    snprintf(vol_str, sizeof(vol_str), "🔊 %d%%", g_volume);
    
    char batt_str[16];
    snprintf(batt_str, sizeof(batt_str), "🔋 %d%%", g_battery);
    
    /* Single horizontal row */
    return vaxp_row(
        .gap = 12,
        .padding = (VaxpInsets){6, 12, 6, 12},
        .background = vaxp_color_rgba(20, 20, 30, 250),
        .align = VAXP_ALIGN_CENTER,
        .children = VAXP_CHILDREN(
            
            /* Left: Logo */
            vaxp_text("🐍 VAXP", .size = 11, .color = vaxp_color_rgba(255, 255, 255, 180)),
            
            /* Center: Spacer */
            vaxp_spacer(),
            
            /* Center: Clock */
            vaxp_text(g_time_str, 
                .size = 12,
                .color = vaxp_color_rgb(255, 255, 255)
            ),
            
            /* Center: Spacer */
            vaxp_spacer(),
            
            /* Right: System tray icons */
            vaxp_text(g_wifi_on ? "📶" : "📴", .size = 11),
            vaxp_text(g_bluetooth_on ? "🔵" : "⚫", .size = 11),
            vaxp_text(vol_str, .size = 10, .color = vaxp_color_rgb(255, 255, 255)),
            vaxp_text(batt_str, .size = 10, .color = vaxp_color_rgb(255, 255, 255)),
            
            /* Control Center button */
            vaxp_btn("⚙",
                .color = vaxp_color_rgba(255, 255, 255, 30),
                .text_color = vaxp_color_rgb(255, 255, 255),
                .corner_radius = 6,
                .on_click = on_control_center_click
            )
        )
    );
}

/* ============================================================================
 * MAIN
 * ============================================================================ */

int main(void) {
    printf("===========================================\n");
    printf("   VAXPUI Desktop Panel\n");
    printf("   Click ⚙ to open Control Center\n");
    printf("===========================================\n\n");
    
    return VAXP_PANEL_APP(
        .title = "VAXPUI Panel",
        .height = 38,
        .build = build_panel,
        .debug = VAXP_TRUE
    );
}
