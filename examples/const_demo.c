/*
 * VAXPUI - Const Widget System Demo (Simplified API)
 * 
 * Shows Flutter-like VAXP_CONST() syntax.
 */

#include <stdio.h>
#include <vaxp/vaxpui.h>
#include <sys/time.h>

/* ============================================================================
 * STATE
 * ============================================================================ */

typedef struct {
    int count;
    int rebuild_count;
} AppState;

VAXP_DEFINE_CUBIT(App, AppState);
static AppCubit* g_cubit = NULL;

/* ============================================================================
 * TIMING
 * ============================================================================ */

static VaxpF64 get_time_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (VaxpF64)tv.tv_sec * 1000.0 + (VaxpF64)tv.tv_usec / 1000.0;
}

/* ============================================================================
 * ACTIONS
 * ============================================================================ */

static void increment(void) {
    AppState s = g_cubit->state;
    s.count++;
    s.rebuild_count++;
    vaxp_cubit_emit_raw(g_cubit, &s);
    vaxp_rebuild();
}

static void decrement(void) {
    AppState s = g_cubit->state;
    s.count--;
    s.rebuild_count++;
    vaxp_cubit_emit_raw(g_cubit, &s);
    vaxp_rebuild();
}

/* ============================================================================
 * CALLBACKS
 * ============================================================================ */

static void on_inc(VaxpButton* b, void* d) { (void)b; (void)d; increment(); }
static void on_dec(VaxpButton* b, void* d) { (void)b; (void)d; decrement(); }

/* ============================================================================
 * BUILD UI - Clean Flutter-like Syntax!
 * ============================================================================ */

static VaxpWidget* build_app(void* ud) {
    (void)ud;
    VaxpF64 start = get_time_ms();
    
    AppState* s = &g_cubit->state;
    
    /* Dynamic widgets - recreated each build */
    char count_str[32];
    snprintf(count_str, sizeof(count_str), "Count: %d", s->count);
    
    char info_str[64];
    snprintf(info_str, sizeof(info_str), "Rebuilds: %d", s->rebuild_count);
    
    VaxpWidget* ui = vaxp_center(
        .gap = 20,
        .padding = (VaxpInsets){40, 40, 40, 40},
        .background = VAXP_DARK,
        .corner_radius = 16,
        .children = VAXP_CHILDREN(
            /* ✨ CONST - Never recreated! */
            VAXP_CONST(vaxp_text("🔄 Const Widget Demo", .size = 24, .color = VAXP_COLOR_WHITE)),
            VAXP_CONST(vaxp_text("Widgets with VAXP_CONST are NOT recreated", .size = 12, .color = VAXP_MUTED)),
            
            /* Dynamic - Recreated each build */
            vaxp_text(count_str, .size = 48, .color = s->count >= 0 ? VAXP_PRIMARY : VAXP_DANGER),
            
            /* ✨ CONST buttons - Never recreated! */
            vaxp_row(.gap = 20, .children = VAXP_CHILDREN(
                VAXP_CONST(vaxp_btn("➖", .color = VAXP_DANGER, .on_click = on_dec)),
                VAXP_CONST(vaxp_btn("➕", .color = VAXP_SUCCESS, .on_click = on_inc))
            )),
            
            /* Dynamic info */
            vaxp_text(info_str, .size = 14, .color = VAXP_MUTED)
        )
    );
    
    VaxpF64 elapsed = get_time_ms() - start;
    printf("⏱️ Build: %.3f ms (Rebuilds: %d)\n", elapsed, s->rebuild_count);
    
    return ui;
}

/* ============================================================================
 * MAIN
 * ============================================================================ */

int main(void) {
    printf("=== VAXPUI Const Widget Demo ===\n");
    printf("Using simplified VAXP_CONST() syntax\n\n");
    
    g_cubit = VAXP_CUBIT_CREATE(App, AppState, { .count = 0, .rebuild_count = 0 });
    if (!g_cubit) {
        fprintf(stderr, "Failed to create cubit\n");
        return 1;
    }
    
    int ret = VAXP_APP(
        .title = "Const Widget Demo",
        .width = 420,
        .height = 380,
        .build = build_app,
        .debug = VAXP_TRUE
    );
    
    vaxp_cubit_destroy(g_cubit);
    
    printf("\n=== Demo Ended ===\n");
    return ret;
}
