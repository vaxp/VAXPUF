/*
 * VAXPUI - Counter Example with BLoC State Management
 * 
 * Demonstrates Cubit pattern for state management.
 */

#include <stdio.h>
#include <vaxp/vaxpui.h>

/* ============================================================================
 * STATE DEFINITION
 * ============================================================================ */

typedef struct {
    int count;
} CounterState;

/* Define Cubit type */
VAXP_DEFINE_CUBIT(Counter, CounterState);

/* Global cubit instance */
static CounterCubit* counter_cubit = NULL;

/* ============================================================================
 * CUBIT METHODS
 * ============================================================================ */

void counter_increment(void) {
    CounterState current = VAXP_CUBIT_STATE(counter_cubit, CounterState);
    VAXP_CUBIT_EMIT(counter_cubit, CounterState, { .count = current.count + 1 });
}

void counter_decrement(void) {
    CounterState current = VAXP_CUBIT_STATE(counter_cubit, CounterState);
    VAXP_CUBIT_EMIT(counter_cubit, CounterState, { .count = current.count - 1 });
}

void counter_reset(void) {
    VAXP_CUBIT_EMIT(counter_cubit, CounterState, { .count = 0 });
}

/* ============================================================================
 * UI CALLBACKS
 * ============================================================================ */

static void on_increment(VaxpButton* btn, void* data) {
    (void)btn; (void)data;
    counter_increment();
    printf("Increment! Count is now: %d\n", VAXP_CUBIT_STATE(counter_cubit, CounterState).count);
}

static void on_decrement(VaxpButton* btn, void* data) {
    (void)btn; (void)data;
    counter_decrement();
    printf("Decrement! Count is now: %d\n", VAXP_CUBIT_STATE(counter_cubit, CounterState).count);
}

static void on_reset(VaxpButton* btn, void* data) {
    (void)btn; (void)data;
    counter_reset();
    printf("Reset! Count is now: 0\n");
}

/* ============================================================================
 * BUILDER FUNCTION (called when state changes)
 * ============================================================================ */

static VaxpWidget* counter_builder(void* state_ptr, void* user_data) {
    (void)user_data;
    CounterState* state = (CounterState*)state_ptr;
    
    /* Create formatted text */
    char count_text[32];
    snprintf(count_text, sizeof(count_text), "Count: %d", state->count);
    
    VaxpWidget* count_label = vaxp_text(count_text);
    if (count_label) {
        vaxp_label_set_font_size((VaxpLabel*)count_label, 48);
        vaxp_label_set_color((VaxpLabel*)count_label, 
            state->count >= 0 ? VAXP_PRIMARY : VAXP_DANGER);
    }
    
    return count_label;
}

/* ============================================================================
 * MAIN BUILD FUNCTION
 * ============================================================================ */

static VaxpWidget* build_app(void* user_data) {
    (void)user_data;
    
    /* Create BlocBuilder for counter display */
    VaxpResultPtr builder_result = vaxp_bloc_builder_create(
        counter_cubit, counter_builder, NULL
    );
    VaxpWidget* counter_display = builder_result.ok ? builder_result.value : NULL;
    
    /* Create buttons */
    VaxpWidget* btn_dec = vaxp_btn("-", .color = VAXP_DANGER, .on_click = on_decrement);
    VaxpWidget* btn_reset = vaxp_btn("Reset", .color = VAXP_MUTED, .on_click = on_reset);
    VaxpWidget* btn_inc = vaxp_btn("+", .color = VAXP_SUCCESS, .on_click = on_increment);
    
    VaxpWidget* button_row = vaxp_row(
        .gap = 15,
        .children = VAXP_CHILDREN(btn_dec, btn_reset, btn_inc)
    );
    
    /* Main layout */
    return vaxp_center(
        .gap = 30,
        .padding = { 50, 50, 50, 50 },
        .background = VAXP_LIGHT,
        .corner_radius = 20,
        .children = VAXP_CHILDREN(
            vaxp_text("Counter App"),
            counter_display,
            button_row
        )
    );
}

/* ============================================================================
 * MAIN
 * ============================================================================ */

int main(void) {
    /* Create cubit with initial state */
    counter_cubit = VAXP_CUBIT_CREATE(Counter, CounterState, { .count = 0 });
    if (!counter_cubit) {
        fprintf(stderr, "Failed to create counter cubit\n");
        return 1;
    }
    
    printf("Counter App - BLoC State Management Demo\n");
    printf("Click +/- buttons or Reset\n");
    
    int result = VAXP_APP(
        .title = "VAXPUI Counter - BLoC Demo",
        .width = 400,
        .height = 350,
        .build = build_app,
        .debug = VAXP_TRUE
    );
    
    /* Cleanup */
    vaxp_cubit_destroy(counter_cubit);
    
    return result;
}
