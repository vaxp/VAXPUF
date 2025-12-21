# VENOMUI

**إطار عمل واجهة مستخدم رسومية عالي الأداء مكتوب بـ C**

VENOMUI هو إطار GUI مصمم مع التركيز على:
- 🧹 **إدارة ذاكرة صارمة** - Reference counting مع تتبع التسريبات
- 🎨 **رسم عالي الأداء** - مبني على Skia
- 🖥️ **وصول أصلي** - تكامل مباشر مع X11/Wayland
- 🧩 **بنية قابلة للصيانة** - VTable inheritance واضح

## البنية الأساسية

```
VENOMUI/
├── include/venom/
│   ├── core/           # الأنواع، الذاكرة، Ref counting، Results
│   ├── backend/        # X11/Wayland abstraction
│   ├── graphics/       # Canvas API (Skia)
│   └── widgets/        # Widget system
├── src/
│   ├── core/
│   ├── backend/x11/
│   ├── graphics/
│   └── widgets/
└── examples/
```

## المميزات الرئيسية

### 1. إدارة الذاكرة
```c
// كل كائن مرجعي
VenomButton* btn = venom_button_create("Click Me");
venom_ref(btn);    // +1
venom_unref(btn);  // -1, يُحرر تلقائياً عند 0
```

### 2. معالجة الأخطاء
```c
VenomResultPtr result = venom_display_open(VENOM_BACKEND_X11, NULL);
if (!result.ok) {
    printf("Error: %s\n", venom_error_string(result.error));
    return;
}
VenomDisplay* display = result.value;
```

### 3. تخطيط Flexbox
```c
VenomContainer* row = venom_container_create_row();
venom_container_set_gap(row, 10);
venom_container_set_justify(row, VENOM_JUSTIFY_SPACE_BETWEEN);
venom_widget_add_child(row, button1);
venom_widget_add_child(row, button2);
```

## البناء

```bash
# التبعيات (Arch Linux)
sudo pacman -S meson ninja libx11 skia

# البناء
meson setup build
meson compile -C build

# تشغيل المثال
./build/examples/hello_world
```

## الخطوات القادمة

- [ ] System Tray integration
- [ ] Wayland backend
- [ ] Theming system
- [ ] More widgets (TextField, ScrollView, etc.)

## الترخيص

MIT License
# -Venom-UI-Framework
