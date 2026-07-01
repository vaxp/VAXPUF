# VAXPUI Quick Start Guide

## Installation

```bash
# Clone repository
git clone https://github.com/your-repo/vaxpui.git
cd vaxpui

# Build
meson setup build
meson compile -C build

# Install (optional)
sudo meson install -C build
```

## Project Setup

### Using pkg-config (after installation)

```bash
gcc myapp.c -o myapp $(pkg-config --cflags --libs vaxpui)
```

### Using Meson subproject

In your `meson.build`:
```meson
vaxpui_proj = subproject('vaxpui')
vaxpui_dep = vaxpui_proj.get_variable('vaxpui_dep')

executable('myapp', 'main.c', dependencies: vaxpui_dep)
```

---

## Hello World

```c
#include <vaxpui.h>

int main(void) {
    /* Initialize */
    vaxp_init();
    
    /* Create window */
    VaxpWindow* window = vaxp_window_create("Hello VAXPUI", 400, 300);
    
    /* Create content */
    VaxpWidget* content = vaxp_container(
        .direction = VAXP_DIRECTION_VERTICAL,
        .padding = (VaxpInsets){ 20, 20, 20, 20 },
        .alignment = VAXP_ALIGN_CENTER
    );
    
    VaxpWidget* label = vaxp_label(
        .text = "Hello, World!",
        .font_size = 32.0f
    );
    
    vaxp_container_add_child((VaxpContainer*)content, label);
    vaxp_window_set_content(window, content);
    
    /* Run */
    vaxp_app_run();
    vaxp_shutdown();
    
    return 0;
}
```

---

## Common Patterns

### Layout Pattern

```c
/* Vertical layout with header, content, footer */
VaxpWidget* root = vaxp_container(.direction = VAXP_DIRECTION_VERTICAL);

VaxpWidget* header = create_header();
VaxpWidget* content = vaxp_container(.direction = VAXP_DIRECTION_VERTICAL);
content->layout.flex = 1.0f;  /* Take remaining space */
VaxpWidget* footer = create_footer();

vaxp_container_add_child(root, header);
vaxp_container_add_child(root, content);
vaxp_container_add_child(root, footer);
```

### Form Pattern

```c
VaxpWidget* form = vaxp_container(
    .direction = VAXP_DIRECTION_VERTICAL,
    .spacing = 12.0f
);

/* Name field */
VaxpWidget* name_row = vaxp_container(.direction = VAXP_DIRECTION_VERTICAL, .spacing = 4);
vaxp_container_add_child(name_row, vaxp_label(.text = "Name"));
vaxp_container_add_child(name_row, vaxp_text_input(.placeholder = "Enter name"));
vaxp_container_add_child(form, name_row);

/* Email field */
VaxpWidget* email_row = vaxp_container(.direction = VAXP_DIRECTION_VERTICAL, .spacing = 4);
vaxp_container_add_child(email_row, vaxp_label(.text = "Email"));
vaxp_container_add_child(email_row, vaxp_text_input(.placeholder = "Enter email"));
vaxp_container_add_child(form, email_row);

/* Submit button */
vaxp_container_add_child(form, vaxp_button(.label = "Submit", .on_click = on_submit));
```

### Card List Pattern

```c
VaxpWidget* build_card(const char* title, const char* desc) {
    VaxpWidget* content = vaxp_container(
        .direction = VAXP_DIRECTION_VERTICAL,
        .spacing = 8
    );
    vaxp_container_add_child(content, vaxp_label(.text = title, .font_size = 18));
    vaxp_container_add_child(content, vaxp_label(.text = desc));
    
    return vaxp_card(.child = content, .elevation = 2, .padding = 16);
}

VaxpWidget* card_list = vaxp_container(.direction = VAXP_DIRECTION_VERTICAL, .spacing = 16);
vaxp_container_add_child(card_list, build_card("Card 1", "Description 1"));
vaxp_container_add_child(card_list, build_card("Card 2", "Description 2"));
```

### Tab Navigation Pattern

```c
/* Create pages */
VaxpWidget* home_page = create_home_page();
VaxpWidget* settings_page = create_settings_page();
VaxpWidget* about_page = create_about_page();

/* Create tab bar and view */
VaxpTabBar* tabs = (VaxpTabBar*)vaxp_tab_bar_create().value;
VaxpTabView* view = (VaxpTabView*)vaxp_tab_view_create().value;
vaxp_tab_bar_link_view(tabs, view);

vaxp_tab_bar_add_tab(tabs, "Home", home_page);
vaxp_tab_bar_add_tab(tabs, "Settings", settings_page);
vaxp_tab_bar_add_tab(tabs, "About", about_page);

/* Layout */
VaxpWidget* layout = vaxp_container(.direction = VAXP_DIRECTION_VERTICAL);
vaxp_container_add_child(layout, (VaxpWidget*)tabs);
vaxp_container_add_child(layout, (VaxpWidget*)view);
((VaxpWidget*)view)->layout.flex = 1.0f;
```

---

## Styling

### Colors

```c
/* Predefined colors */
VaxpColor red = VAXP_COLOR_RED;
VaxpColor blue = VAXP_COLOR_BLUE;
VaxpColor white = VAXP_COLOR_WHITE;
VaxpColor transparent = VAXP_COLOR_TRANSPARENT;

/* Custom colors */
VaxpColor custom = { 
    .r = 100, 
    .g = 150, 
    .b = 200, 
    .a = 255  /* 255 = opaque */
};
```

### Insets (Padding/Margin)

```c
/* All sides equal */
VaxpInsets all = { 16, 16, 16, 16 };  /* top, right, bottom, left */

/* Symmetric */
VaxpInsets symmetric = { 10, 20, 10, 20 };  /* vertical=10, horizontal=20 */
```

---

## Event Handling

### Button Click

```c
void handle_click(VaxpWidget* widget, void* user_data) {
    const char* message = (const char*)user_data;
    printf("Clicked: %s\n", message);
}

VaxpWidget* btn = vaxp_button(
    .label = "Click Me",
    .on_click = handle_click,
    .data = "Button was clicked!"
);
```

### Input Change

```c
void handle_change(VaxpTextInput* input, const char* text, void* data) {
    printf("Text changed: %s\n", text);
}

VaxpWidget* input = vaxp_text_input(
    .on_change = handle_change
);
```

---

## Best Practices

1. **Always check result.ok** for create functions
2. **Use reference counting** - `vaxp_ref()` and `vaxp_unref()`
3. **Set layout.flex** for flexible sizing
4. **Use compound literals** for inline config
5. **Prefer macros** (`vaxp_button(...)`) over direct create calls

---

## Debugging

```bash
# Build with debug
meson setup build -Ddebug_memory=true
meson compile -C build

# Run with memory leak detection
VAXP_DEBUG=1 ./build/myapp
```
