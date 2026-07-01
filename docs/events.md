# Events & Input Handling

VAXPUI provides comprehensive event handling for mouse, keyboard, and window events.

## Event Types

### Window Events

| Event | Description |
|-------|-------------|
| `VAXP_EVENT_WINDOW_CLOSE` | Window close requested |
| `VAXP_EVENT_WINDOW_RESIZE` | Window resized |
| `VAXP_EVENT_WINDOW_FOCUS_IN` | Window gained focus |
| `VAXP_EVENT_WINDOW_FOCUS_OUT` | Window lost focus |
| `VAXP_EVENT_WINDOW_EXPOSE` | Window needs redraw |

### Mouse Events

| Event | Description |
|-------|-------------|
| `VAXP_EVENT_MOUSE_MOVE` | Mouse moved |
| `VAXP_EVENT_MOUSE_ENTER` | Mouse entered window |
| `VAXP_EVENT_MOUSE_LEAVE` | Mouse left window |
| `VAXP_EVENT_MOUSE_BUTTON_DOWN` | Mouse button pressed |
| `VAXP_EVENT_MOUSE_BUTTON_UP` | Mouse button released |
| `VAXP_EVENT_MOUSE_SCROLL` | Mouse wheel scrolled |

### Keyboard Events

| Event | Description |
|-------|-------------|
| `VAXP_EVENT_KEY_DOWN` | Key pressed |
| `VAXP_EVENT_KEY_UP` | Key released |
| `VAXP_EVENT_TEXT_INPUT` | Text input (UTF-8) |

## Mouse Buttons

```c
VAXP_MOUSE_BUTTON_LEFT   // Primary click
VAXP_MOUSE_BUTTON_MIDDLE // Middle click
VAXP_MOUSE_BUTTON_RIGHT  // Context menu
VAXP_MOUSE_BUTTON_X1     // Back
VAXP_MOUSE_BUTTON_X2     // Forward
```

## Key Codes

### Special Keys

```c
VAXP_KEY_ESCAPE
VAXP_KEY_RETURN    // Enter
VAXP_KEY_TAB
VAXP_KEY_BACKSPACE
VAXP_KEY_DELETE
VAXP_KEY_SPACE

// Navigation
VAXP_KEY_LEFT, VAXP_KEY_RIGHT
VAXP_KEY_UP, VAXP_KEY_DOWN
VAXP_KEY_HOME, VAXP_KEY_END
VAXP_KEY_PAGE_UP, VAXP_KEY_PAGE_DOWN

// Function keys
VAXP_KEY_F1 through VAXP_KEY_F12
```

### Letters and Numbers

```c
VAXP_KEY_A through VAXP_KEY_Z  // 'a' to 'z'
VAXP_KEY_0 through VAXP_KEY_9  // '0' to '9'
```

## Key Modifiers

```c
VAXP_KEYMOD_NONE   // No modifier
VAXP_KEYMOD_SHIFT  // Shift key
VAXP_KEYMOD_CTRL   // Control key
VAXP_KEYMOD_ALT    // Alt key
VAXP_KEYMOD_SUPER  // Super/Windows key
VAXP_KEYMOD_CAPS   // Caps Lock
VAXP_KEYMOD_NUM    // Num Lock
```

### Checking Modifiers

```c
if (event->key.modifiers & VAXP_KEYMOD_CTRL) {
    // Ctrl is held
}

if (event->key.modifiers & VAXP_KEYMOD_SHIFT) {
    // Shift is held
}
```

## Widget Event Handling

### Button Click

```c
void on_button_click(VaxpButton* btn, void* user_data) {
    printf("Button clicked!\n");
}

vaxp_btn("Click Me", .on_click = on_button_click)
```

### Text Input Change

```c
void on_text_change(VaxpTextInput* input, const char* text, void* data) {
    printf("Text changed: %s\n", text);
}

void on_text_submit(VaxpTextInput* input, const char* text, void* data) {
    printf("Submitted: %s\n", text);
}

vaxp_input(
    .on_change = on_text_change,
    .on_submit = on_text_submit
)
```

## Keyboard Navigation

VAXPUI includes built-in keyboard navigation:

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
vaxp_focus_set(widget);

// Get currently focused widget
VaxpWidget* focused = vaxp_focus_get();

// Check if widget has focus
if (vaxp_focus_has(widget)) {
    // Widget is focused
}
```

### Focus Navigation

```c
// Move focus to next widget
vaxp_focus_next();

// Move focus to previous widget
vaxp_focus_prev();
```

### Making Widgets Focusable

```c
widget->focusable = VAXP_TRUE;  // Widget can receive focus
widget->focusable = VAXP_FALSE; // Widget cannot receive focus
```

## Custom Event Handling

### Low-Level Events

For advanced use cases:

```c
// Custom widget event handler
static VaxpBool my_widget_on_event(VaxpWidget* widget, const VaxpEvent* event) {
    switch (event->type) {
        case VAXP_EVENT_MOUSE_BUTTON_DOWN:
            printf("Click at (%d, %d)\n", event->mouse.x, event->mouse.y);
            return VAXP_TRUE;  // Event consumed
            
        case VAXP_EVENT_KEY_DOWN:
            if (event->key.key == VAXP_KEY_ESCAPE) {
                // Handle escape
                return VAXP_TRUE;
            }
            break;
            
        default:
            break;
    }
    
    return VAXP_FALSE;  // Event not consumed
}
```

## See Also

- [Widgets](widgets.md) - Widget event callbacks
- [Internationalization](i18n.md) - Text input handling
