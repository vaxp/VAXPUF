# VAXPUI API Cheat Sheet

## Widget Creation

| Widget | Macro | Create Function |
|--------|-------|-----------------|
| Button | `vaxp_button(...)` | `vaxp_button_create()` |
| Label | `vaxp_label(...)` | `vaxp_label_create()` |
| Container | `vaxp_container(...)` | `vaxp_container_create()` |
| TextInput | `vaxp_text_input(...)` | `vaxp_text_input_create()` |
| Checkbox | `vaxp_checkbox(...)` | `vaxp_checkbox_create()` |
| Switch | `vaxp_switch(...)` | `vaxp_switch_create()` |
| Slider | `vaxp_slider(...)` | `vaxp_slider_create()` |
| Dropdown | `vaxp_dropdown(...)` | `vaxp_dropdown_create()` |
| Card | `vaxp_card(...)` | `vaxp_card_create()` |
| Avatar | `vaxp_avatar(...)` | `vaxp_avatar_create()` |
| Icon | `vaxp_icon(...)` | `vaxp_icon_create()` |
| Badge | `vaxp_badge(...)` | `vaxp_badge_create()` |
| Chip | `vaxp_chip(...)` | `vaxp_chip_create()` |
| Tooltip | `vaxp_tooltip(...)` | `vaxp_tooltip_create()` |
| Notification | `vaxp_notification(...)` | `vaxp_notification_create()` |
| Snackbar | `vaxp_snackbar(...)` | `vaxp_snackbar_create()` |
| Spinner | `vaxp_spinner(...)` | `vaxp_spinner_create()` |
| ProgressBar | `vaxp_progress_bar(...)` | `vaxp_progress_bar_create()` |
| Rating | `vaxp_rating(...)` | `vaxp_rating_create()` |
| Link | `vaxp_link(...)` | `vaxp_link_create()` |
| Skeleton | `vaxp_skeleton(...)` | `vaxp_skeleton_create()` |
| SearchBar | `vaxp_search_bar(...)` | `vaxp_search_bar_create()` |
| NumberInput | `vaxp_number_input(...)` | `vaxp_number_input_create()` |
| ToggleButton | `vaxp_toggle_button(...)` | `vaxp_toggle_button_create()` |
| ColorSwatch | `vaxp_color_swatch(...)` | `vaxp_color_swatch_create()` |
| SplitPane | `vaxp_split_pane(...)` | `vaxp_split_pane_create()` |
| TextArea | `vaxp_text_area(...)` | `vaxp_text_area_create()` |
| Popover | `vaxp_popover(...)` | `vaxp_popover_create()` |
| Calendar | `vaxp_calendar(...)` | `vaxp_calendar_create()` |
| DatePicker | `vaxp_date_picker(...)` | `vaxp_date_picker_create()` |
| TimePicker | `vaxp_time_picker(...)` | `vaxp_time_picker_create()` |
| ColorPicker | `vaxp_color_picker(...)` | `vaxp_color_picker_create()` |
| FileChooser | `vaxp_file_chooser(...)` | `vaxp_file_chooser_create()` |

## Layout

```c
/* Horizontal Row */
vaxp_container(.direction = VAXP_DIRECTION_HORIZONTAL)

/* Vertical Column */
vaxp_container(.direction = VAXP_DIRECTION_VERTICAL)

/* Grid */
vaxp_container(.direction = VAXP_DIRECTION_GRID, .columns = 3)

/* Spacing */
vaxp_container(.spacing = 16.0f)

/* Padding */
vaxp_container(.padding = (VaxpInsets){ 20, 20, 20, 20 })

/* Alignment */
vaxp_container(.alignment = VAXP_ALIGN_CENTER)  /* START, CENTER, END */
```

## Colors

```c
/* Basic Colors */
VAXP_COLOR_RED          VAXP_COLOR_GREEN
VAXP_COLOR_BLUE         VAXP_COLOR_WHITE
VAXP_COLOR_BLACK        VAXP_COLOR_TRANSPARENT

/* Custom */
(VaxpColor){ r, g, b, a }  /* 0-255 each */
```

## Common APIs

```c
/* Reference Counting */
vaxp_ref(widget);
vaxp_unref(widget);

/* Visibility */
widget->visible = VAXP_TRUE;
widget->enabled = VAXP_FALSE;

/* Layout */
widget->layout.flex = 1.0f;
widget->layout.preferred_width = 200.0f;
widget->layout.preferred_height = 100.0f;

/* Invalidate */
vaxp_widget_invalidate(widget);
vaxp_widget_invalidate_layout(widget);
```

## Event Types

```c
VAXP_EVENT_MOUSE_MOVE
VAXP_EVENT_MOUSE_BUTTON_DOWN
VAXP_EVENT_MOUSE_BUTTON_UP
VAXP_EVENT_MOUSE_SCROLL
VAXP_EVENT_MOUSE_ENTER
VAXP_EVENT_MOUSE_LEAVE
VAXP_EVENT_KEY_DOWN
VAXP_EVENT_KEY_UP
VAXP_EVENT_TEXT_INPUT
VAXP_EVENT_FOCUS
VAXP_EVENT_BLUR
```

## Keys

```c
VAXP_KEY_RETURN    VAXP_KEY_ESCAPE
VAXP_KEY_UP        VAXP_KEY_DOWN
VAXP_KEY_LEFT      VAXP_KEY_RIGHT
VAXP_KEY_BACKSPACE VAXP_KEY_DELETE
VAXP_KEY_TAB       VAXP_KEY_SPACE
VAXP_KEY_HOME      VAXP_KEY_END
```

## Mouse Buttons

```c
VAXP_MOUSE_BUTTON_LEFT
VAXP_MOUSE_BUTTON_RIGHT
VAXP_MOUSE_BUTTON_MIDDLE
```

## Widget States

```c
VAXP_WIDGET_STATE_NORMAL
VAXP_WIDGET_STATE_HOVER
VAXP_WIDGET_STATE_FOCUSED
VAXP_WIDGET_STATE_DISABLED
VAXP_WIDGET_STATE_PRESSED
```

## Callback Signatures

```c
/* Button */
void (*on_click)(VaxpWidget* btn, void* data);

/* Checkbox/Switch */
void (*on_change)(VaxpCheckbox* cb, VaxpBool checked, void* data);

/* Slider */
void (*on_change)(VaxpSlider* slider, VaxpF32 value, void* data);

/* TextInput */
void (*on_change)(VaxpTextInput* input, const char* text, void* data);

/* Dropdown */
void (*on_select)(VaxpDropdown* dd, VaxpU32 index, const char* value, void* data);

/* Tabs */
void (*on_change)(VaxpTabBar* bar, VaxpU32 index, void* data);

/* Rating */
void (*on_change)(VaxpRating* rating, VaxpF32 value, void* data);
```

## Type Aliases

```c
VaxpBool   = int           /* VAXP_TRUE / VAXP_FALSE */
VaxpU8     = uint8_t
VaxpU32    = uint32_t
VaxpI32    = int32_t
VaxpF32    = float
VaxpSize   = size_t
```
