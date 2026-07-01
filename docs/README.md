# VAXPUI Documentation

Welcome to VAXPUI - A high-performance, Flutter-like GUI framework for Linux written in C.

## 📚 Documentation

| Document | Description |
|----------|-------------|
| [Getting Started](getting-started.md) | Quick setup and first application |
| [Widgets Guide](widgets.md) | All available widgets and usage |
| [State Management](state-management.md) | BLoC/Cubit pattern for state |
| [Theming](theming.md) | Colors, typography, and themes |
| [Events & Input](events.md) | Keyboard, mouse, and input handling |
| [Internationalization](i18n.md) | Arabic, Unicode, and RTL support |
| [API Reference](api-reference.md) | Complete API documentation |

## 🚀 Quick Example

```c
#include <vaxp/vaxpui.h>

VaxpWidget* build_app(void* data) {
    return vaxp_center(
        .children = VAXP_CHILDREN(
            vaxp_text("مرحباً بالعالم!"),
            vaxp_btn("Click Me", .on_click = handle_click)
        )
    );
}

int main(void) {
    return VAXP_APP(
        .title = "My App",
        .width = 800,
        .height = 600,
        .build = build_app
    );
}
```

## 🏗️ Architecture

```
VAXPUI
├── Core (Memory, Types, Threading)
├── Backend (X11, Events, Windows)
├── Graphics (Cairo + Pango)
├── Widgets (Button, Label, Container, etc.)
└── State (BLoC/Cubit Pattern)
```

## 📦 Dependencies

- X11 (libx11-dev)
- Cairo (libcairo2-dev)
- PangoCairo (libpango1.0-dev)
- libpng (libpng-dev)

## 🔧 Building

```bash
meson setup build
meson compile -C build
```

## 📄 License

MIT License
