# Widgets Guide

VAXPUI provides a comprehensive set of widgets for building desktop UIs.

## Container Widgets

### Column (vaxp_col)

Arranges children vertically.

```c
vaxp_col(
    .gap = 10,                          // Space between children
    .padding = { 10, 10, 10, 10 },      // top, right, bottom, left
    .background = VAXP_LIGHT,          // Background color
    .alignment = VAXP_ALIGN_CENTER,    // Horizontal alignment
    .children = VAXP_CHILDREN(...)
)
```

### Row (vaxp_row)

Arranges children horizontally.

```c
vaxp_row(
    .gap = 16,
    .padding = { 8, 16, 8, 16 },
    .children = VAXP_CHILDREN(
        vaxp_btn("Cancel"),
        vaxp_btn("OK", .color = VAXP_PRIMARY)
    )
)
```

### Center (vaxp_center)

Centers content in available space.

```c
vaxp_center(
    .children = VAXP_CHILDREN(
        vaxp_text("Centered Content")
    )
)
```

### Scrollable (vaxp_scroll)

Allows scrolling of content larger than viewport.

```c
vaxp_scroll(
    .direction = VAXP_SCROLL_VERTICAL,
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

### Button (vaxp_btn)

Clickable button with various styles.

```c
// Simple button
vaxp_btn("Click Me")

// With callback
vaxp_btn("Submit",
    .on_click = handle_submit,
    .color = VAXP_PRIMARY
)

// Callback signature
void handle_submit(VaxpButton* btn, void* user_data) {
    printf("Button clicked!\n");
}
```

**Keyboard:**
- Tab: Focus
- Enter/Space: Activate

### TextInput (vaxp_input)

Single-line text input field.

```c
vaxp_input(
    .placeholder = "Enter username",
    .on_change = on_text_change,
    .on_submit = on_enter_pressed,
    .max_length = 50,
    .password = VAXP_FALSE
)

// Callbacks
void on_text_change(VaxpTextInput* input, const char* text, void* data) {
    printf("Text: %s\n", text);
}

void on_enter_pressed(VaxpTextInput* input, const char* text, void* data) {
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

### Label/Text (vaxp_text)

Displays text with proper Unicode rendering.

```c
vaxp_text("Hello World")
vaxp_text("مرحباً بالعالم")  // Arabic - RTL supported
vaxp_text("こんにちは")       // Japanese
```

### Image (vaxp_image)

Displays images from files.

```c
vaxp_image(
    .src = "/path/to/image.png",
    .fit = VAXP_IMAGE_FIT_CONTAIN,
    .width = 200,
    .height = 150,
    .corner_radius = 8
)
```

**Fit Modes:**
- `VAXP_IMAGE_FIT_CONTAIN` - Scale to fit, letterbox
- `VAXP_IMAGE_FIT_COVER` - Scale to cover, may crop
- `VAXP_IMAGE_FIT_FILL` - Stretch to fill
- `VAXP_IMAGE_FIT_NONE` - Original size
- `VAXP_IMAGE_FIT_SCALE_DOWN` - Like contain, never upscale

---

## State Widgets

### BlocBuilder

Rebuilds UI when state changes.

```c
VAXP_BLOC_BUILDER(
    .cubit = my_cubit,
    .builder = my_builder_function,
    .user_data = optional_data
)

VaxpWidget* my_builder_function(void* state, void* user_data) {
    MyState* s = (MyState*)state;
    return vaxp_text(s->message);
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
widget->layout.padding = (VaxpInsets){ top, right, bottom, left };
widget->layout.margin = (VaxpInsets){ top, right, bottom, left };
```

### Visibility

```c
widget->visible = VAXP_TRUE;   // Show widget
widget->visible = VAXP_FALSE;  // Hide widget
```

### Focus

```c
widget->focusable = VAXP_TRUE;  // Can receive keyboard focus
vaxp_focus_set(widget);         // Set focus to widget
VaxpWidget* focused = vaxp_focus_get();  // Get focused widget
```

---

## Creating Widgets Programmatically

### Direct Creation

```c
VaxpResultPtr result = vaxp_button_create();
if (result.ok) {
    VaxpButton* btn = (VaxpButton*)result.value;
    vaxp_button_set_text(btn, "My Button");
    vaxp_button_set_on_click(btn, my_handler, NULL);
}
```

### Using VAXP_CHILDREN

```c
// Automatically manages child array
.children = VAXP_CHILDREN(
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
