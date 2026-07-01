# State Management in VAXPUI

VAXPUI uses the **BLoC/Cubit pattern** for state management, inspired by Flutter's BLoC library.

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
// Define the cubit with VAXP_DEFINE_CUBIT
VAXP_DEFINE_CUBIT(Counter, CounterState,
    VAXP_CUBIT_ACTION(increment, counter_increment),
    VAXP_CUBIT_ACTION(decrement, counter_decrement),
    VAXP_CUBIT_ACTION(reset, counter_reset)
);

// Create instance with initial state
VaxpCubit* counter_cubit = VAXP_CUBIT_CREATE(Counter, CounterState, { .count = 0 });
```

## Using the Cubit

### Emitting New State

```c
// Call actions to emit new state
VAXP_CUBIT_EMIT(counter_cubit, Counter, increment);
VAXP_CUBIT_EMIT(counter_cubit, Counter, decrement);
VAXP_CUBIT_EMIT(counter_cubit, Counter, reset);
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
void on_counter_changed(VaxpCubit* cubit, void* state, void* user_data) {
    CounterState* counter_state = (CounterState*)state;
    printf("Counter changed to: %d\n", counter_state->count);
}

vaxp_cubit_add_listener(counter_cubit, on_counter_changed, NULL);
```

## BlocBuilder Widget

Connect state to UI with `VAXP_BLOC_BUILDER`:

```c
VaxpWidget* build_counter_display(void* state, void* user_data) {
    CounterState* counter_state = (CounterState*)state;
    
    char text[32];
    snprintf(text, sizeof(text), "Count: %d", counter_state->count);
    
    return vaxp_text(text);
}

// In your build function:
VaxpWidget* counter_view = VAXP_BLOC_BUILDER(
    .cubit = counter_cubit,
    .builder = build_counter_display,
    .user_data = NULL
);
```

## Complete Example

```c
#include <vaxp/vaxpui.h>

// State
typedef struct { int count; } CounterState;

// Actions
static CounterState increment(CounterState s) { return (CounterState){ s.count + 1 }; }
static CounterState decrement(CounterState s) { return (CounterState){ s.count - 1 }; }

// Define cubit
VAXP_DEFINE_CUBIT(Counter, CounterState,
    VAXP_CUBIT_ACTION(increment, increment),
    VAXP_CUBIT_ACTION(decrement, decrement)
);

static VaxpCubit* counter_cubit;

// Button handlers
static void on_increment(VaxpButton* btn, void* data) {
    (void)btn; (void)data;
    VAXP_CUBIT_EMIT(counter_cubit, Counter, increment);
}

static void on_decrement(VaxpButton* btn, void* data) {
    (void)btn; (void)data;
    VAXP_CUBIT_EMIT(counter_cubit, Counter, decrement);
}

// Builder for counter display
VaxpWidget* counter_builder(void* state, void* data) {
    CounterState* s = (CounterState*)state;
    char text[32];
    snprintf(text, sizeof(text), "%d", s->count);
    return vaxp_text(text);
}

// Main build function
VaxpWidget* build_app(void* data) {
    return vaxp_center(
        .gap = 20,
        .children = VAXP_CHILDREN(
            vaxp_text("Counter Example"),
            VAXP_BLOC_BUILDER(
                .cubit = counter_cubit,
                .builder = counter_builder
            ),
            vaxp_row(
                .gap = 10,
                .children = VAXP_CHILDREN(
                    vaxp_btn("-", .on_click = on_decrement),
                    vaxp_btn("+", .on_click = on_increment)
                )
            )
        )
    );
}

int main(void) {
    // Create cubit with initial state
    counter_cubit = VAXP_CUBIT_CREATE(Counter, CounterState, { .count = 0 });
    
    int result = VAXP_APP(
        .title = "Counter",
        .width = 300,
        .height = 200,
        .build = build_app
    );
    
    // Cleanup
    vaxp_cubit_destroy(counter_cubit);
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
VaxpCubit* auth_cubit;      // Authentication state
VaxpCubit* settings_cubit;  // App settings
VaxpCubit* data_cubit;      // Data/content
```

### 4. Clean Up Resources

```c
// Always destroy cubits when done
vaxp_cubit_destroy(my_cubit);
```

## See Also

- [Widgets Guide](widgets.md) - BlocBuilder widget details
- [Getting Started](getting-started.md) - Basic setup
