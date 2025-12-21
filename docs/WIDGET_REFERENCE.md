# VENOMUI Widget Documentation

Complete API reference and usage guide for all 45 VENOMUI widgets.

---

## Table of Contents

1. [Getting Started](#getting-started)
2. [Layout Widgets](#layout-widgets)
3. [Input Controls](#input-controls)
4. [Display Widgets](#display-widgets)
5. [Navigation Widgets](#navigation-widgets)
6. [Feedback & Overlays](#feedback--overlays)
7. [Multi-Step Widgets](#multi-step-widgets)

---

## Getting Started

### Including VENOMUI

```c
#include <venomui.h>
```

### Widget Lifecycle

All widgets follow this lifecycle:
1. **Create** - Use `venom_*_create()` or macro builder
2. **Configure** - Set properties
3. **Add to parent** - `venom_container_add_child()`
4. **Use** - Widget handles events automatically
5. **Destroy** - Automatic via reference counting with `venom_unref()`

### Memory Management

```c
/* Widgets use reference counting */
VenomWidget* widget = venom_button_create().value;
venom_ref(widget);    /* Increment ref count */
venom_unref(widget);  /* Decrement (frees when 0) */
```

---

## Layout Widgets

### VenomContainer

Flexible box layout for arranging children in rows, columns, or grids.

```c
/* Create a horizontal row */
VenomWidget* row = venom_container(
    .direction = VENOM_DIRECTION_HORIZONTAL,
    .spacing = 10.0f,
    .padding = (VenomInsets){ 16, 16, 16, 16 },
    .alignment = VENOM_ALIGN_CENTER
);

/* Add children */
venom_container_add_child((VenomContainer*)row, button1);
venom_container_add_child((VenomContainer*)row, button2);

/* Create a vertical column */
VenomWidget* column = venom_container(
    .direction = VENOM_DIRECTION_VERTICAL,
    .spacing = 8.0f
);
```

**Properties:**
| Property | Type | Description |
|----------|------|-------------|
| `direction` | `VenomDirection` | HORIZONTAL, VERTICAL, GRID |
| `spacing` | `VenomF32` | Space between children |
| `padding` | `VenomInsets` | Inner padding |
| `alignment` | `VenomAlignment` | START, CENTER, END |

---

### VenomStack

Overlays children on top of each other.

```c
VenomWidget* stack = venom_stack_create().value;

/* Children are stacked - last added is on top */
venom_stack_add_child((VenomStack*)stack, background_image);
venom_stack_add_child((VenomStack*)stack, overlay_text);
```

---

### VenomSpacer

Flexible empty space that expands to fill available area.

```c
/* Push button to the right */
venom_container_add_child(row, label);
venom_container_add_child(row, VENOM_SPACER(.flex = 1.0f));
venom_container_add_child(row, button);
```

---

### VenomDivider

Horizontal or vertical line separator.

```c
VenomWidget* divider = venom_divider(
    .vertical = VENOM_FALSE,  /* Horizontal line */
    .thickness = 1.0f,
    .color = (VenomColor){ 200, 200, 200, 255 }
);
```

---

### VenomSizedBox

Fixed-size container.

```c
VenomWidget* box = VENOM_SIZED_BOX(
    .width = 100.0f,
    .height = 50.0f,
    .child = some_widget
);
```

---

### VenomPadding

Adds padding around a child widget.

```c
VenomWidget* padded = venom_padding(
    .padding = (VenomInsets){ 20, 20, 20, 20 },
    .child = content_widget
);
```

---

### VenomSplitPane

Resizable split panels with draggable divider.

```c
VenomWidget* split = venom_split_pane(
    .first = left_panel,
    .second = right_panel,
    .direction = VENOM_SPLIT_HORIZONTAL,
    .position = 0.3f  /* 30% for first panel */
);

/* API */
venom_split_pane_set_position(split, 0.5f);
venom_split_pane_set_min_sizes(split, 100, 100);
```

---

### VenomScrollable

Scrollable container for content larger than viewport.

```c
VenomWidget* scroll = venom_scrollable_create().value;
venom_scrollable_set_content((VenomScrollable*)scroll, long_content);
venom_scrollable_set_direction((VenomScrollable*)scroll, VENOM_SCROLL_VERTICAL);
```

---

## Input Controls

### VenomButton

Clickable button with various styles.

```c
void on_click(VenomWidget* btn, void* data) {
    printf("Button clicked!\n");
}

VenomWidget* button = venom_button(
    .label = "Click Me",
    .on_click = on_click,
    .data = NULL,
    .style = VENOM_BUTTON_FILLED
);

/* Styles: FILLED, OUTLINED, TEXT */
```

**API:**
```c
venom_button_set_label(btn, "New Label");
venom_button_set_enabled(btn, VENOM_FALSE);
venom_button_set_loading(btn, VENOM_TRUE);
```

---

### VenomCheckbox

Boolean toggle with label.

```c
void on_toggle(VenomCheckbox* cb, VenomBool checked, void* data) {
    printf("Checked: %s\n", checked ? "yes" : "no");
}

VenomWidget* checkbox = venom_checkbox(
    .label = "Accept Terms",
    .checked = VENOM_FALSE,
    .on_change = on_toggle
);

/* API */
VenomBool is_checked = venom_checkbox_get_checked(checkbox);
venom_checkbox_set_checked(checkbox, VENOM_TRUE);
```

---

### VenomSwitch

On/off toggle switch.

```c
void on_switch(VenomSwitch* sw, VenomBool on, void* data) {
    printf("Switch: %s\n", on ? "ON" : "OFF");
}

VenomWidget* sw = venom_switch(
    .on = VENOM_FALSE,
    .on_change = on_switch
);
```

---

### VenomSlider

Value selection slider.

```c
void on_slide(VenomSlider* slider, VenomF32 value, void* data) {
    printf("Value: %.2f\n", value);
}

VenomWidget* slider = venom_slider(
    .value = 50.0f,
    .min = 0.0f,
    .max = 100.0f,
    .step = 1.0f,
    .on_change = on_slide
);

/* API */
VenomF32 val = venom_slider_get_value(slider);
venom_slider_set_value(slider, 75.0f);
```

---

### VenomRadioButton & VenomRadioGroup

Mutually exclusive selection.

```c
void on_select(VenomRadioGroup* group, VenomU32 index, void* data) {
    printf("Selected option: %u\n", index);
}

/* Create group */
VenomRadioGroup* group = (VenomRadioGroup*)venom_radio_group_create().value;
venom_radio_group_set_on_change(group, on_select, NULL);

/* Add options */
venom_radio_group_add_option(group, "Option A");
venom_radio_group_add_option(group, "Option B");
venom_radio_group_add_option(group, "Option C");

/* Set selected */
venom_radio_group_set_selected(group, 0);
```

---

### VenomTextInput

Single-line text input field.

```c
void on_text_change(VenomTextInput* input, const char* text, void* data) {
    printf("Text: %s\n", text);
}

VenomWidget* input = venom_text_input(
    .placeholder = "Enter name...",
    .on_change = on_text_change
);

/* API */
const char* text = venom_text_input_get_text(input);
venom_text_input_set_text(input, "Hello");
venom_text_input_clear(input);
```

---

### VenomTextArea

Multi-line text input.

```c
VenomWidget* textarea = venom_text_area(
    .placeholder = "Enter description...",
    .max_length = 1000,
    .on_change = on_change_callback
);

/* API */
venom_text_area_set_text(textarea, "Multi\nline\ntext");
const char* content = venom_text_area_get_text(textarea);
```

---

### VenomDropdown

Selection dropdown list.

```c
void on_select(VenomDropdown* dd, VenomU32 index, const char* value, void* data) {
    printf("Selected: %s (index %u)\n", value, index);
}

VenomWidget* dropdown = venom_dropdown(
    .placeholder = "Select country...",
    .on_select = on_select
);

/* Add items */
venom_dropdown_add_item(dropdown, "USA");
venom_dropdown_add_item(dropdown, "UK");
venom_dropdown_add_item(dropdown, "Germany");

/* API */
venom_dropdown_set_selected(dropdown, 0);
VenomU32 idx = venom_dropdown_get_selected(dropdown);
```

---

### VenomSearchBar

Search input with icon and clear button.

```c
void on_search(VenomSearchBar* bar, const char* query, void* data) {
    printf("Searching for: %s\n", query);
}

VenomWidget* search = venom_search_bar(
    .placeholder = "Search...",
    .on_search = on_search
);

/* API */
venom_search_bar_clear(search);
const char* query = venom_search_bar_get_text(search);
```

---

### VenomNumberInput

Numeric input with +/- buttons.

```c
void on_number(VenomNumberInput* input, VenomF32 value, void* data) {
    printf("Value: %.0f\n", value);
}

VenomWidget* num = venom_number_input(
    .value = 5,
    .min = 0,
    .max = 100,
    .step = 1,
    .on_change = on_number
);

/* API */
venom_number_input_set_value(num, 10);
VenomF32 val = venom_number_input_get_value(num);
```

---

### VenomToggleButton

Button that maintains on/off state.

```c
void on_toggle(VenomToggleButton* btn, VenomBool toggled, void* data) {
    printf("Toggled: %s\n", toggled ? "ON" : "OFF");
}

VenomWidget* toggle = venom_toggle_button(
    .label = "Bold",
    .toggled = VENOM_FALSE,
    .on_toggle = on_toggle
);
```

---

### VenomRating

Star rating widget.

```c
void on_rate(VenomRating* rating, VenomF32 value, void* data) {
    printf("Rating: %.1f stars\n", value);
}

VenomWidget* rating = venom_rating(
    .value = 3.5f,
    .max = 5,
    .read_only = VENOM_FALSE,
    .on_change = on_rate
);

/* API */
venom_rating_set_value(rating, 4.0f);
VenomF32 stars = venom_rating_get_value(rating);
```

---

## Display Widgets

### VenomLabel

Text display widget.

```c
VenomWidget* label = venom_label(
    .text = "Hello, World!",
    .font_size = 16.0f,
    .color = (VenomColor){ 0, 0, 0, 255 }
);

/* API */
venom_label_set_text(label, "New text");
```

---

### VenomImage

Image display widget.

```c
VenomWidget* image = venom_image_create().value;
venom_image_load((VenomImage*)image, "/path/to/image.png");
venom_image_set_fit((VenomImage*)image, VENOM_IMAGE_FIT_CONTAIN);
```

---

### VenomIcon

Emoji or icon font display.

```c
VenomWidget* icon = venom_icon(
    .icon = "⚙️",  /* or "settings" for icon fonts */
    .size = 24.0f,
    .color = (VenomColor){ 97, 97, 97, 255 }
);
```

---

### VenomAvatar

Circular profile display with initials or image.

```c
VenomWidget* avatar = venom_avatar(
    .initials = "JD",  /* Shows "JD" if no image */
    .image = "/path/to/photo.jpg",
    .size = 40.0f,
    .color = (VenomColor){ 63, 81, 181, 255 }
);

/* Show online indicator */
venom_avatar_set_indicator(avatar, VENOM_TRUE, 
    (VenomColor){ 76, 175, 80, 255 });
```

---

### VenomBadge

Notification count indicator.

```c
VenomWidget* badge = venom_badge(
    .count = 5,
    .child = icon_widget,
    .max_count = 99,  /* Shows "99+" if exceeded */
    .dot = VENOM_FALSE  /* Just dot, no number */
);

/* API */
venom_badge_set_count(badge, 10);
```

---

### VenomChip

Tag or filter element.

```c
void on_chip_click(VenomChip* chip, void* data) {
    printf("Chip clicked\n");
}

VenomWidget* chip = venom_chip(
    .label = "Technology",
    .type = VENOM_CHIP_FILTER,  /* FILTER, CHOICE, INPUT, ACTION */
    .selected = VENOM_FALSE,
    .on_click = on_chip_click
);
```

---

### VenomCard

Elevated container with shadow.

```c
VenomWidget* card = venom_card(
    .child = content_widget,
    .elevation = 4.0f,
    .corner_radius = 12.0f,
    .padding = 16.0f,
    .outlined = VENOM_FALSE
);
```

---

### VenomColorSwatch

Color display/picker swatch.

```c
void on_color_click(VenomColorSwatch* swatch, VenomColor color, void* data) {
    printf("Color clicked: #%02X%02X%02X\n", color.r, color.g, color.b);
}

VenomWidget* swatch = venom_color_swatch(
    .color = (VenomColor){ 255, 0, 0, 255 },
    .size = 32.0f,
    .selectable = VENOM_TRUE,
    .on_click = on_color_click
);
```

---

### VenomLink

Clickable hyperlink.

```c
void on_link_click(VenomLink* link, const char* url, void* data) {
    printf("Opening: %s\n", url);
    /* Open URL in browser */
}

VenomWidget* link = venom_link(
    .text = "Visit Website",
    .url = "https://example.com",
    .on_click = on_link_click
);
```

---

### VenomSkeleton

Loading placeholder with shimmer animation.

```c
VenomWidget* skeleton = venom_skeleton(
    .variant = VENOM_SKELETON_RECTANGULAR,  /* TEXT, CIRCULAR */
    .width = 200.0f,
    .height = 20.0f
);

/* Call periodically for animation */
venom_skeleton_animate(skeleton, delta_time);
```

---

## Navigation Widgets

### VenomTabBar & VenomTabView

Tab-based navigation.

```c
void on_tab_change(VenomTabBar* bar, VenomU32 index, void* data) {
    printf("Tab changed to: %u\n", index);
}

/* Create tab bar */
VenomTabBar* tabs = (VenomTabBar*)venom_tab_bar_create().value;
venom_tab_bar_set_style(tabs, VENOM_TAB_STYLE_UNDERLINE);
venom_tab_bar_set_on_change(tabs, on_tab_change, NULL);

/* Create tab view */
VenomTabView* view = (VenomTabView*)venom_tab_view_create().value;

/* Link them */
venom_tab_bar_link_view(tabs, view);

/* Add tabs */
venom_tab_bar_add_tab(tabs, "Home", home_content);
venom_tab_bar_add_tab(tabs, "Settings", settings_content);
venom_tab_bar_add_tab(tabs, "About", about_content);

/* Styles: UNDERLINE, PILL, BOXED */
```

---

### VenomBreadcrumb

Path navigation.

```c
void on_navigate(VenomBreadcrumb* bc, VenomU32 index, const char* path, void* data) {
    printf("Navigate to: %s\n", path);
}

VenomBreadcrumb* bc = (VenomBreadcrumb*)venom_breadcrumb_create().value;
venom_breadcrumb_set_on_navigate(bc, on_navigate, NULL);

/* Add items manually */
venom_breadcrumb_add_item(bc, "Home", "/");
venom_breadcrumb_add_item(bc, "Documents", "/documents");
venom_breadcrumb_add_item(bc, "Report.pdf", "/documents/report.pdf");

/* Or parse from path */
venom_breadcrumb_set_path(bc, "/home/user/documents", '/');
```

---

### VenomTreeView

Hierarchical tree display.

```c
void on_tree_select(VenomTreeView* tree, VenomTreeNode* node, void* data) {
    printf("Selected: %s\n", node->label);
}

/* Create tree */
VenomTreeView* tree = (VenomTreeView*)venom_tree_view_create().value;
venom_tree_view_set_on_select(tree, on_tree_select, NULL);

/* Create nodes */
VenomTreeNode* root = venom_tree_node_create("Root", NULL);
VenomTreeNode* child1 = venom_tree_node_create("Child 1", NULL);
VenomTreeNode* child2 = venom_tree_node_create("Child 2", NULL);
VenomTreeNode* grandchild = venom_tree_node_create("Grandchild", NULL);

/* Build hierarchy */
venom_tree_node_add_child(root, child1);
venom_tree_node_add_child(root, child2);
venom_tree_node_add_child(child1, grandchild);

/* Set root */
venom_tree_view_add_root(tree, root);

/* API */
venom_tree_view_expand_all(tree);
venom_tree_view_collapse_all(tree);
```

---

### VenomListView

Scrollable list with virtualization.

```c
VenomWidget* build_item(VenomU32 index, void* data, void* user_data) {
    const char* text = (const char*)data;
    return venom_label(.text = text);
}

VenomListView* list = (VenomListView*)venom_list_view_create().value;
venom_list_view_set_builder(list, build_item, NULL);

/* Add items */
venom_list_view_add_item(list, "Item 1");
venom_list_view_add_item(list, "Item 2");
venom_list_view_add_item(list, "Item 3");

/* API */
venom_list_view_set_selected(list, 0);
VenomI32 selected = venom_list_view_get_selected(list);
```

---

### VenomGridView

Grid layout list.

```c
VenomWidget* build_grid_item(VenomU32 index, void* data, void* user_data) {
    return venom_card(.child = venom_label(.text = data));
}

VenomGridView* grid = (VenomGridView*)venom_grid_view_create().value;
venom_grid_view_set_columns(grid, 3);
venom_grid_view_set_spacing(grid, 16.0f, 16.0f);
venom_grid_view_set_builder(grid, build_grid_item, NULL);
venom_grid_view_set_item_size(grid, 0, 150.0f);  /* 0 = auto width */

/* Add items */
for (int i = 0; i < 20; i++) {
    venom_grid_view_add_item(grid, "Item");
}
```

---

## Feedback & Overlays

### VenomTooltip

Hover information popup.

```c
VenomWidget* tooltip = venom_tooltip(
    .text = "This is helpful information",
    .child = button_widget,
    .delay_ms = 500,
    .position = VENOM_TOOLTIP_TOP
);
```

---

### VenomDialog

Modal dialog.

```c
void on_ok(VenomDialog* dialog, void* data) {
    printf("OK clicked\n");
    venom_dialog_close(dialog);
}

void on_cancel(VenomDialog* dialog, void* data) {
    venom_dialog_close(dialog);
}

VenomDialog* dialog = (VenomDialog*)venom_dialog_create().value;
venom_dialog_set_title(dialog, "Confirm Action");
venom_dialog_set_content(dialog, content_widget);
venom_dialog_add_action(dialog, "Cancel", on_cancel, NULL);
venom_dialog_add_action(dialog, "OK", on_ok, NULL);

/* Show dialog */
venom_dialog_show(dialog);
```

---

### VenomContextMenu

Right-click context menu.

```c
void on_menu_item(VenomContextMenu* menu, VenomU32 index, void* data) {
    printf("Menu item %u clicked\n", index);
}

VenomContextMenu* menu = (VenomContextMenu*)venom_context_menu_create().value;
venom_context_menu_add_item(menu, "Cut", on_menu_item, NULL);
venom_context_menu_add_item(menu, "Copy", on_menu_item, NULL);
venom_context_menu_add_item(menu, "Paste", on_menu_item, NULL);
venom_context_menu_add_separator(menu);
venom_context_menu_add_item(menu, "Delete", on_menu_item, NULL);

/* Show at position */
venom_context_menu_show(menu, x, y);
```

---

### VenomNotification

Toast notification.

```c
void on_dismiss(VenomNotification* notif, void* data) {
    printf("Notification dismissed\n");
}

VenomWidget* notif = venom_notification(
    .title = "Success",
    .message = "File saved successfully",
    .type = VENOM_NOTIFY_SUCCESS,  /* INFO, SUCCESS, WARNING, ERROR */
    .duration_ms = 4000,
    .dismissible = VENOM_TRUE,
    .position = VENOM_NOTIFY_TOP_RIGHT
);

venom_notification_set_on_dismiss(notif, on_dismiss, NULL);
venom_notification_show(notif);
```

---

### VenomSnackbar

Bottom message bar with action.

```c
void on_undo(VenomSnackbar* bar, void* data) {
    printf("Undo clicked\n");
}

VenomWidget* snackbar = venom_snackbar(
    .message = "Item deleted",
    .action = "UNDO",
    .on_action = on_undo,
    .duration = 5000
);

venom_snackbar_show(snackbar);
```

---

### VenomSpinner

Loading indicator.

```c
VenomWidget* spinner = venom_spinner(
    .size = 40.0f,
    .color = (VenomColor){ 63, 81, 181, 255 }
);

/* Call in animation loop */
venom_spinner_animate(spinner, delta_time);
```

---

## Multi-Step Widgets

### VenomProgressBar

Linear progress indicator.

```c
VenomWidget* progress = venom_progress_bar(
    .value = 0.75f,  /* 0.0 - 1.0 */
    .indeterminate = VENOM_FALSE,
    .show_label = VENOM_TRUE
);

/* API */
venom_progress_bar_set_value(progress, 0.5f);
```

---

### VenomAccordion

Collapsible sections.

```c
VenomAccordion* accordion = (VenomAccordion*)venom_accordion_create().value;

venom_accordion_add_section(accordion, "Section 1", content1);
venom_accordion_add_section(accordion, "Section 2", content2);
venom_accordion_add_section(accordion, "Section 3", content3);

/* API */
venom_accordion_expand(accordion, 0);
venom_accordion_collapse(accordion, 0);
venom_accordion_toggle(accordion, 1);
venom_accordion_expand_all(accordion);
venom_accordion_collapse_all(accordion);
```

---

### VenomStepper

Step indicator for wizards.

```c
VenomStepper* stepper = (VenomStepper*)venom_stepper_create().value;

venom_stepper_add_step(stepper, "Account");
venom_stepper_add_step(stepper, "Details");
venom_stepper_add_step(stepper, "Review");
venom_stepper_add_step(stepper, "Complete");

/* API */
venom_stepper_set_current(stepper, 1);
venom_stepper_next(stepper);         /* Marks current as complete, moves to next */
venom_stepper_prev(stepper);
venom_stepper_complete_step(stepper, 0);
venom_stepper_set_error(stepper, 2, VENOM_TRUE);
```

---

## Complete Example

```c
#include <venomui.h>

void on_button_click(VenomWidget* btn, void* data) {
    printf("Button clicked!\n");
}

int main(void) {
    /* Initialize VENOMUI */
    venom_init();
    
    /* Create main window */
    VenomWindow* window = venom_window_create("My App", 800, 600);
    
    /* Create layout */
    VenomWidget* root = venom_container(
        .direction = VENOM_DIRECTION_VERTICAL,
        .padding = (VenomInsets){ 20, 20, 20, 20 },
        .spacing = 16
    );
    
    /* Add header */
    VenomWidget* header = venom_label(
        .text = "Welcome to VENOMUI",
        .font_size = 24
    );
    venom_container_add_child((VenomContainer*)root, header);
    
    /* Add input */
    VenomWidget* input = venom_text_input(
        .placeholder = "Enter your name..."
    );
    venom_container_add_child((VenomContainer*)root, input);
    
    /* Add button */
    VenomWidget* button = venom_button(
        .label = "Submit",
        .on_click = on_button_click
    );
    venom_container_add_child((VenomContainer*)root, button);
    
    /* Set window content */
    venom_window_set_content(window, root);
    
    /* Run application */
    venom_app_run();
    
    /* Cleanup */
    venom_shutdown();
    return 0;
}
```

---

## Building

```bash
# Configure build
meson setup build

# Compile
meson compile -C build

# Run example
./build/examples/hello_world
```

---

*Documentation generated for VENOMUI v0.1.0*
