# Getting Started with VAXPUI

## Installation

### Prerequisites

```bash
# Ubuntu/Debian
sudo apt install build-essential meson ninja-build
sudo apt install libx11-dev libcairo2-dev libpango1.0-dev libpng-dev
```

### Building the Library

```bash
git clone https://github.com/VAXP/vaxpui.git
cd vaxpui
meson setup build
meson compile -C build
```

## Your First Application

### 1. Simple Hello World

```c
#include <vaxp/vaxpui.h>

VaxpWidget* build_app(void* data) {
    (void)data;
    
    return vaxp_center(
        .background = VAXP_LIGHT,
        .children = VAXP_CHILDREN(
            vaxp_text("Hello, VAXPUI!")
        )
    );
}

int main(void) {
    return VAXP_APP(
        .title = "Hello World",
        .width = 400,
        .height = 300,
        .build = build_app
    );
}
```

### 2. Compile and Run

```bash
cc -o hello hello.c $(pkg-config --cflags --libs vaxpui)
./hello
```

## Application Structure

### The VAXP_APP Macro

```c
VAXP_APP(
    .title = "Window Title",    // Window title
    .width = 800,               // Initial width
    .height = 600,              // Initial height
    .build = build_function,    // Widget builder function
    .debug = VAXP_TRUE,        // Enable debug output
)
```

### The Build Function

The build function creates your UI tree:

```c
VaxpWidget* build_app(void* data) {
    // Return the root widget of your UI
    return vaxp_col(
        .gap = 10,
        .children = VAXP_CHILDREN(
            vaxp_text("Title"),
            vaxp_btn("Button"),
        )
    );
}
```

## Widget Basics

### Containers

```c
// Vertical layout
vaxp_col(
    .gap = 10,           // Space between children
    .padding = { 20, 20, 20, 20 },
    .children = VAXP_CHILDREN(...)
)

// Horizontal layout
vaxp_row(
    .gap = 10,
    .children = VAXP_CHILDREN(...)
)

// Centered layout
vaxp_center(
    .children = VAXP_CHILDREN(...)
)
```

### Text

```c
vaxp_text("Hello World")

// With styling (via theme)
VaxpWidget* label = vaxp_text("Styled Text");
```

### Buttons

```c
vaxp_btn("Click Me",
    .on_click = handle_click,
    .color = VAXP_PRIMARY
)

static void handle_click(VaxpButton* btn, void* data) {
    printf("Button clicked!\n");
}
```

### Text Input

```c
vaxp_input(
    .placeholder = "Enter text...",
    .on_change = on_text_changed,
    .on_submit = on_enter_pressed
)

static void on_text_changed(VaxpTextInput* input, const char* text, void* data) {
    printf("Text: %s\n", text);
}
```

## Event Handling

### Mouse Events

```c
// Handled automatically by widgets
// Buttons respond to clicks
// TextInput responds to focus
```

### Keyboard Navigation

```c
// Built-in support:
// - Tab / Shift+Tab: Navigate between widgets
// - Enter / Space: Activate focused button
// - Arrow keys: Navigate in text input
// - Page Up/Down: Scroll in scrollable containers
```

## Next Steps

- [Widgets Guide](widgets.md) - All available widgets
- [State Management](state-management.md) - Managing application state
- [Theming](theming.md) - Customizing appearance

## Optimization

### Constant Widgets

For static UI elements that don't change between rebuilds (like titles, icons, or static buttons), you can use `VAXP_CONST` to prevent unnecessary reallocation.

**Why use it?**
- Prevents destroying and recreating widgets every frame.
- Significantly improves performance in complex apps.
- Internal state of the widget (like scroll position) is preserved.

**Example:**

```c
VaxpWidget* build_app(void* data) {
    return vaxp_center(
        .children = VAXP_CHILDREN(
            /* ✨ This label is created once and reused forever */
            VAXP_CONST(vaxp_text("Static Title", .size = 24)),
            
            /* ⚠️ This label is recreated every build because it changes */
            vaxp_text(dynamic_loop_counter_string),
            
            /* ✨ Complex sub-trees can also be const */
            VAXP_CONST(vaxp_row(
                .gap = 10, 
                .children = VAXP_CHILDREN(
                    vaxp_btn("Static Button 1"),
                    vaxp_btn("Static Button 2")
                )
            ))
        )
    );
}
```

The `VAXP_CONST` macro automatically generates a unique key for the widget based on its location in the code file.

