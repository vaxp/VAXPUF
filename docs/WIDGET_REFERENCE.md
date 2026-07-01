# VAXPUI Widget Documentation

Complete API reference and usage guide for all 61 VAXPUI widgets.

---

## Table of Contents

1. [Getting Started](#getting-started)
2. [Layout Widgets](#layout-widgets)
3. [Input Controls](#input-controls)
4. [Display Widgets](#display-widgets)
5. [Navigation Widgets](#navigation-widgets)
6. [Feedback & Overlays](#feedback--overlays)
7. [Multi-Step Widgets](#multi-step-widgets)
8. [Desktop UI Widgets](#desktop-ui-widgets)


---

## Getting Started

### Including VAXPUI

```c
#include <vaxpui.h>
```

### Widget Lifecycle

All widgets follow this lifecycle:
1. **Create** - Use `vaxp_*_create()` or macro builder
2. **Configure** - Set properties
3. **Add to parent** - `vaxp_container_add_child()`
4. **Use** - Widget handles events automatically
5. **Destroy** - Automatic via reference counting with `vaxp_unref()`

### Memory Management

```c
/* Widgets use reference counting */
VaxpWidget* widget = vaxp_button_create().value;
vaxp_ref(widget);    /* Increment ref count */
vaxp_unref(widget);  /* Decrement (frees when 0) */
```

---

## Layout Widgets

### VaxpContainer

Flexible box layout for arranging children in rows, columns, or grids.

```c
/* Create a horizontal row */
VaxpWidget* row = vaxp_container(
    .direction = VAXP_DIRECTION_HORIZONTAL,
    .spacing = 10.0f,
    .padding = (VaxpInsets){ 16, 16, 16, 16 },
    .alignment = VAXP_ALIGN_CENTER
);

/* Add children */
vaxp_container_add_child((VaxpContainer*)row, button1);
vaxp_container_add_child((VaxpContainer*)row, button2);

/* Create a vertical column */
VaxpWidget* column = vaxp_container(
    .direction = VAXP_DIRECTION_VERTICAL,
    .spacing = 8.0f
);
```

**Properties:**
| Property | Type | Description |
|----------|------|-------------|
| `direction` | `VaxpDirection` | HORIZONTAL, VERTICAL, GRID |
| `spacing` | `VaxpF32` | Space between children |
| `padding` | `VaxpInsets` | Inner padding |
| `alignment` | `VaxpAlignment` | START, CENTER, END |

---

### VaxpStack

Overlays children on top of each other.

```c
VaxpWidget* stack = vaxp_stack_create().value;

/* Children are stacked - last added is on top */
vaxp_stack_add_child((VaxpStack*)stack, background_image);
vaxp_stack_add_child((VaxpStack*)stack, overlay_text);
```

---

### VaxpSpacer

Flexible empty space that expands to fill available area.

```c
/* Push button to the right */
vaxp_container_add_child(row, label);
vaxp_container_add_child(row, VAXP_SPACER(.flex = 1.0f));
vaxp_container_add_child(row, button);
```

---

### VaxpDivider

Horizontal or vertical line separator.

```c
VaxpWidget* divider = vaxp_divider(
    .vertical = VAXP_FALSE,  /* Horizontal line */
    .thickness = 1.0f,
    .color = (VaxpColor){ 200, 200, 200, 255 }
);
```

---

### VaxpSizedBox

Fixed-size container.

```c
VaxpWidget* box = VAXP_SIZED_BOX(
    .width = 100.0f,
    .height = 50.0f,
    .child = some_widget
);
```

---

### VaxpPadding

Adds padding around a child widget.

```c
VaxpWidget* padded = vaxp_padding(
    .padding = (VaxpInsets){ 20, 20, 20, 20 },
    .child = content_widget
);
```

---

### VaxpSplitPane

Resizable split panels with draggable divider.

```c
VaxpWidget* split = vaxp_split_pane(
    .first = left_panel,
    .second = right_panel,
    .direction = VAXP_SPLIT_HORIZONTAL,
    .position = 0.3f  /* 30% for first panel */
);

/* API */
vaxp_split_pane_set_position(split, 0.5f);
vaxp_split_pane_set_min_sizes(split, 100, 100);
```

---

### VaxpScrollable

Scrollable container for content larger than viewport.

```c
VaxpWidget* scroll = vaxp_scrollable_create().value;
vaxp_scrollable_set_content((VaxpScrollable*)scroll, long_content);
vaxp_scrollable_set_direction((VaxpScrollable*)scroll, VAXP_SCROLL_VERTICAL);
```

---

## Input Controls

### VaxpButton

Clickable button with various styles.

```c
void on_click(VaxpWidget* btn, void* data) {
    printf("Button clicked!\n");
}

VaxpWidget* button = vaxp_button(
    .label = "Click Me",
    .on_click = on_click,
    .data = NULL,
    .style = VAXP_BUTTON_FILLED
);

/* Styles: FILLED, OUTLINED, TEXT */
```

**API:**
```c
vaxp_button_set_label(btn, "New Label");
vaxp_button_set_enabled(btn, VAXP_FALSE);
vaxp_button_set_loading(btn, VAXP_TRUE);
```

---

### VaxpCheckbox

Boolean toggle with label.

```c
void on_toggle(VaxpCheckbox* cb, VaxpBool checked, void* data) {
    printf("Checked: %s\n", checked ? "yes" : "no");
}

VaxpWidget* checkbox = vaxp_checkbox(
    .label = "Accept Terms",
    .checked = VAXP_FALSE,
    .on_change = on_toggle
);

/* API */
VaxpBool is_checked = vaxp_checkbox_get_checked(checkbox);
vaxp_checkbox_set_checked(checkbox, VAXP_TRUE);
```

---

### VaxpSwitch

On/off toggle switch.

```c
void on_switch(VaxpSwitch* sw, VaxpBool on, void* data) {
    printf("Switch: %s\n", on ? "ON" : "OFF");
}

VaxpWidget* sw = vaxp_switch(
    .on = VAXP_FALSE,
    .on_change = on_switch
);
```

---

### VaxpSlider

Value selection slider.

```c
void on_slide(VaxpSlider* slider, VaxpF32 value, void* data) {
    printf("Value: %.2f\n", value);
}

VaxpWidget* slider = vaxp_slider(
    .value = 50.0f,
    .min = 0.0f,
    .max = 100.0f,
    .step = 1.0f,
    .on_change = on_slide
);

/* API */
VaxpF32 val = vaxp_slider_get_value(slider);
vaxp_slider_set_value(slider, 75.0f);
```

---

### VaxpRadioButton & VaxpRadioGroup

Mutually exclusive selection.

```c
void on_select(VaxpRadioGroup* group, VaxpU32 index, void* data) {
    printf("Selected option: %u\n", index);
}

/* Create group */
VaxpRadioGroup* group = (VaxpRadioGroup*)vaxp_radio_group_create().value;
vaxp_radio_group_set_on_change(group, on_select, NULL);

/* Add options */
vaxp_radio_group_add_option(group, "Option A");
vaxp_radio_group_add_option(group, "Option B");
vaxp_radio_group_add_option(group, "Option C");

/* Set selected */
vaxp_radio_group_set_selected(group, 0);
```

---

### VaxpTextInput

Single-line text input field.

```c
void on_text_change(VaxpTextInput* input, const char* text, void* data) {
    printf("Text: %s\n", text);
}

VaxpWidget* input = vaxp_text_input(
    .placeholder = "Enter name...",
    .on_change = on_text_change
);

/* API */
const char* text = vaxp_text_input_get_text(input);
vaxp_text_input_set_text(input, "Hello");
vaxp_text_input_clear(input);
```

---

### VaxpTextArea

Multi-line text input.

```c
VaxpWidget* textarea = vaxp_text_area(
    .placeholder = "Enter description...",
    .max_length = 1000,
    .on_change = on_change_callback
);

/* API */
vaxp_text_area_set_text(textarea, "Multi\nline\ntext");
const char* content = vaxp_text_area_get_text(textarea);
```

---

### VaxpDropdown

Selection dropdown list.

```c
void on_select(VaxpDropdown* dd, VaxpU32 index, const char* value, void* data) {
    printf("Selected: %s (index %u)\n", value, index);
}

VaxpWidget* dropdown = vaxp_dropdown(
    .placeholder = "Select country...",
    .on_select = on_select
);

/* Add items */
vaxp_dropdown_add_item(dropdown, "USA");
vaxp_dropdown_add_item(dropdown, "UK");
vaxp_dropdown_add_item(dropdown, "Germany");

/* API */
vaxp_dropdown_set_selected(dropdown, 0);
VaxpU32 idx = vaxp_dropdown_get_selected(dropdown);
```

---

### VaxpSearchBar

Search input with icon and clear button.

```c
void on_search(VaxpSearchBar* bar, const char* query, void* data) {
    printf("Searching for: %s\n", query);
}

VaxpWidget* search = vaxp_search_bar(
    .placeholder = "Search...",
    .on_search = on_search
);

/* API */
vaxp_search_bar_clear(search);
const char* query = vaxp_search_bar_get_text(search);
```

---

### VaxpNumberInput

Numeric input with +/- buttons.

```c
void on_number(VaxpNumberInput* input, VaxpF32 value, void* data) {
    printf("Value: %.0f\n", value);
}

VaxpWidget* num = vaxp_number_input(
    .value = 5,
    .min = 0,
    .max = 100,
    .step = 1,
    .on_change = on_number
);

/* API */
vaxp_number_input_set_value(num, 10);
VaxpF32 val = vaxp_number_input_get_value(num);
```

---

### VaxpToggleButton

Button that maintains on/off state.

```c
void on_toggle(VaxpToggleButton* btn, VaxpBool toggled, void* data) {
    printf("Toggled: %s\n", toggled ? "ON" : "OFF");
}

VaxpWidget* toggle = vaxp_toggle_button(
    .label = "Bold",
    .toggled = VAXP_FALSE,
    .on_toggle = on_toggle
);
```

---

### VaxpRating

Star rating widget.

```c
void on_rate(VaxpRating* rating, VaxpF32 value, void* data) {
    printf("Rating: %.1f stars\n", value);
}

VaxpWidget* rating = vaxp_rating(
    .value = 3.5f,
    .max = 5,
    .read_only = VAXP_FALSE,
    .on_change = on_rate
);

/* API */
vaxp_rating_set_value(rating, 4.0f);
VaxpF32 stars = vaxp_rating_get_value(rating);
```

---

## Display Widgets

### VaxpLabel

Text display widget.

```c
VaxpWidget* label = vaxp_label(
    .text = "Hello, World!",
    .font_size = 16.0f,
    .color = (VaxpColor){ 0, 0, 0, 255 }
);

/* API */
vaxp_label_set_text(label, "New text");
```

---

### VaxpImage

Image display widget.

```c
VaxpWidget* image = vaxp_image_create().value;
vaxp_image_load((VaxpImage*)image, "/path/to/image.png");
vaxp_image_set_fit((VaxpImage*)image, VAXP_IMAGE_FIT_CONTAIN);
```

---

### VaxpIcon

Emoji or icon font display.

```c
VaxpWidget* icon = vaxp_icon(
    .icon = "⚙️",  /* or "settings" for icon fonts */
    .size = 24.0f,
    .color = (VaxpColor){ 97, 97, 97, 255 }
);
```

---

### VaxpAvatar

Circular profile display with initials or image.

```c
VaxpWidget* avatar = vaxp_avatar(
    .initials = "JD",  /* Shows "JD" if no image */
    .image = "/path/to/photo.jpg",
    .size = 40.0f,
    .color = (VaxpColor){ 63, 81, 181, 255 }
);

/* Show online indicator */
vaxp_avatar_set_indicator(avatar, VAXP_TRUE, 
    (VaxpColor){ 76, 175, 80, 255 });
```

---

### VaxpBadge

Notification count indicator.

```c
VaxpWidget* badge = vaxp_badge(
    .count = 5,
    .child = icon_widget,
    .max_count = 99,  /* Shows "99+" if exceeded */
    .dot = VAXP_FALSE  /* Just dot, no number */
);

/* API */
vaxp_badge_set_count(badge, 10);
```

---

### VaxpChip

Tag or filter element.

```c
void on_chip_click(VaxpChip* chip, void* data) {
    printf("Chip clicked\n");
}

VaxpWidget* chip = vaxp_chip(
    .label = "Technology",
    .type = VAXP_CHIP_FILTER,  /* FILTER, CHOICE, INPUT, ACTION */
    .selected = VAXP_FALSE,
    .on_click = on_chip_click
);
```

---

### VaxpCard

Elevated container with shadow.

```c
VaxpWidget* card = vaxp_card(
    .child = content_widget,
    .elevation = 4.0f,
    .corner_radius = 12.0f,
    .padding = 16.0f,
    .outlined = VAXP_FALSE
);
```

---

### VaxpColorSwatch

Color display/picker swatch.

```c
void on_color_click(VaxpColorSwatch* swatch, VaxpColor color, void* data) {
    printf("Color clicked: #%02X%02X%02X\n", color.r, color.g, color.b);
}

VaxpWidget* swatch = vaxp_color_swatch(
    .color = (VaxpColor){ 255, 0, 0, 255 },
    .size = 32.0f,
    .selectable = VAXP_TRUE,
    .on_click = on_color_click
);
```

---

### VaxpLink

Clickable hyperlink.

```c
void on_link_click(VaxpLink* link, const char* url, void* data) {
    printf("Opening: %s\n", url);
    /* Open URL in browser */
}

VaxpWidget* link = vaxp_link(
    .text = "Visit Website",
    .url = "https://VAXP.ORG",
    .on_click = on_link_click
);
```

---

### VaxpSkeleton

Loading placeholder with shimmer animation.

```c
VaxpWidget* skeleton = vaxp_skeleton(
    .variant = VAXP_SKELETON_RECTANGULAR,  /* TEXT, CIRCULAR */
    .width = 200.0f,
    .height = 20.0f
);

/* Call periodically for animation */
vaxp_skeleton_animate(skeleton, delta_time);
```

---

### VaxpCarousel

Image and widget carousel with auto-play and indicators.

```c
VaxpWidget* carousel = vaxp_carousel_create().value;

/* Add slides */
vaxp_carousel_add_item((VaxpCarousel*)carousel, image1);
vaxp_carousel_add_item((VaxpCarousel*)carousel, image2);

/* Configure */
vaxp_carousel_set_auto_play((VaxpCarousel*)carousel, VAXP_TRUE, 3000);
vaxp_carousel_set_show_arrows((VaxpCarousel*)carousel, VAXP_TRUE);
vaxp_carousel_set_indicator((VaxpCarousel*)carousel, VAXP_CAROUSEL_DOTS);
```

---

### VaxpTable

Data grid with sortable columns and row selection.

```c
/* Define columns */
VaxpTableColumn cols[] = {
    { .key = "id", .title = "ID", .width = 50, .sortable = VAXP_TRUE },
    { .key = "name", .title = "Name", .flex = 1, .sortable = VAXP_TRUE },
    { .key = "dpt", .title = "Department", .width = 150 }
};

VaxpWidget* table = vaxp_table_create(cols, 3).value;

/* Add rows */
const char* row1[] = { "1", "John Doe", "Engineering" };
vaxp_table_add_row((VaxpTable*)table, row1, 3);

/* API */
vaxp_table_set_striped((VaxpTable*)table, VAXP_TRUE);
vaxp_table_set_row_height((VaxpTable*)table, 40.0f);
```

---

### VaxpChart

Statistical charts (Line, Bar, Pie, Donut).

```c
VaxpWidget* chart = vaxp_chart_create(VAXP_CHART_BAR).value;

/* Set labels */
const char* labels[] = { "Jan", "Feb", "Mar", "Apr" };
vaxp_chart_set_labels((VaxpChart*)chart, labels, 4);

/* Add dataset */
VaxpF32 values[] = { 10, 25, 15, 30 };
VaxpDataset ds = {
    .label = "Sales",
    .values = values,
    .value_count = 4,
    .color = VAXP_COLOR_BLUE
};
vaxp_chart_add_dataset((VaxpChart*)chart, &ds);

/* Configure */
vaxp_chart_set_title((VaxpChart*)chart, "Quarterly Sales");
vaxp_chart_set_legend_visible((VaxpChart*)chart, VAXP_TRUE);
```

---

## Navigation Widgets

### VaxpTabBar & VaxpTabView

Tab-based navigation.

```c
void on_tab_change(VaxpTabBar* bar, VaxpU32 index, void* data) {
    printf("Tab changed to: %u\n", index);
}

/* Create tab bar */
VaxpTabBar* tabs = (VaxpTabBar*)vaxp_tab_bar_create().value;
vaxp_tab_bar_set_style(tabs, VAXP_TAB_STYLE_UNDERLINE);
vaxp_tab_bar_set_on_change(tabs, on_tab_change, NULL);

/* Create tab view */
VaxpTabView* view = (VaxpTabView*)vaxp_tab_view_create().value;

/* Link them */
vaxp_tab_bar_link_view(tabs, view);

/* Add tabs */
vaxp_tab_bar_add_tab(tabs, "Home", home_content);
vaxp_tab_bar_add_tab(tabs, "Settings", settings_content);
vaxp_tab_bar_add_tab(tabs, "About", about_content);

/* Styles: UNDERLINE, PILL, BOXED */
```

---

### VaxpBreadcrumb

Path navigation.

```c
void on_navigate(VaxpBreadcrumb* bc, VaxpU32 index, const char* path, void* data) {
    printf("Navigate to: %s\n", path);
}

VaxpBreadcrumb* bc = (VaxpBreadcrumb*)vaxp_breadcrumb_create().value;
vaxp_breadcrumb_set_on_navigate(bc, on_navigate, NULL);

/* Add items manually */
vaxp_breadcrumb_add_item(bc, "Home", "/");
vaxp_breadcrumb_add_item(bc, "Documents", "/documents");
vaxp_breadcrumb_add_item(bc, "Report.pdf", "/documents/report.pdf");

/* Or parse from path */
vaxp_breadcrumb_set_path(bc, "/home/user/documents", '/');
```

---

### VaxpTreeView

Hierarchical tree display.

```c
void on_tree_select(VaxpTreeView* tree, VaxpTreeNode* node, void* data) {
    printf("Selected: %s\n", node->label);
}

/* Create tree */
VaxpTreeView* tree = (VaxpTreeView*)vaxp_tree_view_create().value;
vaxp_tree_view_set_on_select(tree, on_tree_select, NULL);

/* Create nodes */
VaxpTreeNode* root = vaxp_tree_node_create("Root", NULL);
VaxpTreeNode* child1 = vaxp_tree_node_create("Child 1", NULL);
VaxpTreeNode* child2 = vaxp_tree_node_create("Child 2", NULL);
VaxpTreeNode* grandchild = vaxp_tree_node_create("Grandchild", NULL);

/* Build hierarchy */
vaxp_tree_node_add_child(root, child1);
vaxp_tree_node_add_child(root, child2);
vaxp_tree_node_add_child(child1, grandchild);

/* Set root */
vaxp_tree_view_add_root(tree, root);

/* API */
vaxp_tree_view_expand_all(tree);
vaxp_tree_view_collapse_all(tree);
```

---

### VaxpListView

Scrollable list with virtualization.

```c
VaxpWidget* build_item(VaxpU32 index, void* data, void* user_data) {
    const char* text = (const char*)data;
    return vaxp_label(.text = text);
}

VaxpListView* list = (VaxpListView*)vaxp_list_view_create().value;
vaxp_list_view_set_builder(list, build_item, NULL);

/* Add items */
vaxp_list_view_add_item(list, "Item 1");
vaxp_list_view_add_item(list, "Item 2");
vaxp_list_view_add_item(list, "Item 3");

/* API */
vaxp_list_view_set_selected(list, 0);
VaxpI32 selected = vaxp_list_view_get_selected(list);
```

---

### VaxpGridView

Grid layout list.

```c
VaxpWidget* build_grid_item(VaxpU32 index, void* data, void* user_data) {
    return vaxp_card(.child = vaxp_label(.text = data));
}

VaxpGridView* grid = (VaxpGridView*)vaxp_grid_view_create().value;
vaxp_grid_view_set_columns(grid, 3);
vaxp_grid_view_set_spacing(grid, 16.0f, 16.0f);
vaxp_grid_view_set_builder(grid, build_grid_item, NULL);
vaxp_grid_view_set_item_size(grid, 0, 150.0f);  /* 0 = auto width */

/* Add items */
for (int i = 0; i < 20; i++) {
    vaxp_grid_view_add_item(grid, "Item");
}
```

---

### VaxpAppBar

Material Design app bar with title, actions, and navigation control.

```c
VaxpWidget* appbar = vaxp_appbar_create("My App").value;

/* Configure */
vaxp_appbar_set_subtitle((VaxpAppBar*)appbar, "Dashboard");
vaxp_appbar_set_elevation((VaxpAppBar*)appbar, 4.0f);
vaxp_appbar_set_center_title((VaxpAppBar*)appbar, VAXP_FALSE);

/* Add entry point (leading) */
vaxp_appbar_set_leading((VaxpAppBar*)appbar, menu_icon);

/* Add actions */
vaxp_appbar_add_action((VaxpAppBar*)appbar, search_btn);
vaxp_appbar_add_action((VaxpAppBar*)appbar, settings_btn);
```

---

### VaxpBottomNav

Bottom navigation bar for top-level views.

```c
/* Define items */
VaxpNavItem items[] = {
    { .icon = "Home", .label = "Home" },
    { .icon = "Search", .label = "Explore" },
    { .icon = "Person", .label = "Profile" }
};

VaxpWidget* nav = vaxp_bottom_nav_create(items, 3).value;

/* Handle selection */
void on_nav(VaxpI32 index, void* data) {
    printf("Selected tab: %d\n", index);
}
vaxp_bottom_nav_set_on_change((VaxpBottomNav*)nav, on_nav, NULL);

/* API */
vaxp_bottom_nav_set_selected((VaxpBottomNav*)nav, 0);
vaxp_bottom_nav_set_color((VaxpBottomNav*)nav, VAXP_COLOR_BLUE);
```

---

### VaxpDrawer

Navigation drawer (sidebar) with header and items.

```c
VaxpWidget* drawer = vaxp_drawer_create().value;

/* Set header */
VaxpWidget* header = create_user_header();
vaxp_drawer_set_header((VaxpDrawer*)drawer, header);

/* Add items */
vaxp_drawer_add_item((VaxpDrawer*)drawer, "inbox", "Inbox", "12");
vaxp_drawer_add_item((VaxpDrawer*)drawer, "star", "Starred", NULL);
vaxp_drawer_add_separator((VaxpDrawer*)drawer);
vaxp_drawer_add_item((VaxpDrawer*)drawer, "settings", "Settings", NULL);

/* Usage */
vaxp_drawer_open((VaxpDrawer*)drawer);
vaxp_drawer_close((VaxpDrawer*)drawer);
```

---

## Feedback & Overlays

### VaxpTooltip

Hover information popup.

```c
VaxpWidget* tooltip = vaxp_tooltip(
    .text = "This is helpful information",
    .child = button_widget,
    .delay_ms = 500,
    .position = VAXP_TOOLTIP_TOP
);
```

---

### VaxpDialog

Modal dialog.

```c
void on_ok(VaxpDialog* dialog, void* data) {
    printf("OK clicked\n");
    vaxp_dialog_close(dialog);
}

void on_cancel(VaxpDialog* dialog, void* data) {
    vaxp_dialog_close(dialog);
}

VaxpDialog* dialog = (VaxpDialog*)vaxp_dialog_create().value;
vaxp_dialog_set_title(dialog, "Confirm Action");
vaxp_dialog_set_content(dialog, content_widget);
vaxp_dialog_add_action(dialog, "Cancel", on_cancel, NULL);
vaxp_dialog_add_action(dialog, "OK", on_ok, NULL);

/* Show dialog */
vaxp_dialog_show(dialog);
```

---

### VaxpContextMenu

Right-click context menu.

```c
void on_menu_item(VaxpContextMenu* menu, VaxpU32 index, void* data) {
    printf("Menu item %u clicked\n", index);
}

VaxpContextMenu* menu = (VaxpContextMenu*)vaxp_context_menu_create().value;
vaxp_context_menu_add_item(menu, "Cut", on_menu_item, NULL);
vaxp_context_menu_add_item(menu, "Copy", on_menu_item, NULL);
vaxp_context_menu_add_item(menu, "Paste", on_menu_item, NULL);
vaxp_context_menu_add_separator(menu);
vaxp_context_menu_add_item(menu, "Delete", on_menu_item, NULL);

/* Show at position */
vaxp_context_menu_show(menu, x, y);
```

---

### VaxpNotification

Toast notification.

```c
void on_dismiss(VaxpNotification* notif, void* data) {
    printf("Notification dismissed\n");
}

VaxpWidget* notif = vaxp_notification(
    .title = "Success",
    .message = "File saved successfully",
    .type = VAXP_NOTIFY_SUCCESS,  /* INFO, SUCCESS, WARNING, ERROR */
    .duration_ms = 4000,
    .dismissible = VAXP_TRUE,
    .position = VAXP_NOTIFY_TOP_RIGHT
);

vaxp_notification_set_on_dismiss(notif, on_dismiss, NULL);
vaxp_notification_show(notif);
```

---

### VaxpSnackbar

Bottom message bar with action.

```c
void on_undo(VaxpSnackbar* bar, void* data) {
    printf("Undo clicked\n");
}

VaxpWidget* snackbar = vaxp_snackbar(
    .message = "Item deleted",
    .action = "UNDO",
    .on_action = on_undo,
    .duration = 5000
);

vaxp_snackbar_show(snackbar);
```

---

### VaxpSpinner

Loading indicator.

```c
VaxpWidget* spinner = vaxp_spinner(
    .size = 40.0f,
    .color = (VaxpColor){ 63, 81, 181, 255 }
);

/* Call in animation loop */
vaxp_spinner_animate(spinner, delta_time);
```

---

## Multi-Step Widgets

### VaxpProgressBar

Linear progress indicator.

```c
VaxpWidget* progress = vaxp_progress_bar(
    .value = 0.75f,  /* 0.0 - 1.0 */
    .indeterminate = VAXP_FALSE,
    .show_label = VAXP_TRUE
);

/* API */
vaxp_progress_bar_set_value(progress, 0.5f);
```

---

### VaxpAccordion

Collapsible sections.

```c
VaxpAccordion* accordion = (VaxpAccordion*)vaxp_accordion_create().value;

vaxp_accordion_add_section(accordion, "Section 1", content1);
vaxp_accordion_add_section(accordion, "Section 2", content2);
vaxp_accordion_add_section(accordion, "Section 3", content3);

/* API */
vaxp_accordion_expand(accordion, 0);
vaxp_accordion_collapse(accordion, 0);
vaxp_accordion_toggle(accordion, 1);
vaxp_accordion_expand_all(accordion);
vaxp_accordion_collapse_all(accordion);
```

---

### VaxpStepper

Step indicator for wizards.

```c
VaxpStepper* stepper = (VaxpStepper*)vaxp_stepper_create().value;

vaxp_stepper_add_step(stepper, "Account");
vaxp_stepper_add_step(stepper, "Details");
vaxp_stepper_add_step(stepper, "Review");
vaxp_stepper_add_step(stepper, "Complete");

/* API */
vaxp_stepper_set_current(stepper, 1);
vaxp_stepper_next(stepper);         /* Marks current as complete, moves to next */
vaxp_stepper_prev(stepper);
vaxp_stepper_complete_step(stepper, 0);
vaxp_stepper_set_error(stepper, 2, VAXP_TRUE);
```

---

## Advanced Pickers

### VaxpPopover

Anchored popup with arrow, multiple placements, and auto-close.

```c
void on_popover_open(VaxpPopover* popover, VaxpBool open, void* data) {
    printf("Popover %s\n", open ? "opened" : "closed");
}

VaxpWidget* popover = vaxp_popover(
    .anchor = button_widget,
    .content = menu_content,
    .placement = VAXP_POPOVER_BOTTOM,  /* TOP, BOTTOM, LEFT, RIGHT + _START/_END */
    .trigger = VAXP_POPOVER_TRIGGER_CLICK,  /* CLICK, HOVER, FOCUS, MANUAL */
    .show_arrow = VAXP_TRUE,
    .offset = 8.0f
);

/* API */
vaxp_popover_open(popover);
vaxp_popover_close(popover);
vaxp_popover_toggle(popover);
VaxpBool is_open = vaxp_popover_is_open(popover);
```

---

### VaxpCalendar

Full calendar widget with month navigation and date selection.

```c
void on_date_select(VaxpCalendar* cal, VaxpDate date, void* data) {
    printf("Selected: %04d-%02d-%02d\n", date.year, date.month, date.day);
}

VaxpWidget* calendar = vaxp_calendar(
    .initial_date = vaxp_date_today(),
    .on_select = on_date_select
);

/* API */
vaxp_calendar_go_to_today(calendar);
vaxp_calendar_prev_month(calendar);
vaxp_calendar_next_month(calendar);
vaxp_calendar_select_date(calendar, (VaxpDate){ 2024, 12, 25 });
VaxpDate selected = vaxp_calendar_get_selected(calendar);
```

---

### VaxpDatePicker

Date input field with dropdown calendar.

```c
void on_date_change(VaxpDatePicker* picker, VaxpDate date, void* data) {
    printf("Date: %04d-%02d-%02d\n", date.year, date.month, date.day);
}

VaxpWidget* date_picker = vaxp_date_picker(
    .initial_date = vaxp_date_today(),
    .placeholder = "Select date...",
    .format = "YYYY-MM-DD",
    .on_change = on_date_change
);

/* API */
vaxp_date_picker_set_date(picker, (VaxpDate){ 2024, 1, 15 });
VaxpDate date = vaxp_date_picker_get_date(picker);
vaxp_date_picker_open(picker);
vaxp_date_picker_close(picker);
```

---

### VaxpTimePicker

Time input with hour/minute/second selection.

```c
void on_time_change(VaxpTimePicker* picker, VaxpTime time, void* data) {
    printf("Time: %02d:%02d:%02d\n", time.hour, time.minute, time.second);
}

VaxpWidget* time_picker = vaxp_time_picker(
    .initial_time = (VaxpTime){ 14, 30, 0 },
    .use_24h = VAXP_TRUE,
    .show_seconds = VAXP_FALSE,
    .on_change = on_time_change
);

/* API */
vaxp_time_picker_set_time(picker, (VaxpTime){ 9, 0, 0 });
VaxpTime time = vaxp_time_picker_get_time(picker);
vaxp_time_picker_set_24h(picker, VAXP_FALSE);
vaxp_time_picker_set_minute_step(picker, 15);  /* 15-minute intervals */
```

---

### VaxpColorPicker

Full HSV color picker with saturation/value square, hue bar, and alpha slider.

```c
void on_color_change(VaxpColorPicker* picker, VaxpColor color, void* data) {
    printf("Color: #%02X%02X%02X (alpha: %d)\n", 
           color.r, color.g, color.b, color.a);
}

VaxpWidget* color_picker = vaxp_color_picker(
    .initial_color = (VaxpColor){ 255, 0, 0, 255 },
    .mode = VAXP_COLOR_PICKER_HSV,  /* HSV, RGB, PALETTE */
    .show_alpha = VAXP_TRUE,
    .on_change = on_color_change
);

/* API */
vaxp_color_picker_set_color(picker, (VaxpColor){ 0, 128, 255, 255 });
VaxpColor color = vaxp_color_picker_get_color(picker);
vaxp_color_picker_set_show_alpha(picker, VAXP_FALSE);
```

---

### VaxpFileChooser

File browser with directory navigation, filtering, and selection.

```c
void on_file_confirm(VaxpFileChooser* chooser, const char* path, void* data) {
    printf("Selected file: %s\n", path);
}

VaxpWidget* file_chooser = vaxp_file_chooser(
    .mode = VAXP_FILE_CHOOSER_OPEN,  /* OPEN, SAVE, SELECT_FOLDER, OPEN_MULTIPLE */
    .initial_path = "/home/user",
    .filter = "*.txt;*.md",
    .on_confirm = on_file_confirm
);

/* API */
vaxp_file_chooser_set_path(chooser, "/home/user/documents");
const char* path = vaxp_file_chooser_get_path(chooser);
const char* selected = vaxp_file_chooser_get_selected(chooser);
vaxp_file_chooser_set_filter(chooser, "*.png;*.jpg");
vaxp_file_chooser_set_show_hidden(chooser, VAXP_TRUE);
vaxp_file_chooser_go_up(chooser);
vaxp_file_chooser_refresh(chooser);
```

---

## Desktop UI Widgets

### VaxpMenuBar

Classic desktop-style menu bar (File, Edit, View, etc.).

```c
/* Create menu bar */
VaxpMenuBar* menubar = (VaxpMenuBar*)vaxp_menubar_create().value;

/* Create submenus using ContextMenu */
VaxpContextMenu* file_menu = (VaxpContextMenu*)vaxp_context_menu_create().value;
vaxp_context_menu_add_item(file_menu, "New", on_new, NULL);
vaxp_context_menu_add_item(file_menu, "Open", on_open, NULL);
vaxp_context_menu_add_separator(file_menu);
vaxp_context_menu_add_item(file_menu, "Exit", on_exit, NULL);

VaxpContextMenu* edit_menu = (VaxpContextMenu*)vaxp_context_menu_create().value;
vaxp_context_menu_add_item(edit_menu, "Undo", on_undo, NULL);
vaxp_context_menu_add_item(edit_menu, "Redo", on_redo, NULL);

/* Add menus to bar */
vaxp_menubar_add_menu(menubar, "File", file_menu);
vaxp_menubar_add_menu(menubar, "Edit", edit_menu);

/* Or with mnemonic (Alt+F) */
vaxp_menubar_add_menu_with_mnemonic(menubar, "_File", file_menu);

/* API */
vaxp_menubar_set_background(menubar, (VaxpColor){240, 240, 240, 255});
vaxp_menubar_set_height(menubar, 28.0f);
vaxp_menubar_open_menu(menubar, 0);  /* Open first menu */
vaxp_menubar_close_menus(menubar);
```

---

### VaxpToolbar

Horizontal bar with tool buttons.

```c
void on_tool_click(VaxpToolbar* bar, VaxpU32 index, void* data) {
    printf("Tool %u clicked\n", index);
}

/* Create toolbar */
VaxpToolbar* toolbar = (VaxpToolbar*)vaxp_toolbar_create().value;

/* Add buttons */
vaxp_toolbar_add_button(toolbar, "📄", "New", "Create new file", on_tool_click, NULL);
vaxp_toolbar_add_button(toolbar, "📂", "Open", "Open file", on_tool_click, NULL);
vaxp_toolbar_add_button(toolbar, "💾", "Save", "Save file", on_tool_click, NULL);

/* Add separator */
vaxp_toolbar_add_separator(toolbar);

/* Add toggle buttons */
vaxp_toolbar_add_toggle(toolbar, "B", "Bold", "Toggle bold", VAXP_FALSE, on_tool_click, NULL);
vaxp_toolbar_add_toggle(toolbar, "I", "Italic", "Toggle italic", VAXP_FALSE, on_tool_click, NULL);

/* Add flexible spacer (pushes remaining items to right) */
vaxp_toolbar_add_spacer(toolbar);

/* API */
vaxp_toolbar_set_style(toolbar, VAXP_TOOLBAR_STYLE_ICON_ONLY);  /* ICON_ONLY, TEXT_ONLY, ICON_TEXT */
vaxp_toolbar_set_height(toolbar, 40.0f);
vaxp_toolbar_set_button_size(toolbar, 32.0f);
vaxp_toolbar_set_enabled(toolbar, 2, VAXP_FALSE);  /* Disable save button */
vaxp_toolbar_set_toggled(toolbar, 3, VAXP_TRUE);   /* Set bold on */
```

---

### VaxpTitleBar

Custom window title bar with control buttons.

```c
void on_close(VaxpTitleBar* bar, void* data) {
    printf("Close button clicked\n");
    /* Exit app */
}

void on_drag(VaxpTitleBar* bar, VaxpI32 dx, VaxpI32 dy, void* data) {
    /* Move window */
    vaxp_window_move_by(window, dx, dy);
}

/* Create title bar */
VaxpTitleBar* titlebar = (VaxpTitleBar*)vaxp_titlebar_create("My Application").value;

/* Set callbacks */
vaxp_titlebar_on_minimize(titlebar, on_minimize, NULL);
vaxp_titlebar_on_maximize(titlebar, on_maximize, NULL);
vaxp_titlebar_on_restore(titlebar, on_restore, NULL);  /* When clicking max while maximized */
vaxp_titlebar_on_close(titlebar, on_close, NULL);
vaxp_titlebar_on_drag(titlebar, on_drag, NULL);  /* For window movement */

/* Configure buttons */
vaxp_titlebar_set_buttons(titlebar, 
    VAXP_TITLEBAR_BUTTON_MINIMIZE | 
    VAXP_TITLEBAR_BUTTON_MAXIMIZE | 
    VAXP_TITLEBAR_BUTTON_CLOSE);

/* API */
vaxp_titlebar_set_title(titlebar, "New Title");
vaxp_titlebar_set_maximized(titlebar, VAXP_TRUE);  /* Changes maximize icon */
vaxp_titlebar_set_center_title(titlebar, VAXP_TRUE);
vaxp_titlebar_set_height(titlebar, 32.0f);

/* Themes */
vaxp_titlebar_apply_dark_theme(titlebar);
vaxp_titlebar_apply_light_theme(titlebar);

/* Add custom widgets */
vaxp_titlebar_add_left_widget(titlebar, menu_button);
vaxp_titlebar_add_right_widget(titlebar, search_button);
```

---

## Complete Example


```c
#include <vaxpui.h>

void on_button_click(VaxpWidget* btn, void* data) {
    printf("Button clicked!\n");
}

int main(void) {
    /* Initialize VAXPUI */
    vaxp_init();
    
    /* Create main window */
    VaxpWindow* window = vaxp_window_create("My App", 800, 600);
    
    /* Create layout */
    VaxpWidget* root = vaxp_container(
        .direction = VAXP_DIRECTION_VERTICAL,
        .padding = (VaxpInsets){ 20, 20, 20, 20 },
        .spacing = 16
    );
    
    /* Add header */
    VaxpWidget* header = vaxp_label(
        .text = "Welcome to VAXPUI",
        .font_size = 24
    );
    vaxp_container_add_child((VaxpContainer*)root, header);
    
    /* Add input */
    VaxpWidget* input = vaxp_text_input(
        .placeholder = "Enter your name..."
    );
    vaxp_container_add_child((VaxpContainer*)root, input);
    
    /* Add button */
    VaxpWidget* button = vaxp_button(
        .label = "Submit",
        .on_click = on_button_click
    );
    vaxp_container_add_child((VaxpContainer*)root, button);
    
    /* Set window content */
    vaxp_window_set_content(window, root);
    
    /* Run application */
    vaxp_app_run();
    
    /* Cleanup */
    vaxp_shutdown();
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

*Documentation generated for VAXPUI v0.1.0*
