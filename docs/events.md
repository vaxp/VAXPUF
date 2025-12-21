# Events & Input Handling

VENOMUI provides comprehensive event handling for mouse, keyboard, and window events.

## Event Types

### Window Events

| Event | Description |
|-------|-------------|
| `VENOM_EVENT_WINDOW_CLOSE` | Window close requested |
| `VENOM_EVENT_WINDOW_RESIZE` | Window resized |
| `VENOM_EVENT_WINDOW_FOCUS_IN` | Window gained focus |
| `VENOM_EVENT_WINDOW_FOCUS_OUT` | Window lost focus |
| `VENOM_EVENT_WINDOW_EXPOSE` | Window needs redraw |

### Mouse Events

| Event | Description |
|-------|-------------|
| `VENOM_EVENT_MOUSE_MOVE` | Mouse moved |
| `VENOM_EVENT_MOUSE_ENTER` | Mouse entered window |
| `VENOM_EVENT_MOUSE_LEAVE` | Mouse left window |
| `VENOM_EVENT_MOUSE_BUTTON_DOWN` | Mouse button pressed |
| `VENOM_EVENT_MOUSE_BUTTON_UP` | Mouse button released |
| `VENOM_EVENT_MOUSE_SCROLL` | Mouse wheel scrolled |

### Keyboard Events

| Event | Description |
|-------|-------------|
| `VENOM_EVENT_KEY_DOWN` | Key pressed |
| `VENOM_EVENT_KEY_UP` | Key released |
| `VENOM_EVENT_TEXT_INPUT` | Text input (UTF-8) |

## Mouse Buttons

```c
VENOM_MOUSE_BUTTON_LEFT   // Primary click
VENOM_MOUSE_BUTTON_MIDDLE // Middle click
VENOM_MOUSE_BUTTON_RIGHT  // Context menu
VENOM_MOUSE_BUTTON_X1     // Back
VENOM_MOUSE_BUTTON_X2     // Forward
```

## Key Codes

### Special Keys

```c
VENOM_KEY_ESCAPE
VENOM_KEY_RETURN    // Enter
VENOM_KEY_TAB
VENOM_KEY_BACKSPACE
VENOM_KEY_DELETE
VENOM_KEY_SPACE

// Navigation
VENOM_KEY_LEFT, VENOM_KEY_RIGHT
VENOM_KEY_UP, VENOM_KEY_DOWN
VENOM_KEY_HOME, VENOM_KEY_END
VENOM_KEY_PAGE_UP, VENOM_KEY_PAGE_DOWN

// Function keys
VENOM_KEY_F1 through VENOM_KEY_F12
```

### Letters and Numbers

```c
VENOM_KEY_A through VENOM_KEY_Z  // 'a' to 'z'
VENOM_KEY_0 through VENOM_KEY_9  // '0' to '9'
```

## Key Modifiers

```c
VENOM_KEYMOD_NONE   // No modifier
VENOM_KEYMOD_SHIFT  // Shift key
VENOM_KEYMOD_CTRL   // Control key
VENOM_KEYMOD_ALT    // Alt key
VENOM_KEYMOD_SUPER  // Super/Windows key
VENOM_KEYMOD_CAPS   // Caps Lock
VENOM_KEYMOD_NUM    // Num Lock
```

### Checking Modifiers

```c
if (event->key.modifiers & VENOM_KEYMOD_CTRL) {
    // Ctrl is held
}

if (event->key.modifiers & VENOM_KEYMOD_SHIFT) {
    // Shift is held
}
```

## Widget Event Handling

### Button Click

```c
void on_button_click(VenomButton* btn, void* user_data) {
    printf("Button clicked!\n");
}

venom_btn("Click Me", .on_click = on_button_click)
```

### Text Input Change

```c
void on_text_change(VenomTextInput* input, const char* text, void* data) {
    printf("Text changed: %s\n", text);
}

void on_text_submit(VenomTextInput* input, const char* text, void* data) {
    printf("Submitted: %s\n", text);
}

venom_input(
    .on_change = on_text_change,
    .on_submit = on_text_submit
)
```

## Keyboard Navigation

VENOMUI includes built-in keyboard navigation:

### Focus Navigation

```c
// Tab: Move to next focusable widget
// Shift+Tab: Move to previous focusable widget
```

### Widget Activation

```c
// Enter/Space: Activate focused button
```

### Text Navigation

```c
// Arrow Left/Right: Move cursor
// Home: Move to start
// End: Move to end
// Backspace: Delete before cursor
// Delete: Delete at cursor
// Enter: Submit text
```

### Scrolling

```c
// Page Up: Scroll up one page
// Page Down: Scroll down one page
// Ctrl+Home: Scroll to top
// Ctrl+End: Scroll to bottom
```

## Focus Management

### Setting Focus

```c
// Set focus to a widget
venom_focus_set(widget);

// Get currently focused widget
VenomWidget* focused = venom_focus_get();

// Check if widget has focus
if (venom_focus_has(widget)) {
    // Widget is focused
}
```

### Focus Navigation

```c
// Move focus to next widget
venom_focus_next();

// Move focus to previous widget
venom_focus_prev();
```

### Making Widgets Focusable

```c
widget->focusable = VENOM_TRUE;  // Widget can receive focus
widget->focusable = VENOM_FALSE; // Widget cannot receive focus
```

## Custom Event Handling

### Low-Level Events

For advanced use cases:

```c
// Custom widget event handler
static VenomBool my_widget_on_event(VenomWidget* widget, const VenomEvent* event) {
    switch (event->type) {
        case VENOM_EVENT_MOUSE_BUTTON_DOWN:
            printf("Click at (%d, %d)\n", event->mouse.x, event->mouse.y);
            return VENOM_TRUE;  // Event consumed
            
        case VENOM_EVENT_KEY_DOWN:
            if (event->key.key == VENOM_KEY_ESCAPE) {
                // Handle escape
                return VENOM_TRUE;
            }
            break;
            
        default:
            break;
    }
    
    return VENOM_FALSE;  // Event not consumed
}
```

## See Also

- [Widgets](widgets.md) - Widget event callbacks
- [Internationalization](i18n.md) - Text input handling
