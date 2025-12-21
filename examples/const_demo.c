/*
 * VENOMUI - Const Widget System Demo
 * 
 * Demonstrates const widgets that persist across rebuilds.
 */

#include <stdio.h>
#include <venom/venomui.h>
#include <sys/time.h>

/* ============================================================================
 * STATE
 * ============================================================================ */

typedef struct {
    int count;
    int rebuild_count;
} AppState;

VENOM_DEFINE_CUBIT(App, AppState);
static AppCubit* g_cubit = NULL;

/* ============================================================================
 * TIMING
 * ============================================================================ */

static VenomF64 get_time_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (VenomF64)tv.tv_sec * 1000.0 + (VenomF64)tv.tv_usec / 1000.0;
}

/* ============================================================================
 * ACTIONS
 * ============================================================================ */

static void increment(void) {
    AppState s = g_cubit->state;
    s.count++;
    s.rebuild_count++;
    venom_cubit_emit_raw(g_cubit, &s);
    venom_rebuild();  /* Trigger UI update */
}

static void decrement(void) {
    AppState s = g_cubit->state;
    s.count--;
    s.rebuild_count++;
    venom_cubit_emit_raw(g_cubit, &s);
    venom_rebuild();  /* Trigger UI update */
}

/* ============================================================================
 * CALLBACKS
 * ============================================================================ */

static void on_inc(VenomButton* b, void* d) { (void)b; (void)d; increment(); }
static void on_dec(VenomButton* b, void* d) { (void)b; (void)d; decrement(); }

/* ============================================================================
 * BUILD UI (uses const widgets)
 * ============================================================================ */

static VenomWidget* build_app(void* ud) {
    (void)ud;
    VenomF64 start = get_time_ms();
    
    AppState* s = &g_cubit->state;
    
    /* ========== CONST WIDGETS (reused across rebuilds) ========== */
    
    /* Title - const, never changes */
    VenomWidget* title = venom_get_const_widget("title");
    if (!title) {
        title = venom_text("🔄 Const Widget Demo", .size = 24, .color = VENOM_COLOR_WHITE);
        venom_widget_set_const(title, VENOM_TRUE);
        venom_widget_set_const_key(title, "title");
        printf("📦 Created NEW title widget\n");
    } else {
        venom_ref(title);
        printf("♻️ REUSED title widget\n");
    }
    
    /* Subtitle - const */
    VenomWidget* subtitle = venom_get_const_widget("subtitle");
    if (!subtitle) {
        subtitle = venom_text("Widgets marked const are NOT recreated", 
                              .size = 12, .color = venom_color_rgb(150, 150, 170));
        venom_widget_set_const(subtitle, VENOM_TRUE);
        venom_widget_set_const_key(subtitle, "subtitle");
        printf("📦 Created NEW subtitle widget\n");
    } else {
        venom_ref(subtitle);
        printf("♻️ REUSED subtitle widget\n");
    }
    
    /* Buttons - const (they don't change) */
    VenomWidget* btn_dec = venom_get_const_widget("btn_dec");
    if (!btn_dec) {
        btn_dec = venom_btn("➖", .color = VENOM_DANGER, .on_click = on_dec);
        venom_widget_set_const(btn_dec, VENOM_TRUE);
        venom_widget_set_const_key(btn_dec, "btn_dec");
        printf("📦 Created NEW decrement button\n");
    } else {
        venom_ref(btn_dec);
        printf("♻️ REUSED decrement button\n");
    }
    
    VenomWidget* btn_inc = venom_get_const_widget("btn_inc");
    if (!btn_inc) {
        btn_inc = venom_btn("➕", .color = VENOM_SUCCESS, .on_click = on_inc);
        venom_widget_set_const(btn_inc, VENOM_TRUE);
        venom_widget_set_const_key(btn_inc, "btn_inc");
        printf("📦 Created NEW increment button\n");
    } else {
        venom_ref(btn_inc);
        printf("♻️ REUSED increment button\n");
    }
    
    /* ========== DYNAMIC WIDGETS (recreated each time) ========== */
    
    /* Count display - dynamic, changes with state */
    char count_str[32];
    snprintf(count_str, sizeof(count_str), "Count: %d", s->count);
    VenomWidget* count_label = venom_text(count_str, .size = 48, 
                                           .color = s->count >= 0 ? VENOM_PRIMARY : VENOM_DANGER);
    printf("🔄 Created DYNAMIC count label\n");
    
    /* Rebuild info - dynamic */
    char info_str[64];
    snprintf(info_str, sizeof(info_str), "Rebuilds: %d", s->rebuild_count);
    VenomWidget* info_label = venom_text(info_str, .size = 14, .color = VENOM_MUTED);
    printf("🔄 Created DYNAMIC rebuild info\n");
    
    /* Button row */
    VenomWidget* btn_row = venom_row(.gap = 20, 
                                      .children = (VenomWidget*[]){btn_dec, btn_inc, NULL});
    
    VenomF64 elapsed = get_time_ms() - start;
    printf("⏱️ Build took: %.3f ms\n\n", elapsed);
    
    return venom_center(
        .gap = 20,
        .padding = (VenomInsets){40, 40, 40, 40},
        .background = VENOM_DARK,
        .corner_radius = 16,
        .children = (VenomWidget*[]){
            title,
            subtitle,
            count_label,
            btn_row,
            info_label,
            NULL
        }
    );
}

/* ============================================================================
 * MAIN
 * ============================================================================ */

int main(void) {
    printf("=== VENOMUI Const Widget Demo ===\n\n");
    printf("Legend:\n");
    printf("  📦 = New widget created\n");
    printf("  ♻️ = Const widget reused\n");
    printf("  🔄 = Dynamic widget (always recreated)\n\n");
    
    g_cubit = VENOM_CUBIT_CREATE(App, AppState, { .count = 0, .rebuild_count = 0 });
    if (!g_cubit) {
        fprintf(stderr, "Failed to create cubit\n");
        return 1;
    }
    
    int ret = VENOM_APP(
        .title = "Const Widget Demo",
        .width = 400,
        .height = 350,
        .build = build_app,
        .debug = VENOM_TRUE
    );
    
    venom_cubit_destroy(g_cubit);
    
    printf("\n=== Demo Ended ===\n");
    return ret;
}
