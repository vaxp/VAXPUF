/*
 * VAXPUI - Widget-Based Panel Demo
 * 
 * Tests the framework's widget system maturity
 * Uses built-in widgets instead of direct canvas drawing
 */

#include <vaxp/vaxpui.h>
#include <stdio.h>
#include <time.h>

/* ============================================================================
 * STATE
 * ============================================================================ */

static struct {
    VaxpF32 volume;
    VaxpF32 brightness;
    VaxpBool wifi_on;
    VaxpBool bluetooth_on;
    VaxpBool dark_mode;
    int battery_percent;
    char time_str[16];
} g_state = {
    .volume = 0.75f,
    .brightness = 0.8f,
    .wifi_on = VAXP_TRUE,
    .bluetooth_on = VAXP_FALSE,
    .dark_mode = VAXP_TRUE,
    .battery_percent = 75,
};

/* ============================================================================
 * CALLBACKS
 * ============================================================================ */

static void on_volume_change(VaxpSlider* slider, VaxpF32 value, void* data) {
    (void)slider; (void)data;
    g_state.volume = value;
    printf("Volume: %.0f%%\n", value * 100);
}

static void on_brightness_change(VaxpSlider* slider, VaxpF32 value, void* data) {
    (void)slider; (void)data;
    g_state.brightness = value;
    printf("Brightness: %.0f%%\n", value * 100);
}

static void on_wifi_toggle(VaxpSwitch* sw, VaxpBool on, void* data) {
    (void)sw; (void)data;
    g_state.wifi_on = on;
    printf("WiFi: %s\n", on ? "ON" : "OFF");
    vaxp_rebuild();
}

static void on_bluetooth_toggle(VaxpSwitch* sw, VaxpBool on, void* data) {
    (void)sw; (void)data;
    g_state.bluetooth_on = on;
    printf("Bluetooth: %s\n", on ? "ON" : "OFF");
    vaxp_rebuild();
}

/* ============================================================================
 * UPDATE TIME
 * ============================================================================ */

static void update_time(void) {
    time_t now = time(NULL);
    struct tm* tm = localtime(&now);
    strftime(g_state.time_str, sizeof(g_state.time_str), "%H:%M", tm);
}

/* ============================================================================
 * PANEL BUILD FUNCTION
 * ============================================================================ */

static VaxpWidget* build_control_center(void* data) {
    (void)data;
    update_time();
    
    /* Main container (Column layout) */
    VaxpResultPtr cr = vaxp_container_create_column();
    if (!cr.ok) return NULL;
    VaxpContainer* main_col = (VaxpContainer*)cr.value;
    vaxp_container_set_gap(main_col, 12);
    vaxp_container_set_background(main_col, vaxp_color_rgb(35, 35, 50));
    ((VaxpWidget*)main_col)->layout.padding = (VaxpInsets){ 15, 15, 15, 15 };
    
    /* ===== Title Label ===== */
    VaxpResultPtr lr = vaxp_label_create("Control Center");
    if (lr.ok) {
        VaxpLabel* title = (VaxpLabel*)lr.value;
        vaxp_label_set_font_size(title, 20);
        vaxp_label_set_color(title, vaxp_color_rgb(255, 255, 255));
        vaxp_widget_add_child((VaxpWidget*)main_col, (VaxpWidget*)title);
    }
    
    /* ===== Time Label ===== */
    lr = vaxp_label_create(g_state.time_str);
    if (lr.ok) {
        VaxpLabel* time_label = (VaxpLabel*)lr.value;
        vaxp_label_set_font_size(time_label, 48);
        vaxp_label_set_color(time_label, vaxp_color_rgb(255, 255, 255));
        vaxp_widget_add_child((VaxpWidget*)main_col, (VaxpWidget*)time_label);
    }
    
    /* ===== Toggle Buttons Row ===== */
    VaxpResultPtr row_r = vaxp_container_create_row();
    if (row_r.ok) {
        VaxpContainer* row = (VaxpContainer*)row_r.value;
        vaxp_container_set_gap(row, 10);
        
        /* WiFi Toggle */
        VaxpResultPtr sw_r = vaxp_switch_create();
        if (sw_r.ok) {
            VaxpSwitch* wifi = (VaxpSwitch*)sw_r.value;
            vaxp_switch_set_on(wifi, g_state.wifi_on);
            vaxp_switch_set_on_change(wifi, on_wifi_toggle, NULL);
            vaxp_widget_add_child((VaxpWidget*)row, (VaxpWidget*)wifi);
        }
        lr = vaxp_label_create("WiFi");
        if (lr.ok) {
            vaxp_label_set_color((VaxpLabel*)lr.value, vaxp_color_rgb(200, 200, 200));
            vaxp_widget_add_child((VaxpWidget*)row, (VaxpWidget*)lr.value);
        }
        
        /* Bluetooth Toggle */
        sw_r = vaxp_switch_create();
        if (sw_r.ok) {
            VaxpSwitch* bt = (VaxpSwitch*)sw_r.value;
            vaxp_switch_set_on(bt, g_state.bluetooth_on);
            vaxp_switch_set_on_change(bt, on_bluetooth_toggle, NULL);
            vaxp_widget_add_child((VaxpWidget*)row, (VaxpWidget*)bt);
        }
        lr = vaxp_label_create("Bluetooth");
        if (lr.ok) {
            vaxp_label_set_color((VaxpLabel*)lr.value, vaxp_color_rgb(200, 200, 200));
            vaxp_widget_add_child((VaxpWidget*)row, (VaxpWidget*)lr.value);
        }
        
        vaxp_widget_add_child((VaxpWidget*)main_col, (VaxpWidget*)row);
    }
    
    /* ===== Brightness Label ===== */
    lr = vaxp_label_create("Brightness");
    if (lr.ok) {
        vaxp_label_set_color((VaxpLabel*)lr.value, vaxp_color_rgb(200, 200, 200));
        vaxp_widget_add_child((VaxpWidget*)main_col, (VaxpWidget*)lr.value);
    }
    
    /* ===== Brightness Slider ===== */
    VaxpResultPtr sr = vaxp_slider_create();
    if (sr.ok) {
        VaxpSlider* brightness = (VaxpSlider*)sr.value;
        vaxp_slider_set_range(brightness, 0.0f, 1.0f);
        vaxp_slider_set_value(brightness, g_state.brightness);
        vaxp_slider_set_on_change(brightness, on_brightness_change, NULL);
        brightness->fill_color = vaxp_color_rgb(255, 200, 100);
        vaxp_widget_add_child((VaxpWidget*)main_col, (VaxpWidget*)brightness);
    }
    
    /* ===== Volume Label ===== */
    lr = vaxp_label_create("Volume");
    if (lr.ok) {
        vaxp_label_set_color((VaxpLabel*)lr.value, vaxp_color_rgb(200, 200, 200));
        vaxp_widget_add_child((VaxpWidget*)main_col, (VaxpWidget*)lr.value);
    }
    
    /* ===== Volume Slider ===== */
    sr = vaxp_slider_create();
    if (sr.ok) {
        VaxpSlider* volume = (VaxpSlider*)sr.value;
        vaxp_slider_set_range(volume, 0.0f, 1.0f);
        vaxp_slider_set_value(volume, g_state.volume);
        vaxp_slider_set_on_change(volume, on_volume_change, NULL);
        volume->fill_color = vaxp_color_rgb(100, 200, 150);
        vaxp_widget_add_child((VaxpWidget*)main_col, (VaxpWidget*)volume);
    }
    
    /* ===== Battery Progress ===== */
    lr = vaxp_label_create("Battery");
    if (lr.ok) {
        vaxp_label_set_color((VaxpLabel*)lr.value, vaxp_color_rgb(200, 200, 200));
        vaxp_widget_add_child((VaxpWidget*)main_col, (VaxpWidget*)lr.value);
    }
    
    VaxpResultPtr pr = vaxp_progress_create();
    if (pr.ok) {
        VaxpProgressBar* battery = (VaxpProgressBar*)pr.value;
        vaxp_progress_set_value(battery, g_state.battery_percent / 100.0f);
        vaxp_progress_set_colors(battery, 
            vaxp_color_rgb(60, 60, 70),
            g_state.battery_percent > 20 
                ? vaxp_color_rgb(80, 200, 120) 
                : vaxp_color_rgb(255, 80, 80));
        vaxp_widget_add_child((VaxpWidget*)main_col, (VaxpWidget*)battery);
    }
    
    /* ===== Buttons Row ===== */
    row_r = vaxp_container_create_row();
    if (row_r.ok) {
        VaxpContainer* row = (VaxpContainer*)row_r.value;
        vaxp_container_set_gap(row, 10);
        
        /* Settings Button */
        VaxpResultPtr br = vaxp_button_create("Settings");
        if (br.ok) {
            vaxp_widget_add_child((VaxpWidget*)row, (VaxpWidget*)br.value);
        }
        
        /* About Button */
        br = vaxp_button_create("About");
        if (br.ok) {
            vaxp_widget_add_child((VaxpWidget*)row, (VaxpWidget*)br.value);
        }
        
        vaxp_widget_add_child((VaxpWidget*)main_col, (VaxpWidget*)row);
    }
    
    printf("Control Center built with %u widgets\n", ((VaxpWidget*)main_col)->children_count);
    
    return (VaxpWidget*)main_col;
}

/* ============================================================================
 * MAIN
 * ============================================================================ */

int main(void) {
    setbuf(stdout, NULL);
    
    printf("===========================================\n");
    printf("   VAXPUI Widget-Based Panel Demo\n");
    printf("   Testing Framework Widget Maturity\n");
    printf("===========================================\n\n");
    
    /* Read battery */
    FILE* f = fopen("/sys/class/power_supply/BAT0/capacity", "r");
    if (f) { 
        if (fscanf(f, "%d", &g_state.battery_percent) != 1) 
            g_state.battery_percent = 75;
        fclose(f); 
    }
    
    VaxpAppConfig cfg = {
        .title = "Control Center (Widget Demo)",
        .width = 350,
        .height = 500,
        .background = vaxp_color_rgb(35, 35, 50),
        .build = build_control_center,
    };
    
    return vaxp_run_app(&cfg);
}
