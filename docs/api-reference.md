# API Reference

Complete API reference for VAXPUI.

---

## Core

### Initialization

```c
VaxpResult vaxp_init(void);
void vaxp_shutdown(void);
const char* vaxp_version_string(void);
```

### Memory

```c
void* vaxp_alloc(VaxpSize size);
void* vaxp_realloc(void* ptr, VaxpSize old_size, VaxpSize new_size);
void vaxp_free(void* ptr, VaxpSize size);

// Debug
void vaxp_memory_dump_stats(void);
```

### Reference Counting

```c
void vaxp_ref(void* obj);
void vaxp_unref(void* obj);
VaxpU32 vaxp_ref_count(void* obj);

// Macro for creating ref-counted objects
VAXP_REF_NEW(Type, destructor)
```

---

## App

```c
// Macro for running application
VAXP_APP(
    .title = "Title",
    .width = 800,
    .height = 600,
    .build = build_function
)

// Build function signature
VaxpWidget* (*VaxpBuildFunc)(void* data);
```

---

## Widgets

### VaxpWidget (Base)

```c
VaxpResultPtr vaxp_widget_create(const VaxpWidgetClass* cls);
void vaxp_widget_destroy(VaxpWidget* widget);
void vaxp_widget_invalidate(VaxpWidget* widget);
void vaxp_widget_invalidate_layout(VaxpWidget* widget);
void vaxp_widget_measure(VaxpWidget* widget, ...);
void vaxp_widget_layout(VaxpWidget* widget, VaxpRectF bounds);
void vaxp_widget_draw(VaxpWidget* widget, VaxpCanvas* canvas);
```

### VaxpButton

```c
VaxpResultPtr vaxp_button_create(void);
void vaxp_button_set_text(VaxpButton* btn, const char* text);
void vaxp_button_set_on_click(VaxpButton* btn, VaxpButtonCallback cb, void* data);

// Callback signature
typedef void (*VaxpButtonCallback)(VaxpButton* btn, void* user_data);

// Macro
vaxp_btn(text, .on_click = handler)
```

### VaxpLabel

```c
VaxpResultPtr vaxp_label_create(void);
void vaxp_label_set_text(VaxpLabel* label, const char* text);

// Macro
vaxp_text("Text content")
```

### VaxpTextInput

```c
VaxpResultPtr vaxp_text_input_create(void);
void vaxp_text_input_set_text(VaxpTextInput* input, const char* text);
const char* vaxp_text_input_get_text(const VaxpTextInput* input);
void vaxp_text_input_set_placeholder(VaxpTextInput* input, const char* placeholder);
void vaxp_text_input_set_password_mode(VaxpTextInput* input, VaxpBool password);
void vaxp_text_input_set_max_length(VaxpTextInput* input, VaxpU32 max);
void vaxp_text_input_set_on_change(VaxpTextInput* input, VaxpTextInputCallback cb, void* data);
void vaxp_text_input_set_on_submit(VaxpTextInput* input, VaxpTextInputCallback cb, void* data);

// Callback signature
typedef void (*VaxpTextInputCallback)(VaxpTextInput* input, const char* text, void* data);

// Macro
vaxp_input(.placeholder = "Enter text", .on_change = handler)
```

### VaxpContainer

```c
VaxpResultPtr vaxp_container_create(VaxpLayoutType type);
void vaxp_container_add_child(VaxpContainer* container, VaxpWidget* child);
void vaxp_container_remove_child(VaxpContainer* container, VaxpWidget* child);
void vaxp_container_clear(VaxpContainer* container);

// Macros
vaxp_col(.gap = 10, .children = VAXP_CHILDREN(...))
vaxp_row(.gap = 10, .children = VAXP_CHILDREN(...))
vaxp_center(.children = VAXP_CHILDREN(...))
```

### VaxpScrollable

```c
VaxpResultPtr vaxp_scrollable_create(void);
VaxpResult vaxp_scrollable_set_content(VaxpScrollable* scroll, VaxpWidget* content);
void vaxp_scrollable_get_scroll(const VaxpScrollable* scroll, VaxpF32* x, VaxpF32* y);
void vaxp_scrollable_set_scroll(VaxpScrollable* scroll, VaxpF32 x, VaxpF32 y);
void vaxp_scrollable_scroll_by(VaxpScrollable* scroll, VaxpF32 dx, VaxpF32 dy);
void vaxp_scrollable_ensure_visible(VaxpScrollable* scroll, VaxpWidget* widget);
void vaxp_scrollable_set_direction(VaxpScrollable* scroll, VaxpScrollDirection dir);

// Macro
vaxp_scroll(.direction = VAXP_SCROLL_VERTICAL, .content = child)
```

### VaxpImageWidget

```c
VaxpResultPtr vaxp_image_load_file(const char* path);
VaxpResultPtr vaxp_image_widget_create(void);
VaxpResult vaxp_image_widget_set_image(VaxpImageWidget* widget, VaxpImageData* image);
VaxpResult vaxp_image_widget_load(VaxpImageWidget* widget, const char* path);
void vaxp_image_widget_set_fit(VaxpImageWidget* widget, VaxpImageFit fit);
void vaxp_image_widget_set_corner_radius(VaxpImageWidget* widget, VaxpF32 radius);

// Macro
vaxp_image(.src = "path.png", .fit = VAXP_IMAGE_FIT_CONTAIN)
```

---

## Focus

```c
void vaxp_focus_init(void);
void vaxp_focus_set_root(VaxpWidget* root);
void vaxp_focus_set(VaxpWidget* widget);
VaxpWidget* vaxp_focus_get(void);
VaxpBool vaxp_focus_has(VaxpWidget* widget);
void vaxp_focus_next(void);
void vaxp_focus_prev(void);
```

---

## Theme

```c
const VaxpTheme* vaxp_theme_get_current(void);
void vaxp_theme_set(const VaxpTheme* theme);
const VaxpTheme* vaxp_theme_light(void);
const VaxpTheme* vaxp_theme_dark(void);
VaxpTheme vaxp_theme_create(VaxpColor primary, VaxpColor secondary, VaxpBool is_dark);
```

---

## State Management

### Cubit

```c
VaxpCubit* vaxp_cubit_create(VaxpSize state_size, void* initial_state);
void vaxp_cubit_destroy(VaxpCubit* cubit);
void vaxp_cubit_emit(VaxpCubit* cubit, void* new_state);
void vaxp_cubit_add_listener(VaxpCubit* cubit, VaxpCubitListener listener, void* data);
void vaxp_cubit_remove_listener(VaxpCubit* cubit, VaxpCubitListener listener);

// Listener signature
typedef void (*VaxpCubitListener)(VaxpCubit* cubit, void* state, void* user_data);

// Macros
VAXP_DEFINE_CUBIT(Name, StateType, actions...)
VAXP_CUBIT_CREATE(Name, StateType, initial_state)
VAXP_CUBIT_EMIT(cubit, Name, action)
```

### BlocBuilder

```c
VAXP_BLOC_BUILDER(
    .cubit = my_cubit,
    .builder = builder_function,
    .user_data = optional_data
)

// Builder signature
VaxpWidget* (*builder)(void* state, void* user_data);
```

---

## Graphics

### Canvas

```c
void vaxp_canvas_save(VaxpCanvas* canvas);
void vaxp_canvas_restore(VaxpCanvas* canvas);
void vaxp_canvas_translate(VaxpCanvas* canvas, VaxpF32 dx, VaxpF32 dy);
void vaxp_canvas_scale(VaxpCanvas* canvas, VaxpF32 sx, VaxpF32 sy);
void vaxp_canvas_rotate(VaxpCanvas* canvas, VaxpF32 degrees);
void vaxp_canvas_clip_rect(VaxpCanvas* canvas, VaxpRectF rect);
void vaxp_canvas_clip_rounded_rect(VaxpCanvas* canvas, VaxpRectF rect, VaxpF32 radius);
void vaxp_canvas_clear(VaxpCanvas* canvas, VaxpColor color);
void vaxp_canvas_draw_rect(VaxpCanvas* canvas, VaxpRectF rect, const VaxpPaint* paint);
void vaxp_canvas_draw_rounded_rect(VaxpCanvas* canvas, VaxpRectF rect, VaxpF32 radius, const VaxpPaint* paint);
void vaxp_canvas_draw_circle(VaxpCanvas* canvas, VaxpF32 cx, VaxpF32 cy, VaxpF32 r, const VaxpPaint* paint);
void vaxp_canvas_draw_text(VaxpCanvas* canvas, const char* text, VaxpF32 x, VaxpF32 y, ...);
void vaxp_canvas_draw_image(VaxpCanvas* canvas, const VaxpImage* image, VaxpF32 x, VaxpF32 y);
void vaxp_canvas_flush(VaxpCanvas* canvas);
```

---

## Types

### Basic Types

```c
typedef int8_t   VaxpI8;
typedef int16_t  VaxpI16;
typedef int32_t  VaxpI32;
typedef int64_t  VaxpI64;
typedef uint8_t  VaxpU8;
typedef uint16_t VaxpU16;
typedef uint32_t VaxpU32;
typedef uint64_t VaxpU64;
typedef float    VaxpF32;
typedef double   VaxpF64;
typedef size_t   VaxpSize;
typedef VaxpU8  VaxpBool;

#define VAXP_TRUE  1
#define VAXP_FALSE 0
```

### Geometry

```c
typedef struct VaxpColor { VaxpU8 r, g, b, a; } VaxpColor;
typedef struct VaxpRectF { VaxpF32 x, y, width, height; } VaxpRectF;
typedef struct VaxpSize2D { VaxpF32 width, height; } VaxpSize2D;
typedef struct VaxpInsets { VaxpF32 top, right, bottom, left; } VaxpInsets;
```

### Result

```c
typedef struct VaxpResult { VaxpBool ok; VaxpErrorCode error; } VaxpResult;
typedef struct VaxpResultPtr { VaxpBool ok; void* value; VaxpErrorCode error; } VaxpResultPtr;

#define VAXP_OK_UNIT() ((VaxpResult){ .ok = VAXP_TRUE })
#define VAXP_OK_PTR(val) ((VaxpResultPtr){ .ok = VAXP_TRUE, .value = (val) })
#define VAXP_ERR_UNIT(err) ((VaxpResult){ .ok = VAXP_FALSE, .error = (err) })
#define VAXP_ERR_PTR(err) ((VaxpResultPtr){ .ok = VAXP_FALSE, .error = (err) })
```
