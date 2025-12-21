/*
 * VENOMUI - Counter Example with BLoC State Management
 * 
 * Demonstrates Cubit pattern for state management.
 */

#include <stdio.h>
#include <venom/venomui.h>

/* ============================================================================
 * STATE DEFINITION
 * ============================================================================ */

typedef struct {
    int count;
} CounterState;

/* Define Cubit type */
VENOM_DEFINE_CUBIT(Counter, CounterState);

/* Global cubit instance */
static CounterCubit* counter_cubit = NULL;

/* ============================================================================
 * CUBIT METHODS
 * ============================================================================ */

void counter_increment(void) {
    CounterState current = VENOM_CUBIT_STATE(counter_cubit, CounterState);
    VENOM_CUBIT_EMIT(counter_cubit, CounterState, { .count = current.count + 1 });
}

void counter_decrement(void) {
    CounterState current = VENOM_CUBIT_STATE(counter_cubit, CounterState);
    VENOM_CUBIT_EMIT(counter_cubit, CounterState, { .count = current.count - 1 });
}

void counter_reset(void) {
    VENOM_CUBIT_EMIT(counter_cubit, CounterState, { .count = 0 });
}

/* ============================================================================
 * UI CALLBACKS
 * ============================================================================ */

static void on_increment(VenomButton* btn, void* data) {
    (void)btn; (void)data;
    counter_increment();
    printf("Increment! Count is now: %d\n", VENOM_CUBIT_STATE(counter_cubit, CounterState).count);
}

static void on_decrement(VenomButton* btn, void* data) {
    (void)btn; (void)data;
    counter_decrement();
    printf("Decrement! Count is now: %d\n", VENOM_CUBIT_STATE(counter_cubit, CounterState).count);
}

static void on_reset(VenomButton* btn, void* data) {
    (void)btn; (void)data;
    counter_reset();
    printf("Reset! Count is now: 0\n");
}

/* ============================================================================
 * BUILDER FUNCTION (called when state changes)
 * ============================================================================ */

static VenomWidget* counter_builder(void* state_ptr, void* user_data) {
    (void)user_data;
    CounterState* state = (CounterState*)state_ptr;
    
    /* Create formatted text */
    char count_text[32];
    snprintf(count_text, sizeof(count_text), "Count: %d", state->count);
    
    VenomWidget* count_label = venom_text(count_text);
    if (count_label) {
        venom_label_set_font_size((VenomLabel*)count_label, 48);
        venom_label_set_color((VenomLabel*)count_label, 
            state->count >= 0 ? VENOM_PRIMARY : VENOM_DANGER);
    }
    
    return count_label;
}

/* ============================================================================
 * MAIN BUILD FUNCTION
 * ============================================================================ */

static VenomWidget* build_app(void* user_data) {
    (void)user_data;
    
    /* Create BlocBuilder for counter display */
    VenomResultPtr builder_result = venom_bloc_builder_create(
        counter_cubit, counter_builder, NULL
    );
    VenomWidget* counter_display = builder_result.ok ? builder_result.value : NULL;
    
    /* Create buttons */
    VenomWidget* btn_dec = venom_btn("-", .color = VENOM_DANGER, .on_click = on_decrement);
    VenomWidget* btn_reset = venom_btn("Reset", .color = VENOM_MUTED, .on_click = on_reset);
    VenomWidget* btn_inc = venom_btn("+", .color = VENOM_SUCCESS, .on_click = on_increment);
    
    VenomWidget* button_row = venom_row(
        .gap = 15,
        .children = VENOM_CHILDREN(btn_dec, btn_reset, btn_inc)
    );
    
    /* Main layout */
    return venom_center(
        .gap = 30,
        .padding = { 50, 50, 50, 50 },
        .background = VENOM_LIGHT,
        .corner_radius = 20,
        .children = VENOM_CHILDREN(
            venom_text("Counter App"),
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
    counter_cubit = VENOM_CUBIT_CREATE(Counter, CounterState, { .count = 0 });
    if (!counter_cubit) {
        fprintf(stderr, "Failed to create counter cubit\n");
        return 1;
    }
    
    printf("Counter App - BLoC State Management Demo\n");
    printf("Click +/- buttons or Reset\n");
    
    int result = VENOM_APP(
        .title = "VENOMUI Counter - BLoC Demo",
        .width = 400,
        .height = 350,
        .build = build_app,
        .debug = VENOM_TRUE
    );
    
    /* Cleanup */
    venom_cubit_destroy(counter_cubit);
    
    return result;
}
