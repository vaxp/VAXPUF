# API Reference

Complete API reference for VENOMUI.

---

## Core

### Initialization

```c
VenomResult venom_init(void);
void venom_shutdown(void);
const char* venom_version_string(void);
```

### Memory

```c
void* venom_alloc(VenomSize size);
void* venom_realloc(void* ptr, VenomSize old_size, VenomSize new_size);
void venom_free(void* ptr, VenomSize size);

// Debug
void venom_memory_dump_stats(void);
```

### Reference Counting

```c
void venom_ref(void* obj);
void venom_unref(void* obj);
VenomU32 venom_ref_count(void* obj);

// Macro for creating ref-counted objects
VENOM_REF_NEW(Type, destructor)
```

---

## App

```c
// Macro for running application
VENOM_APP(
    .title = "Title",
    .width = 800,
    .height = 600,
    .build = build_function
)

// Build function signature
VenomWidget* (*VenomBuildFunc)(void* data);
```

---

## Widgets

### VenomWidget (Base)

```c
VenomResultPtr venom_widget_create(const VenomWidgetClass* cls);
void venom_widget_destroy(VenomWidget* widget);
void venom_widget_invalidate(VenomWidget* widget);
void venom_widget_invalidate_layout(VenomWidget* widget);
void venom_widget_measure(VenomWidget* widget, ...);
void venom_widget_layout(VenomWidget* widget, VenomRectF bounds);
void venom_widget_draw(VenomWidget* widget, VenomCanvas* canvas);
```

### VenomButton

```c
VenomResultPtr venom_button_create(void);
void venom_button_set_text(VenomButton* btn, const char* text);
void venom_button_set_on_click(VenomButton* btn, VenomButtonCallback cb, void* data);

// Callback signature
typedef void (*VenomButtonCallback)(VenomButton* btn, void* user_data);

// Macro
venom_btn(text, .on_click = handler)
```

### VenomLabel

```c
VenomResultPtr venom_label_create(void);
void venom_label_set_text(VenomLabel* label, const char* text);

// Macro
venom_text("Text content")
```

### VenomTextInput

```c
VenomResultPtr venom_text_input_create(void);
void venom_text_input_set_text(VenomTextInput* input, const char* text);
const char* venom_text_input_get_text(const VenomTextInput* input);
void venom_text_input_set_placeholder(VenomTextInput* input, const char* placeholder);
void venom_text_input_set_password_mode(VenomTextInput* input, VenomBool password);
void venom_text_input_set_max_length(VenomTextInput* input, VenomU32 max);
void venom_text_input_set_on_change(VenomTextInput* input, VenomTextInputCallback cb, void* data);
void venom_text_input_set_on_submit(VenomTextInput* input, VenomTextInputCallback cb, void* data);

// Callback signature
typedef void (*VenomTextInputCallback)(VenomTextInput* input, const char* text, void* data);

// Macro
venom_input(.placeholder = "Enter text", .on_change = handler)
```

### VenomContainer

```c
VenomResultPtr venom_container_create(VenomLayoutType type);
void venom_container_add_child(VenomContainer* container, VenomWidget* child);
void venom_container_remove_child(VenomContainer* container, VenomWidget* child);
void venom_container_clear(VenomContainer* container);

// Macros
venom_col(.gap = 10, .children = VENOM_CHILDREN(...))
venom_row(.gap = 10, .children = VENOM_CHILDREN(...))
venom_center(.children = VENOM_CHILDREN(...))
```

### VenomScrollable

```c
VenomResultPtr venom_scrollable_create(void);
VenomResult venom_scrollable_set_content(VenomScrollable* scroll, VenomWidget* content);
void venom_scrollable_get_scroll(const VenomScrollable* scroll, VenomF32* x, VenomF32* y);
void venom_scrollable_set_scroll(VenomScrollable* scroll, VenomF32 x, VenomF32 y);
void venom_scrollable_scroll_by(VenomScrollable* scroll, VenomF32 dx, VenomF32 dy);
void venom_scrollable_ensure_visible(VenomScrollable* scroll, VenomWidget* widget);
void venom_scrollable_set_direction(VenomScrollable* scroll, VenomScrollDirection dir);

// Macro
venom_scroll(.direction = VENOM_SCROLL_VERTICAL, .content = child)
```

### VenomImageWidget

```c
VenomResultPtr venom_image_load_file(const char* path);
VenomResultPtr venom_image_widget_create(void);
VenomResult venom_image_widget_set_image(VenomImageWidget* widget, VenomImageData* image);
VenomResult venom_image_widget_load(VenomImageWidget* widget, const char* path);
void venom_image_widget_set_fit(VenomImageWidget* widget, VenomImageFit fit);
void venom_image_widget_set_corner_radius(VenomImageWidget* widget, VenomF32 radius);

// Macro
venom_image(.src = "path.png", .fit = VENOM_IMAGE_FIT_CONTAIN)
```

---

## Focus

```c
void venom_focus_init(void);
void venom_focus_set_root(VenomWidget* root);
void venom_focus_set(VenomWidget* widget);
VenomWidget* venom_focus_get(void);
VenomBool venom_focus_has(VenomWidget* widget);
void venom_focus_next(void);
void venom_focus_prev(void);
```

---

## Theme

```c
const VenomTheme* venom_theme_get_current(void);
void venom_theme_set(const VenomTheme* theme);
const VenomTheme* venom_theme_light(void);
const VenomTheme* venom_theme_dark(void);
VenomTheme venom_theme_create(VenomColor primary, VenomColor secondary, VenomBool is_dark);
```

---

## State Management

### Cubit

```c
VenomCubit* venom_cubit_create(VenomSize state_size, void* initial_state);
void venom_cubit_destroy(VenomCubit* cubit);
void venom_cubit_emit(VenomCubit* cubit, void* new_state);
void venom_cubit_add_listener(VenomCubit* cubit, VenomCubitListener listener, void* data);
void venom_cubit_remove_listener(VenomCubit* cubit, VenomCubitListener listener);

// Listener signature
typedef void (*VenomCubitListener)(VenomCubit* cubit, void* state, void* user_data);

// Macros
VENOM_DEFINE_CUBIT(Name, StateType, actions...)
VENOM_CUBIT_CREATE(Name, StateType, initial_state)
VENOM_CUBIT_EMIT(cubit, Name, action)
```

### BlocBuilder

```c
VENOM_BLOC_BUILDER(
    .cubit = my_cubit,
    .builder = builder_function,
    .user_data = optional_data
)

// Builder signature
VenomWidget* (*builder)(void* state, void* user_data);
```

---

## Graphics

### Canvas

```c
void venom_canvas_save(VenomCanvas* canvas);
void venom_canvas_restore(VenomCanvas* canvas);
void venom_canvas_translate(VenomCanvas* canvas, VenomF32 dx, VenomF32 dy);
void venom_canvas_scale(VenomCanvas* canvas, VenomF32 sx, VenomF32 sy);
void venom_canvas_rotate(VenomCanvas* canvas, VenomF32 degrees);
void venom_canvas_clip_rect(VenomCanvas* canvas, VenomRectF rect);
void venom_canvas_clip_rounded_rect(VenomCanvas* canvas, VenomRectF rect, VenomF32 radius);
void venom_canvas_clear(VenomCanvas* canvas, VenomColor color);
void venom_canvas_draw_rect(VenomCanvas* canvas, VenomRectF rect, const VenomPaint* paint);
void venom_canvas_draw_rounded_rect(VenomCanvas* canvas, VenomRectF rect, VenomF32 radius, const VenomPaint* paint);
void venom_canvas_draw_circle(VenomCanvas* canvas, VenomF32 cx, VenomF32 cy, VenomF32 r, const VenomPaint* paint);
void venom_canvas_draw_text(VenomCanvas* canvas, const char* text, VenomF32 x, VenomF32 y, ...);
void venom_canvas_draw_image(VenomCanvas* canvas, const VenomImage* image, VenomF32 x, VenomF32 y);
void venom_canvas_flush(VenomCanvas* canvas);
```

---

## Types

### Basic Types

```c
typedef int8_t   VenomI8;
typedef int16_t  VenomI16;
typedef int32_t  VenomI32;
typedef int64_t  VenomI64;
typedef uint8_t  VenomU8;
typedef uint16_t VenomU16;
typedef uint32_t VenomU32;
typedef uint64_t VenomU64;
typedef float    VenomF32;
typedef double   VenomF64;
typedef size_t   VenomSize;
typedef VenomU8  VenomBool;

#define VENOM_TRUE  1
#define VENOM_FALSE 0
```

### Geometry

```c
typedef struct VenomColor { VenomU8 r, g, b, a; } VenomColor;
typedef struct VenomRectF { VenomF32 x, y, width, height; } VenomRectF;
typedef struct VenomSize2D { VenomF32 width, height; } VenomSize2D;
typedef struct VenomInsets { VenomF32 top, right, bottom, left; } VenomInsets;
```

### Result

```c
typedef struct VenomResult { VenomBool ok; VenomErrorCode error; } VenomResult;
typedef struct VenomResultPtr { VenomBool ok; void* value; VenomErrorCode error; } VenomResultPtr;

#define VENOM_OK_UNIT() ((VenomResult){ .ok = VENOM_TRUE })
#define VENOM_OK_PTR(val) ((VenomResultPtr){ .ok = VENOM_TRUE, .value = (val) })
#define VENOM_ERR_UNIT(err) ((VenomResult){ .ok = VENOM_FALSE, .error = (err) })
#define VENOM_ERR_PTR(err) ((VenomResultPtr){ .ok = VENOM_FALSE, .error = (err) })
```
