# Getting Started with VENOMUI

## Installation

### Prerequisites

```bash
# Ubuntu/Debian
sudo apt install build-essential meson ninja-build
sudo apt install libx11-dev libcairo2-dev libpango1.0-dev libpng-dev
```

### Building the Library

```bash
git clone https://github.com/VAXP/venomui.git
cd venomui
meson setup build
meson compile -C build
```

## Your First Application

### 1. Simple Hello World

```c
#include <venom/venomui.h>

VenomWidget* build_app(void* data) {
    (void)data;
    
    return venom_center(
        .background = VENOM_LIGHT,
        .children = VENOM_CHILDREN(
            venom_text("Hello, VENOMUI!")
        )
    );
}

int main(void) {
    return VENOM_APP(
        .title = "Hello World",
        .width = 400,
        .height = 300,
        .build = build_app
    );
}
```

### 2. Compile and Run

```bash
cc -o hello hello.c $(pkg-config --cflags --libs venomui)
./hello
```

## Application Structure

### The VENOM_APP Macro

```c
VENOM_APP(
    .title = "Window Title",    // Window title
    .width = 800,               // Initial width
    .height = 600,              // Initial height
    .build = build_function,    // Widget builder function
    .debug = VENOM_TRUE,        // Enable debug output
)
```

### The Build Function

The build function creates your UI tree:

```c
VenomWidget* build_app(void* data) {
    // Return the root widget of your UI
    return venom_col(
        .gap = 10,
        .children = VENOM_CHILDREN(
            venom_text("Title"),
            venom_btn("Button"),
        )
    );
}
```

## Widget Basics

### Containers

```c
// Vertical layout
venom_col(
    .gap = 10,           // Space between children
    .padding = { 20, 20, 20, 20 },
    .children = VENOM_CHILDREN(...)
)

// Horizontal layout
venom_row(
    .gap = 10,
    .children = VENOM_CHILDREN(...)
)

// Centered layout
venom_center(
    .children = VENOM_CHILDREN(...)
)
```

### Text

```c
venom_text("Hello World")

// With styling (via theme)
VenomWidget* label = venom_text("Styled Text");
```

### Buttons

```c
venom_btn("Click Me",
    .on_click = handle_click,
    .color = VENOM_PRIMARY
)

static void handle_click(VenomButton* btn, void* data) {
    printf("Button clicked!\n");
}
```

### Text Input

```c
venom_input(
    .placeholder = "Enter text...",
    .on_change = on_text_changed,
    .on_submit = on_enter_pressed
)

static void on_text_changed(VenomTextInput* input, const char* text, void* data) {
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
