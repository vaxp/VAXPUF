/*
 * VENOMUI - Widget-Based Panel Demo
 * 
 * Tests the framework's widget system maturity
 * Uses built-in widgets instead of direct canvas drawing
 */

#include <venom/venomui.h>
#include <stdio.h>
#include <time.h>

/* ============================================================================
 * STATE
 * ============================================================================ */

static struct {
    VenomF32 volume;
    VenomF32 brightness;
    VenomBool wifi_on;
    VenomBool bluetooth_on;
    VenomBool dark_mode;
    int battery_percent;
    char time_str[16];
} g_state = {
    .volume = 0.75f,
    .brightness = 0.8f,
    .wifi_on = VENOM_TRUE,
    .bluetooth_on = VENOM_FALSE,
    .dark_mode = VENOM_TRUE,
    .battery_percent = 75,
};

/* ============================================================================
 * CALLBACKS
 * ============================================================================ */

static void on_volume_change(VenomSlider* slider, VenomF32 value, void* data) {
    (void)slider; (void)data;
    g_state.volume = value;
    printf("Volume: %.0f%%\n", value * 100);
}

static void on_brightness_change(VenomSlider* slider, VenomF32 value, void* data) {
    (void)slider; (void)data;
    g_state.brightness = value;
    printf("Brightness: %.0f%%\n", value * 100);
}

static void on_wifi_toggle(VenomSwitch* sw, VenomBool on, void* data) {
    (void)sw; (void)data;
    g_state.wifi_on = on;
    printf("WiFi: %s\n", on ? "ON" : "OFF");
    venom_rebuild();
}

static void on_bluetooth_toggle(VenomSwitch* sw, VenomBool on, void* data) {
    (void)sw; (void)data;
    g_state.bluetooth_on = on;
    printf("Bluetooth: %s\n", on ? "ON" : "OFF");
    venom_rebuild();
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

static VenomWidget* build_control_center(void* data) {
    (void)data;
    update_time();
    
    /* Main container (Column layout) */
    VenomResultPtr cr = venom_container_create_column();
    if (!cr.ok) return NULL;
    VenomContainer* main_col = (VenomContainer*)cr.value;
    venom_container_set_gap(main_col, 12);
    venom_container_set_background(main_col, venom_color_rgb(35, 35, 50));
    ((VenomWidget*)main_col)->layout.padding = (VenomInsets){ 15, 15, 15, 15 };
    
    /* ===== Title Label ===== */
    VenomResultPtr lr = venom_label_create("Control Center");
    if (lr.ok) {
        VenomLabel* title = (VenomLabel*)lr.value;
        venom_label_set_font_size(title, 20);
        venom_label_set_color(title, venom_color_rgb(255, 255, 255));
        venom_widget_add_child((VenomWidget*)main_col, (VenomWidget*)title);
    }
    
    /* ===== Time Label ===== */
    lr = venom_label_create(g_state.time_str);
    if (lr.ok) {
        VenomLabel* time_label = (VenomLabel*)lr.value;
        venom_label_set_font_size(time_label, 48);
        venom_label_set_color(time_label, venom_color_rgb(255, 255, 255));
        venom_widget_add_child((VenomWidget*)main_col, (VenomWidget*)time_label);
    }
    
    /* ===== Toggle Buttons Row ===== */
    VenomResultPtr row_r = venom_container_create_row();
    if (row_r.ok) {
        VenomContainer* row = (VenomContainer*)row_r.value;
        venom_container_set_gap(row, 10);
        
        /* WiFi Toggle */
        VenomResultPtr sw_r = venom_switch_create();
        if (sw_r.ok) {
            VenomSwitch* wifi = (VenomSwitch*)sw_r.value;
            venom_switch_set_on(wifi, g_state.wifi_on);
            venom_switch_set_on_change(wifi, on_wifi_toggle, NULL);
            venom_widget_add_child((VenomWidget*)row, (VenomWidget*)wifi);
        }
        lr = venom_label_create("WiFi");
        if (lr.ok) {
            venom_label_set_color((VenomLabel*)lr.value, venom_color_rgb(200, 200, 200));
            venom_widget_add_child((VenomWidget*)row, (VenomWidget*)lr.value);
        }
        
        /* Bluetooth Toggle */
        sw_r = venom_switch_create();
        if (sw_r.ok) {
            VenomSwitch* bt = (VenomSwitch*)sw_r.value;
            venom_switch_set_on(bt, g_state.bluetooth_on);
            venom_switch_set_on_change(bt, on_bluetooth_toggle, NULL);
            venom_widget_add_child((VenomWidget*)row, (VenomWidget*)bt);
        }
        lr = venom_label_create("Bluetooth");
        if (lr.ok) {
            venom_label_set_color((VenomLabel*)lr.value, venom_color_rgb(200, 200, 200));
            venom_widget_add_child((VenomWidget*)row, (VenomWidget*)lr.value);
        }
        
        venom_widget_add_child((VenomWidget*)main_col, (VenomWidget*)row);
    }
    
    /* ===== Brightness Label ===== */
    lr = venom_label_create("Brightness");
    if (lr.ok) {
        venom_label_set_color((VenomLabel*)lr.value, venom_color_rgb(200, 200, 200));
        venom_widget_add_child((VenomWidget*)main_col, (VenomWidget*)lr.value);
    }
    
    /* ===== Brightness Slider ===== */
    VenomResultPtr sr = venom_slider_create();
    if (sr.ok) {
        VenomSlider* brightness = (VenomSlider*)sr.value;
        venom_slider_set_range(brightness, 0.0f, 1.0f);
        venom_slider_set_value(brightness, g_state.brightness);
        venom_slider_set_on_change(brightness, on_brightness_change, NULL);
        brightness->fill_color = venom_color_rgb(255, 200, 100);
        venom_widget_add_child((VenomWidget*)main_col, (VenomWidget*)brightness);
    }
    
    /* ===== Volume Label ===== */
    lr = venom_label_create("Volume");
    if (lr.ok) {
        venom_label_set_color((VenomLabel*)lr.value, venom_color_rgb(200, 200, 200));
        venom_widget_add_child((VenomWidget*)main_col, (VenomWidget*)lr.value);
    }
    
    /* ===== Volume Slider ===== */
    sr = venom_slider_create();
    if (sr.ok) {
        VenomSlider* volume = (VenomSlider*)sr.value;
        venom_slider_set_range(volume, 0.0f, 1.0f);
        venom_slider_set_value(volume, g_state.volume);
        venom_slider_set_on_change(volume, on_volume_change, NULL);
        volume->fill_color = venom_color_rgb(100, 200, 150);
        venom_widget_add_child((VenomWidget*)main_col, (VenomWidget*)volume);
    }
    
    /* ===== Battery Progress ===== */
    lr = venom_label_create("Battery");
    if (lr.ok) {
        venom_label_set_color((VenomLabel*)lr.value, venom_color_rgb(200, 200, 200));
        venom_widget_add_child((VenomWidget*)main_col, (VenomWidget*)lr.value);
    }
    
    VenomResultPtr pr = venom_progress_create();
    if (pr.ok) {
        VenomProgressBar* battery = (VenomProgressBar*)pr.value;
        venom_progress_set_value(battery, g_state.battery_percent / 100.0f);
        venom_progress_set_colors(battery, 
            venom_color_rgb(60, 60, 70),
            g_state.battery_percent > 20 
                ? venom_color_rgb(80, 200, 120) 
                : venom_color_rgb(255, 80, 80));
        venom_widget_add_child((VenomWidget*)main_col, (VenomWidget*)battery);
    }
    
    /* ===== Buttons Row ===== */
    row_r = venom_container_create_row();
    if (row_r.ok) {
        VenomContainer* row = (VenomContainer*)row_r.value;
        venom_container_set_gap(row, 10);
        
        /* Settings Button */
        VenomResultPtr br = venom_button_create("Settings");
        if (br.ok) {
            venom_widget_add_child((VenomWidget*)row, (VenomWidget*)br.value);
        }
        
        /* About Button */
        br = venom_button_create("About");
        if (br.ok) {
            venom_widget_add_child((VenomWidget*)row, (VenomWidget*)br.value);
        }
        
        venom_widget_add_child((VenomWidget*)main_col, (VenomWidget*)row);
    }
    
    printf("Control Center built with %u widgets\n", ((VenomWidget*)main_col)->children_count);
    
    return (VenomWidget*)main_col;
}

/* ============================================================================
 * MAIN
 * ============================================================================ */

int main(void) {
    setbuf(stdout, NULL);
    
    printf("===========================================\n");
    printf("   VENOMUI Widget-Based Panel Demo\n");
    printf("   Testing Framework Widget Maturity\n");
    printf("===========================================\n\n");
    
    /* Read battery */
    FILE* f = fopen("/sys/class/power_supply/BAT0/capacity", "r");
    if (f) { 
        if (fscanf(f, "%d", &g_state.battery_percent) != 1) 
            g_state.battery_percent = 75;
        fclose(f); 
    }
    
    VenomAppConfig cfg = {
        .title = "Control Center (Widget Demo)",
        .width = 350,
        .height = 500,
        .background = venom_color_rgb(35, 35, 50),
        .build = build_control_center,
    };
    
    return venom_run_app(&cfg);
}
