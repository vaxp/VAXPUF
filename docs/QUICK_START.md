# VENOMUI Quick Start Guide

## Installation

```bash
# Clone repository
git clone https://github.com/your-repo/venomui.git
cd venomui

# Build
meson setup build
meson compile -C build

# Install (optional)
sudo meson install -C build
```

## Project Setup

### Using pkg-config (after installation)

```bash
gcc myapp.c -o myapp $(pkg-config --cflags --libs venomui)
```

### Using Meson subproject

In your `meson.build`:
```meson
venomui_proj = subproject('venomui')
venomui_dep = venomui_proj.get_variable('venomui_dep')

executable('myapp', 'main.c', dependencies: venomui_dep)
```

---

## Hello World

```c
#include <venomui.h>

int main(void) {
    /* Initialize */
    venom_init();
    
    /* Create window */
    VenomWindow* window = venom_window_create("Hello VENOMUI", 400, 300);
    
    /* Create content */
    VenomWidget* content = venom_container(
        .direction = VENOM_DIRECTION_VERTICAL,
        .padding = (VenomInsets){ 20, 20, 20, 20 },
        .alignment = VENOM_ALIGN_CENTER
    );
    
    VenomWidget* label = venom_label(
        .text = "Hello, World!",
        .font_size = 32.0f
    );
    
    venom_container_add_child((VenomContainer*)content, label);
    venom_window_set_content(window, content);
    
    /* Run */
    venom_app_run();
    venom_shutdown();
    
    return 0;
}
```

---

## Common Patterns

### Layout Pattern

```c
/* Vertical layout with header, content, footer */
VenomWidget* root = venom_container(.direction = VENOM_DIRECTION_VERTICAL);

VenomWidget* header = create_header();
VenomWidget* content = venom_container(.direction = VENOM_DIRECTION_VERTICAL);
content->layout.flex = 1.0f;  /* Take remaining space */
VenomWidget* footer = create_footer();

venom_container_add_child(root, header);
venom_container_add_child(root, content);
venom_container_add_child(root, footer);
```

### Form Pattern

```c
VenomWidget* form = venom_container(
    .direction = VENOM_DIRECTION_VERTICAL,
    .spacing = 12.0f
);

/* Name field */
VenomWidget* name_row = venom_container(.direction = VENOM_DIRECTION_VERTICAL, .spacing = 4);
venom_container_add_child(name_row, venom_label(.text = "Name"));
venom_container_add_child(name_row, venom_text_input(.placeholder = "Enter name"));
venom_container_add_child(form, name_row);

/* Email field */
VenomWidget* email_row = venom_container(.direction = VENOM_DIRECTION_VERTICAL, .spacing = 4);
venom_container_add_child(email_row, venom_label(.text = "Email"));
venom_container_add_child(email_row, venom_text_input(.placeholder = "Enter email"));
venom_container_add_child(form, email_row);

/* Submit button */
venom_container_add_child(form, venom_button(.label = "Submit", .on_click = on_submit));
```

### Card List Pattern

```c
VenomWidget* build_card(const char* title, const char* desc) {
    VenomWidget* content = venom_container(
        .direction = VENOM_DIRECTION_VERTICAL,
        .spacing = 8
    );
    venom_container_add_child(content, venom_label(.text = title, .font_size = 18));
    venom_container_add_child(content, venom_label(.text = desc));
    
    return venom_card(.child = content, .elevation = 2, .padding = 16);
}

VenomWidget* card_list = venom_container(.direction = VENOM_DIRECTION_VERTICAL, .spacing = 16);
venom_container_add_child(card_list, build_card("Card 1", "Description 1"));
venom_container_add_child(card_list, build_card("Card 2", "Description 2"));
```

### Tab Navigation Pattern

```c
/* Create pages */
VenomWidget* home_page = create_home_page();
VenomWidget* settings_page = create_settings_page();
VenomWidget* about_page = create_about_page();

/* Create tab bar and view */
VenomTabBar* tabs = (VenomTabBar*)venom_tab_bar_create().value;
VenomTabView* view = (VenomTabView*)venom_tab_view_create().value;
venom_tab_bar_link_view(tabs, view);

venom_tab_bar_add_tab(tabs, "Home", home_page);
venom_tab_bar_add_tab(tabs, "Settings", settings_page);
venom_tab_bar_add_tab(tabs, "About", about_page);

/* Layout */
VenomWidget* layout = venom_container(.direction = VENOM_DIRECTION_VERTICAL);
venom_container_add_child(layout, (VenomWidget*)tabs);
venom_container_add_child(layout, (VenomWidget*)view);
((VenomWidget*)view)->layout.flex = 1.0f;
```

---

## Styling

### Colors

```c
/* Predefined colors */
VenomColor red = VENOM_COLOR_RED;
VenomColor blue = VENOM_COLOR_BLUE;
VenomColor white = VENOM_COLOR_WHITE;
VenomColor transparent = VENOM_COLOR_TRANSPARENT;

/* Custom colors */
VenomColor custom = { 
    .r = 100, 
    .g = 150, 
    .b = 200, 
    .a = 255  /* 255 = opaque */
};
```

### Insets (Padding/Margin)

```c
/* All sides equal */
VenomInsets all = { 16, 16, 16, 16 };  /* top, right, bottom, left */

/* Symmetric */
VenomInsets symmetric = { 10, 20, 10, 20 };  /* vertical=10, horizontal=20 */
```

---

## Event Handling

### Button Click

```c
void handle_click(VenomWidget* widget, void* user_data) {
    const char* message = (const char*)user_data;
    printf("Clicked: %s\n", message);
}

VenomWidget* btn = venom_button(
    .label = "Click Me",
    .on_click = handle_click,
    .data = "Button was clicked!"
);
```

### Input Change

```c
void handle_change(VenomTextInput* input, const char* text, void* data) {
    printf("Text changed: %s\n", text);
}

VenomWidget* input = venom_text_input(
    .on_change = handle_change
);
```

---

## Best Practices

1. **Always check result.ok** for create functions
2. **Use reference counting** - `venom_ref()` and `venom_unref()`
3. **Set layout.flex** for flexible sizing
4. **Use compound literals** for inline config
5. **Prefer macros** (`venom_button(...)`) over direct create calls

---

## Debugging

```bash
# Build with debug
meson setup build -Ddebug_memory=true
meson compile -C build

# Run with memory leak detection
VENOM_DEBUG=1 ./build/myapp
```
