/*
 * VAXPUI - ColorPicker widget implementation
 */

#include "vaxp/widgets/vaxp_color_picker.h"
#include "vaxp/core/vaxp_memory.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

#define PICKER_SIZE 200.0f
#define HUE_WIDTH 20.0f
#define ALPHA_HEIGHT 20.0f
#define PREVIEW_SIZE 40.0f

/* HSV to RGB conversion */
static VaxpColor hsv_to_rgb(VaxpF32 h, VaxpF32 s, VaxpF32 v, VaxpF32 a) {
    VaxpF32 c = v * s;
    VaxpF32 x = c * (1 - fabsf(fmodf(h / 60.0f, 2) - 1));
    VaxpF32 m = v - c;
    VaxpF32 r, g, b;
    
    if (h < 60) { r = c; g = x; b = 0; }
    else if (h < 120) { r = x; g = c; b = 0; }
    else if (h < 180) { r = 0; g = c; b = x; }
    else if (h < 240) { r = 0; g = x; b = c; }
    else if (h < 300) { r = x; g = 0; b = c; }
    else { r = c; g = 0; b = x; }
    
    return (VaxpColor){
        (VaxpU8)((r + m) * 255),
        (VaxpU8)((g + m) * 255),
        (VaxpU8)((b + m) * 255),
        (VaxpU8)(a * 255)
    };
}

/* RGB to HSV conversion */
static void rgb_to_hsv(VaxpColor c, VaxpF32* h, VaxpF32* s, VaxpF32* v) {
    VaxpF32 r = c.r / 255.0f, g = c.g / 255.0f, b = c.b / 255.0f;
    VaxpF32 max = fmaxf(fmaxf(r, g), b);
    VaxpF32 min = fminf(fminf(r, g), b);
    VaxpF32 d = max - min;
    
    *v = max;
    *s = (max == 0) ? 0 : d / max;
    
    if (d == 0) { *h = 0; }
    else if (max == r) { *h = 60 * fmodf((g - b) / d, 6); }
    else if (max == g) { *h = 60 * ((b - r) / d + 2); }
    else { *h = 60 * ((r - g) / d + 4); }
    
    if (*h < 0) *h += 360;
}

static void color_picker_init(VaxpWidget* widget) {
    VaxpColorPicker* picker = (VaxpColorPicker*)widget;
    
    picker->selected_color = (VaxpColor){ 255, 0, 0, 255 };
    picker->mode = VAXP_COLOR_PICKER_HSV;
    picker->hue = 0;
    picker->saturation = 1.0f;
    picker->value = 1.0f;
    picker->alpha = 1.0f;
    
    picker->show_alpha = VAXP_TRUE;
    picker->show_preview = VAXP_TRUE;
    picker->show_hex_input = VAXP_TRUE;
    picker->show_palette = VAXP_FALSE;
    picker->palette = NULL;
    picker->palette_count = 0;
    
    picker->dragging_hue = VAXP_FALSE;
    picker->dragging_sv = VAXP_FALSE;
    picker->dragging_alpha = VAXP_FALSE;
    
    picker->on_change = NULL;
    picker->callback_data = NULL;
    
    widget->focusable = VAXP_TRUE;
}

static void color_picker_destroy(VaxpWidget* widget) {
    VaxpColorPicker* picker = (VaxpColorPicker*)widget;
    if (picker->palette) vaxp_free(picker->palette, picker->palette_count * sizeof(VaxpColor));
    vaxp_widget_class.destroy(widget);
}

static void color_picker_measure(VaxpWidget* widget, VaxpF32 aw, VaxpF32 ah, VaxpF32* w, VaxpF32* h) {
    VaxpColorPicker* picker = (VaxpColorPicker*)widget;
    (void)aw; (void)ah;
    *w = PICKER_SIZE + HUE_WIDTH + 16;
    *h = PICKER_SIZE + (picker->show_alpha ? ALPHA_HEIGHT + 8 : 0) + 50;
}

static void update_color(VaxpColorPicker* picker) {
    picker->selected_color = hsv_to_rgb(picker->hue, picker->saturation, picker->value, picker->alpha);
    if (picker->on_change) picker->on_change(picker, picker->selected_color, picker->callback_data);
}

static void color_picker_draw(VaxpWidget* widget, VaxpCanvas* canvas) {
    VaxpColorPicker* picker = (VaxpColorPicker*)widget;
    
    /* Saturation/Value square */
    VaxpF32 sq = PICKER_SIZE;
    for (int y = 0; y < (int)sq; y += 4) {
        for (int x = 0; x < (int)sq; x += 4) {
            VaxpF32 s = (VaxpF32)x / sq;
            VaxpF32 v = 1.0f - (VaxpF32)y / sq;
            VaxpColor c = hsv_to_rgb(picker->hue, s, v, 1.0f);
            VaxpPaint p = vaxp_paint_fill(c);
            VaxpRectF r = { (VaxpF32)x, (VaxpF32)y, 4, 4 };
            vaxp_canvas_draw_rect(canvas, r, &p);
        }
    }
    
    /* S/V cursor */
    VaxpF32 cx = picker->saturation * sq;
    VaxpF32 cy = (1.0f - picker->value) * sq;
    VaxpPaint wp = vaxp_paint_stroke((VaxpColor){255,255,255,255}, 2.0f);
    VaxpPaint bp = vaxp_paint_stroke((VaxpColor){0,0,0,255}, 1.0f);
    vaxp_canvas_draw_circle(canvas, cx, cy, 6, &wp);
    vaxp_canvas_draw_circle(canvas, cx, cy, 7, &bp);
    
    /* Hue bar */
    VaxpF32 hx = sq + 12;
    for (int y = 0; y < (int)sq; y += 2) {
        VaxpF32 h = (VaxpF32)y / sq * 360.0f;
        VaxpColor c = hsv_to_rgb(h, 1.0f, 1.0f, 1.0f);
        VaxpPaint p = vaxp_paint_fill(c);
        VaxpRectF r = { hx, (VaxpF32)y, HUE_WIDTH, 2 };
        vaxp_canvas_draw_rect(canvas, r, &p);
    }
    
    /* Hue cursor */
    VaxpF32 hy = picker->hue / 360.0f * sq;
    VaxpRectF hc = { hx - 2, hy - 2, HUE_WIDTH + 4, 4 };
    vaxp_canvas_draw_rect(canvas, hc, &wp);
    
    /* Alpha bar */
    if (picker->show_alpha) {
        VaxpF32 ay = sq + 8;
        /* Checkerboard */
        for (int x = 0; x < (int)sq; x += 8) {
            VaxpColor c = ((x / 8) % 2 == 0) ? (VaxpColor){200,200,200,255} : (VaxpColor){255,255,255,255};
            VaxpPaint p = vaxp_paint_fill(c);
            VaxpRectF r = { (VaxpF32)x, ay, 8, ALPHA_HEIGHT };
            vaxp_canvas_draw_rect(canvas, r, &p);
        }
        /* Alpha gradient */
        for (int x = 0; x < (int)sq; x += 2) {
            VaxpColor c = picker->selected_color;
            c.a = (VaxpU8)((VaxpF32)x / sq * 255);
            VaxpPaint p = vaxp_paint_fill(c);
            VaxpRectF r = { (VaxpF32)x, ay, 2, ALPHA_HEIGHT };
            vaxp_canvas_draw_rect(canvas, r, &p);
        }
        /* Alpha cursor */
        VaxpF32 ax = picker->alpha * sq;
        VaxpRectF ac = { ax - 2, ay - 2, 4, ALPHA_HEIGHT + 4 };
        vaxp_canvas_draw_rect(canvas, ac, &bp);
    }
    
    /* Preview */
    VaxpF32 py = sq + (picker->show_alpha ? ALPHA_HEIGHT + 16 : 8);
    VaxpRectF prev = { 0, py, PREVIEW_SIZE, PREVIEW_SIZE };
    VaxpPaint pp = vaxp_paint_fill(picker->selected_color);
    vaxp_canvas_draw_rect(canvas, prev, &pp);
    VaxpPaint pbp = vaxp_paint_stroke((VaxpColor){0,0,0,64}, 1.0f);
    vaxp_canvas_draw_rect(canvas, prev, &pbp);
    
    /* Hex value */
    char hex[16];
    snprintf(hex, sizeof(hex), "#%02X%02X%02X", 
             picker->selected_color.r, picker->selected_color.g, picker->selected_color.b);
    VaxpPaint tp = vaxp_paint_fill((VaxpColor){33,33,33,255});
    vaxp_canvas_draw_text(canvas, hex, PREVIEW_SIZE + 8, py + PREVIEW_SIZE / 2 + 5, NULL, &tp);
}

static VaxpBool color_picker_on_event(VaxpWidget* widget, const VaxpEvent* event) {
    VaxpColorPicker* picker = (VaxpColorPicker*)widget;
    VaxpF32 sq = PICKER_SIZE;
    
    if (event->type == VAXP_EVENT_MOUSE_BUTTON_DOWN && event->mouse.button == VAXP_MOUSE_BUTTON_LEFT) {
        VaxpF32 mx = (VaxpF32)event->mouse.x;
        VaxpF32 my = (VaxpF32)event->mouse.y;
        
        /* S/V square */
        if (mx >= 0 && mx < sq && my >= 0 && my < sq) {
            picker->dragging_sv = VAXP_TRUE;
            picker->saturation = mx / sq;
            picker->value = 1.0f - my / sq;
            update_color(picker);
            widget->needs_redraw = VAXP_TRUE;
            return VAXP_TRUE;
        }
        
        /* Hue bar */
        if (mx >= sq + 12 && mx < sq + 12 + HUE_WIDTH && my >= 0 && my < sq) {
            picker->dragging_hue = VAXP_TRUE;
            picker->hue = my / sq * 360.0f;
            update_color(picker);
            widget->needs_redraw = VAXP_TRUE;
            return VAXP_TRUE;
        }
        
        /* Alpha bar */
        if (picker->show_alpha && my >= sq + 8 && my < sq + 8 + ALPHA_HEIGHT && mx >= 0 && mx < sq) {
            picker->dragging_alpha = VAXP_TRUE;
            picker->alpha = mx / sq;
            update_color(picker);
            widget->needs_redraw = VAXP_TRUE;
            return VAXP_TRUE;
        }
    }
    
    if (event->type == VAXP_EVENT_MOUSE_MOVE) {
        VaxpF32 mx = (VaxpF32)event->mouse.x;
        VaxpF32 my = (VaxpF32)event->mouse.y;
        
        if (picker->dragging_sv) {
            picker->saturation = fminf(fmaxf(mx / sq, 0), 1);
            picker->value = 1.0f - fminf(fmaxf(my / sq, 0), 1);
            update_color(picker);
            widget->needs_redraw = VAXP_TRUE;
            return VAXP_TRUE;
        }
        
        if (picker->dragging_hue) {
            picker->hue = fminf(fmaxf(my / sq * 360.0f, 0), 360);
            update_color(picker);
            widget->needs_redraw = VAXP_TRUE;
            return VAXP_TRUE;
        }
        
        if (picker->dragging_alpha) {
            picker->alpha = fminf(fmaxf(mx / sq, 0), 1);
            update_color(picker);
            widget->needs_redraw = VAXP_TRUE;
            return VAXP_TRUE;
        }
    }
    
    if (event->type == VAXP_EVENT_MOUSE_BUTTON_UP) {
        picker->dragging_sv = picker->dragging_hue = picker->dragging_alpha = VAXP_FALSE;
    }
    
    return VAXP_FALSE;
}

const VaxpWidgetClass vaxp_color_picker_class = {
    .class_name = "VaxpColorPicker",
    .instance_size = sizeof(VaxpColorPicker),
    .parent_class = &vaxp_widget_class,
    .init = color_picker_init,
    .destroy = color_picker_destroy,
    .measure = color_picker_measure,
    .draw = color_picker_draw,
    .on_event = color_picker_on_event,
};

VaxpResultPtr vaxp_color_picker_create(void) { return vaxp_widget_create(&vaxp_color_picker_class); }

void vaxp_color_picker_set_color(VaxpColorPicker* picker, VaxpColor color) {
    if (picker) {
        picker->selected_color = color;
        rgb_to_hsv(color, &picker->hue, &picker->saturation, &picker->value);
        picker->alpha = color.a / 255.0f;
        vaxp_widget_invalidate((VaxpWidget*)picker);
    }
}

VaxpColor vaxp_color_picker_get_color(const VaxpColorPicker* picker) {
    return picker ? picker->selected_color : (VaxpColor){0,0,0,0};
}

void vaxp_color_picker_set_mode(VaxpColorPicker* picker, VaxpColorPickerMode mode) {
    if (picker) { picker->mode = mode; vaxp_widget_invalidate((VaxpWidget*)picker); }
}

void vaxp_color_picker_set_show_alpha(VaxpColorPicker* picker, VaxpBool show) {
    if (picker) { picker->show_alpha = show; vaxp_widget_invalidate((VaxpWidget*)picker); }
}

void vaxp_color_picker_set_on_change(VaxpColorPicker* picker, VaxpColorPickerCallback cb, void* data) {
    if (picker) { picker->on_change = cb; picker->callback_data = data; }
}

VaxpWidget* _vaxp_color_picker_build(const VaxpColorPickerConfig* config) {
    VaxpResultPtr r = vaxp_color_picker_create();
    if (!r.ok) return NULL;
    VaxpColorPicker* p = (VaxpColorPicker*)r.value;
    if (config->initial_color.a > 0) vaxp_color_picker_set_color(p, config->initial_color);
    p->mode = config->mode;
    p->show_alpha = config->show_alpha;
    p->on_change = config->on_change;
    p->callback_data = config->data;
    return (VaxpWidget*)p;
}
