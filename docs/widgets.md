# Widgets Guide

VENOMUI provides a comprehensive set of widgets for building desktop UIs.

## Container Widgets

### Column (venom_col)

Arranges children vertically.

```c
venom_col(
    .gap = 10,                          // Space between children
    .padding = { 10, 10, 10, 10 },      // top, right, bottom, left
    .background = VENOM_LIGHT,          // Background color
    .alignment = VENOM_ALIGN_CENTER,    // Horizontal alignment
    .children = VENOM_CHILDREN(...)
)
```

### Row (venom_row)

Arranges children horizontally.

```c
venom_row(
    .gap = 16,
    .padding = { 8, 16, 8, 16 },
    .children = VENOM_CHILDREN(
        venom_btn("Cancel"),
        venom_btn("OK", .color = VENOM_PRIMARY)
    )
)
```

### Center (venom_center)

Centers content in available space.

```c
venom_center(
    .children = VENOM_CHILDREN(
        venom_text("Centered Content")
    )
)
```

### Scrollable (venom_scroll)

Allows scrolling of content larger than viewport.

```c
venom_scroll(
    .direction = VENOM_SCROLL_VERTICAL,
    .height = 300,
    .content = large_content_widget
)
```

**Features:**
- Mouse wheel scrolling
- Page Up/Down keyboard navigation
- Visible scrollbars
- Smooth scrolling

---

## Interactive Widgets

### Button (venom_btn)

Clickable button with various styles.

```c
// Simple button
venom_btn("Click Me")

// With callback
venom_btn("Submit",
    .on_click = handle_submit,
    .color = VENOM_PRIMARY
)

// Callback signature
void handle_submit(VenomButton* btn, void* user_data) {
    printf("Button clicked!\n");
}
```

**Keyboard:**
- Tab: Focus
- Enter/Space: Activate

### TextInput (venom_input)

Single-line text input field.

```c
venom_input(
    .placeholder = "Enter username",
    .on_change = on_text_change,
    .on_submit = on_enter_pressed,
    .max_length = 50,
    .password = VENOM_FALSE
)

// Callbacks
void on_text_change(VenomTextInput* input, const char* text, void* data) {
    printf("Text: %s\n", text);
}

void on_enter_pressed(VenomTextInput* input, const char* text, void* data) {
    printf("Submitted: %s\n", text);
}
```

**Features:**
- Full Unicode support (Arabic, CJK, etc.)
- Cursor navigation (Arrow keys, Home, End)
- Delete/Backspace (UTF-8 aware)
- Password mode (shows •)

---

## Display Widgets

### Label/Text (venom_text)

Displays text with proper Unicode rendering.

```c
venom_text("Hello World")
venom_text("مرحباً بالعالم")  // Arabic - RTL supported
venom_text("こんにちは")       // Japanese
```

### Image (venom_image)

Displays images from files.

```c
venom_image(
    .src = "/path/to/image.png",
    .fit = VENOM_IMAGE_FIT_CONTAIN,
    .width = 200,
    .height = 150,
    .corner_radius = 8
)
```

**Fit Modes:**
- `VENOM_IMAGE_FIT_CONTAIN` - Scale to fit, letterbox
- `VENOM_IMAGE_FIT_COVER` - Scale to cover, may crop
- `VENOM_IMAGE_FIT_FILL` - Stretch to fill
- `VENOM_IMAGE_FIT_NONE` - Original size
- `VENOM_IMAGE_FIT_SCALE_DOWN` - Like contain, never upscale

---

## State Widgets

### BlocBuilder

Rebuilds UI when state changes.

```c
VENOM_BLOC_BUILDER(
    .cubit = my_cubit,
    .builder = my_builder_function,
    .user_data = optional_data
)

VenomWidget* my_builder_function(void* state, void* user_data) {
    MyState* s = (MyState*)state;
    return venom_text(s->message);
}
```

---

## Widget Properties

### Common Properties

All widgets share these layout properties:

```c
widget->layout.min_width = 100;
widget->layout.min_height = 50;
widget->layout.preferred_width = 200;
widget->layout.preferred_height = 100;
widget->layout.max_width = 400;
widget->layout.max_height = 200;
widget->layout.padding = (VenomInsets){ top, right, bottom, left };
widget->layout.margin = (VenomInsets){ top, right, bottom, left };
```

### Visibility

```c
widget->visible = VENOM_TRUE;   // Show widget
widget->visible = VENOM_FALSE;  // Hide widget
```

### Focus

```c
widget->focusable = VENOM_TRUE;  // Can receive keyboard focus
venom_focus_set(widget);         // Set focus to widget
VenomWidget* focused = venom_focus_get();  // Get focused widget
```

---

## Creating Widgets Programmatically

### Direct Creation

```c
VenomResultPtr result = venom_button_create();
if (result.ok) {
    VenomButton* btn = (VenomButton*)result.value;
    venom_button_set_text(btn, "My Button");
    venom_button_set_on_click(btn, my_handler, NULL);
}
```

### Using VENOM_CHILDREN

```c
// Automatically manages child array
.children = VENOM_CHILDREN(
    widget1,
    widget2,
    widget3
)
```

---

## See Also

- [Getting Started](getting-started.md) - Basic setup
- [State Management](state-management.md) - BLoC pattern
- [Theming](theming.md) - Styling widgets
