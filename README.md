# VAXPUI

**High-Performance GUI Framework for C**

VAXPUI is a feature-rich GUI framework designed for:
- 🧹 **Strict Memory Management** - Reference counting with leak detection
- 🎨 **High Performance Rendering** - Built on Cairo/Skia
- 🖥️ **Native Access** - Direct X11/Wayland integration
- 🧩 **Maintainable Architecture** - Clear VTable inheritance
- 📦 **45+ Ready-to-Use Widgets** - Complete UI toolkit

## Documentation

| Document | Description |
|----------|-------------|
| [Quick Start](docs/QUICK_START.md) | Getting started guide |
| [Widget Reference](docs/WIDGET_REFERENCE.md) | Complete API documentation |
| [Cheat Sheet](docs/CHEAT_SHEET.md) | Quick reference |

## Widget Library

### Layout
`Container` `Stack` `Spacer` `Divider` `SizedBox` `Padding` `SplitPane` `Scrollable`

### Input Controls
`Button` `Checkbox` `Switch` `Slider` `Radio` `TextInput` `TextArea` `Dropdown` `SearchBar` `NumberInput` `ToggleButton` `Rating`

### Display
`Label` `Image` `Icon` `Avatar` `Badge` `Chip` `Card` `ColorSwatch` `Link` `Skeleton`

### Navigation
`TabBar` `TabView` `Breadcrumb` `TreeView` `ListView` `GridView`

### Feedback
`Tooltip` `Dialog` `ContextMenu` `Notification` `Snackbar` `Spinner` `ProgressBar`

### Multi-Step
`Accordion` `Stepper`

## Quick Example

```c
#include <vaxpui.h>

void on_click(VaxpWidget* btn, void* data) {
    printf("Hello VAXPUI!\n");
}

int main(void) {
    vaxp_init();
    
    VaxpWindow* window = vaxp_window_create("My App", 800, 600);
    
    VaxpWidget* content = vaxp_container(
        .direction = VAXP_DIRECTION_VERTICAL,
        .padding = (VaxpInsets){ 20, 20, 20, 20 },
        .spacing = 16
    );
    
    vaxp_container_add_child(content, vaxp_label(.text = "Welcome!"));
    vaxp_container_add_child(content, vaxp_button(.label = "Click Me", .on_click = on_click));
    
    vaxp_window_set_content(window, content);
    vaxp_app_run();
    vaxp_shutdown();
    
    return 0;
}
```

## Building

```bash
# Dependencies (Debian/Ubuntu)
sudo apt install meson ninja-build libx11-dev libcairo2-dev libpango1.0-dev libpng-dev

# Build
meson setup build
meson compile -C build
meson configure build -Dopengl=true && ninja -C build
# Run example
./build/examples/hello_world
```

## Project Structure

```
VAXPUI/
├── include/vaxp/
│   ├── core/           # Types, Memory, Ref counting
│   ├── backend/        # X11/Wayland abstraction
│   ├── graphics/       # Canvas API
│   ├── widgets/        # 45+ widget headers
│   └── state/          # BLoC state management
├── src/
│   ├── core/
│   ├── backend/x11/
│   ├── graphics/
│   ├── widgets/
│   └── state/
├── docs/               # Documentation
└── examples/           # Example applications
```

## Roadmap

- [x] Core widget set (45 widgets)
- [x] X11 backend
- [x] Cairo rendering
- [ ] Wayland backend
- [ ] Theming system
- [ ] Desktop widgets (TitleBar, Toolbar, StatusBar)
- [ ] Date/Time pickers

## License

MIT License
