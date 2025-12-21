# State Management in VENOMUI

VENOMUI uses the **BLoC/Cubit pattern** for state management, inspired by Flutter's BLoC library.

## Overview

```
┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│   Events    │ ──▶ │   Cubit     │ ──▶ │   State     │
│  (Actions)  │     │  (Logic)    │     │   (Data)    │
└─────────────┘     └─────────────┘     └─────────────┘
                           │
                           ▼
                    ┌─────────────┐
                    │    View     │
                    │  (Widgets)  │
                    └─────────────┘
```

## Creating a Cubit

### 1. Define Your State

```c
// Counter state
typedef struct {
    int count;
} CounterState;
```

### 2. Define State Actions

```c
// Action functions that modify state
static CounterState counter_increment(CounterState state) {
    return (CounterState){ .count = state.count + 1 };
}

static CounterState counter_decrement(CounterState state) {
    return (CounterState){ .count = state.count - 1 };
}

static CounterState counter_reset(CounterState state) {
    (void)state;
    return (CounterState){ .count = 0 };
}
```

### 3. Create the Cubit

```c
// Define the cubit with VENOM_DEFINE_CUBIT
VENOM_DEFINE_CUBIT(Counter, CounterState,
    VENOM_CUBIT_ACTION(increment, counter_increment),
    VENOM_CUBIT_ACTION(decrement, counter_decrement),
    VENOM_CUBIT_ACTION(reset, counter_reset)
);

// Create instance with initial state
VenomCubit* counter_cubit = VENOM_CUBIT_CREATE(Counter, CounterState, { .count = 0 });
```

## Using the Cubit

### Emitting New State

```c
// Call actions to emit new state
VENOM_CUBIT_EMIT(counter_cubit, Counter, increment);
VENOM_CUBIT_EMIT(counter_cubit, Counter, decrement);
VENOM_CUBIT_EMIT(counter_cubit, Counter, reset);
```

### Reading Current State

```c
// Get current state
CounterState state = *(CounterState*)counter_cubit->state;
printf("Count: %d\n", state.count);
```

### Listening to State Changes

```c
// Add a listener
void on_counter_changed(VenomCubit* cubit, void* state, void* user_data) {
    CounterState* counter_state = (CounterState*)state;
    printf("Counter changed to: %d\n", counter_state->count);
}

venom_cubit_add_listener(counter_cubit, on_counter_changed, NULL);
```

## BlocBuilder Widget

Connect state to UI with `VENOM_BLOC_BUILDER`:

```c
VenomWidget* build_counter_display(void* state, void* user_data) {
    CounterState* counter_state = (CounterState*)state;
    
    char text[32];
    snprintf(text, sizeof(text), "Count: %d", counter_state->count);
    
    return venom_text(text);
}

// In your build function:
VenomWidget* counter_view = VENOM_BLOC_BUILDER(
    .cubit = counter_cubit,
    .builder = build_counter_display,
    .user_data = NULL
);
```

## Complete Example

```c
#include <venom/venomui.h>

// State
typedef struct { int count; } CounterState;

// Actions
static CounterState increment(CounterState s) { return (CounterState){ s.count + 1 }; }
static CounterState decrement(CounterState s) { return (CounterState){ s.count - 1 }; }

// Define cubit
VENOM_DEFINE_CUBIT(Counter, CounterState,
    VENOM_CUBIT_ACTION(increment, increment),
    VENOM_CUBIT_ACTION(decrement, decrement)
);

static VenomCubit* counter_cubit;

// Button handlers
static void on_increment(VenomButton* btn, void* data) {
    (void)btn; (void)data;
    VENOM_CUBIT_EMIT(counter_cubit, Counter, increment);
}

static void on_decrement(VenomButton* btn, void* data) {
    (void)btn; (void)data;
    VENOM_CUBIT_EMIT(counter_cubit, Counter, decrement);
}

// Builder for counter display
VenomWidget* counter_builder(void* state, void* data) {
    CounterState* s = (CounterState*)state;
    char text[32];
    snprintf(text, sizeof(text), "%d", s->count);
    return venom_text(text);
}

// Main build function
VenomWidget* build_app(void* data) {
    return venom_center(
        .gap = 20,
        .children = VENOM_CHILDREN(
            venom_text("Counter Example"),
            VENOM_BLOC_BUILDER(
                .cubit = counter_cubit,
                .builder = counter_builder
            ),
            venom_row(
                .gap = 10,
                .children = VENOM_CHILDREN(
                    venom_btn("-", .on_click = on_decrement),
                    venom_btn("+", .on_click = on_increment)
                )
            )
        )
    );
}

int main(void) {
    // Create cubit with initial state
    counter_cubit = VENOM_CUBIT_CREATE(Counter, CounterState, { .count = 0 });
    
    int result = VENOM_APP(
        .title = "Counter",
        .width = 300,
        .height = 200,
        .build = build_app
    );
    
    // Cleanup
    venom_cubit_destroy(counter_cubit);
    return result;
}
```

## Best Practices

### 1. Keep State Immutable

```c
// Good: Return new state
static CounterState increment(CounterState state) {
    return (CounterState){ .count = state.count + 1 };
}

// Bad: Modify in place (don't do this!)
// state.count++;
// return state;
```

### 2. Separate Business Logic

```c
// Keep complex logic in action functions
static TodoState add_todo(TodoState state, const char* title) {
    // Complex logic here
    TodoState new_state = copy_state(state);
    add_item(&new_state, title);
    return new_state;
}
```

### 3. Use Multiple Cubits

```c
// Separate concerns into different cubits
VenomCubit* auth_cubit;      // Authentication state
VenomCubit* settings_cubit;  // App settings
VenomCubit* data_cubit;      // Data/content
```

### 4. Clean Up Resources

```c
// Always destroy cubits when done
venom_cubit_destroy(my_cubit);
```

## See Also

- [Widgets Guide](widgets.md) - BlocBuilder widget details
- [Getting Started](getting-started.md) - Basic setup
