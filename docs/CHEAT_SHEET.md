# VENOMUI API Cheat Sheet

## Widget Creation

| Widget | Macro | Create Function |
|--------|-------|-----------------|
| Button | `venom_button(...)` | `venom_button_create()` |
| Label | `venom_label(...)` | `venom_label_create()` |
| Container | `venom_container(...)` | `venom_container_create()` |
| TextInput | `venom_text_input(...)` | `venom_text_input_create()` |
| Checkbox | `venom_checkbox(...)` | `venom_checkbox_create()` |
| Switch | `venom_switch(...)` | `venom_switch_create()` |
| Slider | `venom_slider(...)` | `venom_slider_create()` |
| Dropdown | `venom_dropdown(...)` | `venom_dropdown_create()` |
| Card | `venom_card(...)` | `venom_card_create()` |
| Avatar | `venom_avatar(...)` | `venom_avatar_create()` |
| Icon | `venom_icon(...)` | `venom_icon_create()` |
| Badge | `venom_badge(...)` | `venom_badge_create()` |
| Chip | `venom_chip(...)` | `venom_chip_create()` |
| Tooltip | `venom_tooltip(...)` | `venom_tooltip_create()` |
| Notification | `venom_notification(...)` | `venom_notification_create()` |
| Snackbar | `venom_snackbar(...)` | `venom_snackbar_create()` |
| Spinner | `venom_spinner(...)` | `venom_spinner_create()` |
| ProgressBar | `venom_progress_bar(...)` | `venom_progress_bar_create()` |
| Rating | `venom_rating(...)` | `venom_rating_create()` |
| Link | `venom_link(...)` | `venom_link_create()` |
| Skeleton | `venom_skeleton(...)` | `venom_skeleton_create()` |
| SearchBar | `venom_search_bar(...)` | `venom_search_bar_create()` |
| NumberInput | `venom_number_input(...)` | `venom_number_input_create()` |
| ToggleButton | `venom_toggle_button(...)` | `venom_toggle_button_create()` |
| ColorSwatch | `venom_color_swatch(...)` | `venom_color_swatch_create()` |
| SplitPane | `venom_split_pane(...)` | `venom_split_pane_create()` |
| TextArea | `venom_text_area(...)` | `venom_text_area_create()` |
| Popover | `venom_popover(...)` | `venom_popover_create()` |
| Calendar | `venom_calendar(...)` | `venom_calendar_create()` |
| DatePicker | `venom_date_picker(...)` | `venom_date_picker_create()` |
| TimePicker | `venom_time_picker(...)` | `venom_time_picker_create()` |
| ColorPicker | `venom_color_picker(...)` | `venom_color_picker_create()` |
| FileChooser | `venom_file_chooser(...)` | `venom_file_chooser_create()` |

## Layout

```c
/* Horizontal Row */
venom_container(.direction = VENOM_DIRECTION_HORIZONTAL)

/* Vertical Column */
venom_container(.direction = VENOM_DIRECTION_VERTICAL)

/* Grid */
venom_container(.direction = VENOM_DIRECTION_GRID, .columns = 3)

/* Spacing */
venom_container(.spacing = 16.0f)

/* Padding */
venom_container(.padding = (VenomInsets){ 20, 20, 20, 20 })

/* Alignment */
venom_container(.alignment = VENOM_ALIGN_CENTER)  /* START, CENTER, END */
```

## Colors

```c
/* Basic Colors */
VENOM_COLOR_RED          VENOM_COLOR_GREEN
VENOM_COLOR_BLUE         VENOM_COLOR_WHITE
VENOM_COLOR_BLACK        VENOM_COLOR_TRANSPARENT

/* Custom */
(VenomColor){ r, g, b, a }  /* 0-255 each */
```

## Common APIs

```c
/* Reference Counting */
venom_ref(widget);
venom_unref(widget);

/* Visibility */
widget->visible = VENOM_TRUE;
widget->enabled = VENOM_FALSE;

/* Layout */
widget->layout.flex = 1.0f;
widget->layout.preferred_width = 200.0f;
widget->layout.preferred_height = 100.0f;

/* Invalidate */
venom_widget_invalidate(widget);
venom_widget_invalidate_layout(widget);
```

## Event Types

```c
VENOM_EVENT_MOUSE_MOVE
VENOM_EVENT_MOUSE_BUTTON_DOWN
VENOM_EVENT_MOUSE_BUTTON_UP
VENOM_EVENT_MOUSE_SCROLL
VENOM_EVENT_MOUSE_ENTER
VENOM_EVENT_MOUSE_LEAVE
VENOM_EVENT_KEY_DOWN
VENOM_EVENT_KEY_UP
VENOM_EVENT_TEXT_INPUT
VENOM_EVENT_FOCUS
VENOM_EVENT_BLUR
```

## Keys

```c
VENOM_KEY_RETURN    VENOM_KEY_ESCAPE
VENOM_KEY_UP        VENOM_KEY_DOWN
VENOM_KEY_LEFT      VENOM_KEY_RIGHT
VENOM_KEY_BACKSPACE VENOM_KEY_DELETE
VENOM_KEY_TAB       VENOM_KEY_SPACE
VENOM_KEY_HOME      VENOM_KEY_END
```

## Mouse Buttons

```c
VENOM_MOUSE_BUTTON_LEFT
VENOM_MOUSE_BUTTON_RIGHT
VENOM_MOUSE_BUTTON_MIDDLE
```

## Widget States

```c
VENOM_WIDGET_STATE_NORMAL
VENOM_WIDGET_STATE_HOVER
VENOM_WIDGET_STATE_FOCUSED
VENOM_WIDGET_STATE_DISABLED
VENOM_WIDGET_STATE_PRESSED
```

## Callback Signatures

```c
/* Button */
void (*on_click)(VenomWidget* btn, void* data);

/* Checkbox/Switch */
void (*on_change)(VenomCheckbox* cb, VenomBool checked, void* data);

/* Slider */
void (*on_change)(VenomSlider* slider, VenomF32 value, void* data);

/* TextInput */
void (*on_change)(VenomTextInput* input, const char* text, void* data);

/* Dropdown */
void (*on_select)(VenomDropdown* dd, VenomU32 index, const char* value, void* data);

/* Tabs */
void (*on_change)(VenomTabBar* bar, VenomU32 index, void* data);

/* Rating */
void (*on_change)(VenomRating* rating, VenomF32 value, void* data);
```

## Type Aliases

```c
VenomBool   = int           /* VENOM_TRUE / VENOM_FALSE */
VenomU8     = uint8_t
VenomU32    = uint32_t
VenomI32    = int32_t
VenomF32    = float
VenomSize   = size_t
```
