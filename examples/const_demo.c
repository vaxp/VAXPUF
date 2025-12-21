/*
 * VENOMUI - Const Widget System Demo (Simplified API)
 * 
 * Shows Flutter-like VENOM_CONST() syntax.
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
    venom_rebuild();
}

static void decrement(void) {
    AppState s = g_cubit->state;
    s.count--;
    s.rebuild_count++;
    venom_cubit_emit_raw(g_cubit, &s);
    venom_rebuild();
}

/* ============================================================================
 * CALLBACKS
 * ============================================================================ */

static void on_inc(VenomButton* b, void* d) { (void)b; (void)d; increment(); }
static void on_dec(VenomButton* b, void* d) { (void)b; (void)d; decrement(); }

/* ============================================================================
 * BUILD UI - Clean Flutter-like Syntax!
 * ============================================================================ */

static VenomWidget* build_app(void* ud) {
    (void)ud;
    VenomF64 start = get_time_ms();
    
    AppState* s = &g_cubit->state;
    
    /* Dynamic widgets - recreated each build */
    char count_str[32];
    snprintf(count_str, sizeof(count_str), "Count: %d", s->count);
    
    char info_str[64];
    snprintf(info_str, sizeof(info_str), "Rebuilds: %d", s->rebuild_count);
    
    VenomWidget* ui = venom_center(
        .gap = 20,
        .padding = (VenomInsets){40, 40, 40, 40},
        .background = VENOM_DARK,
        .corner_radius = 16,
        .children = VENOM_CHILDREN(
            /* ✨ CONST - Never recreated! */
            VENOM_CONST(venom_text("🔄 Const Widget Demo", .size = 24, .color = VENOM_COLOR_WHITE)),
            VENOM_CONST(venom_text("Widgets with VENOM_CONST are NOT recreated", .size = 12, .color = VENOM_MUTED)),
            
            /* Dynamic - Recreated each build */
            venom_text(count_str, .size = 48, .color = s->count >= 0 ? VENOM_PRIMARY : VENOM_DANGER),
            
            /* ✨ CONST buttons - Never recreated! */
            venom_row(.gap = 20, .children = VENOM_CHILDREN(
                VENOM_CONST(venom_btn("➖", .color = VENOM_DANGER, .on_click = on_dec)),
                VENOM_CONST(venom_btn("➕", .color = VENOM_SUCCESS, .on_click = on_inc))
            )),
            
            /* Dynamic info */
            venom_text(info_str, .size = 14, .color = VENOM_MUTED)
        )
    );
    
    VenomF64 elapsed = get_time_ms() - start;
    printf("⏱️ Build: %.3f ms (Rebuilds: %d)\n", elapsed, s->rebuild_count);
    
    return ui;
}

/* ============================================================================
 * MAIN
 * ============================================================================ */

int main(void) {
    printf("=== VENOMUI Const Widget Demo ===\n");
    printf("Using simplified VENOM_CONST() syntax\n\n");
    
    g_cubit = VENOM_CUBIT_CREATE(App, AppState, { .count = 0, .rebuild_count = 0 });
    if (!g_cubit) {
        fprintf(stderr, "Failed to create cubit\n");
        return 1;
    }
    
    int ret = VENOM_APP(
        .title = "Const Widget Demo",
        .width = 420,
        .height = 380,
        .build = build_app,
        .debug = VENOM_TRUE
    );
    
    venom_cubit_destroy(g_cubit);
    
    printf("\n=== Demo Ended ===\n");
    return ret;
}
