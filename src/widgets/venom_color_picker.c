/*
 * VENOMUI - ColorPicker widget implementation
 */

#include "venom/widgets/venom_color_picker.h"
#include "venom/core/venom_memory.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

#define PICKER_SIZE 200.0f
#define HUE_WIDTH 20.0f
#define ALPHA_HEIGHT 20.0f
#define PREVIEW_SIZE 40.0f

/* HSV to RGB conversion */
static VenomColor hsv_to_rgb(VenomF32 h, VenomF32 s, VenomF32 v, VenomF32 a) {
    VenomF32 c = v * s;
    VenomF32 x = c * (1 - fabsf(fmodf(h / 60.0f, 2) - 1));
    VenomF32 m = v - c;
    VenomF32 r, g, b;
    
    if (h < 60) { r = c; g = x; b = 0; }
    else if (h < 120) { r = x; g = c; b = 0; }
    else if (h < 180) { r = 0; g = c; b = x; }
    else if (h < 240) { r = 0; g = x; b = c; }
    else if (h < 300) { r = x; g = 0; b = c; }
    else { r = c; g = 0; b = x; }
    
    return (VenomColor){
        (VenomU8)((r + m) * 255),
        (VenomU8)((g + m) * 255),
        (VenomU8)((b + m) * 255),
        (VenomU8)(a * 255)
    };
}

/* RGB to HSV conversion */
static void rgb_to_hsv(VenomColor c, VenomF32* h, VenomF32* s, VenomF32* v) {
    VenomF32 r = c.r / 255.0f, g = c.g / 255.0f, b = c.b / 255.0f;
    VenomF32 max = fmaxf(fmaxf(r, g), b);
    VenomF32 min = fminf(fminf(r, g), b);
    VenomF32 d = max - min;
    
    *v = max;
    *s = (max == 0) ? 0 : d / max;
    
    if (d == 0) { *h = 0; }
    else if (max == r) { *h = 60 * fmodf((g - b) / d, 6); }
    else if (max == g) { *h = 60 * ((b - r) / d + 2); }
    else { *h = 60 * ((r - g) / d + 4); }
    
    if (*h < 0) *h += 360;
}

static void color_picker_init(VenomWidget* widget) {
    VenomColorPicker* picker = (VenomColorPicker*)widget;
    
    picker->selected_color = (VenomColor){ 255, 0, 0, 255 };
    picker->mode = VENOM_COLOR_PICKER_HSV;
    picker->hue = 0;
    picker->saturation = 1.0f;
    picker->value = 1.0f;
    picker->alpha = 1.0f;
    
    picker->show_alpha = VENOM_TRUE;
    picker->show_preview = VENOM_TRUE;
    picker->show_hex_input = VENOM_TRUE;
    picker->show_palette = VENOM_FALSE;
    picker->palette = NULL;
    picker->palette_count = 0;
    
    picker->dragging_hue = VENOM_FALSE;
    picker->dragging_sv = VENOM_FALSE;
    picker->dragging_alpha = VENOM_FALSE;
    
    picker->on_change = NULL;
    picker->callback_data = NULL;
    
    widget->focusable = VENOM_TRUE;
}

static void color_picker_destroy(VenomWidget* widget) {
    VenomColorPicker* picker = (VenomColorPicker*)widget;
    if (picker->palette) venom_free(picker->palette, picker->palette_count * sizeof(VenomColor));
    venom_widget_class.destroy(widget);
}

static void color_picker_measure(VenomWidget* widget, VenomF32 aw, VenomF32 ah, VenomF32* w, VenomF32* h) {
    VenomColorPicker* picker = (VenomColorPicker*)widget;
    (void)aw; (void)ah;
    *w = PICKER_SIZE + HUE_WIDTH + 16;
    *h = PICKER_SIZE + (picker->show_alpha ? ALPHA_HEIGHT + 8 : 0) + 50;
}

static void update_color(VenomColorPicker* picker) {
    picker->selected_color = hsv_to_rgb(picker->hue, picker->saturation, picker->value, picker->alpha);
    if (picker->on_change) picker->on_change(picker, picker->selected_color, picker->callback_data);
}

static void color_picker_draw(VenomWidget* widget, VenomCanvas* canvas) {
    VenomColorPicker* picker = (VenomColorPicker*)widget;
    
    /* Saturation/Value square */
    VenomF32 sq = PICKER_SIZE;
    for (int y = 0; y < (int)sq; y += 4) {
        for (int x = 0; x < (int)sq; x += 4) {
            VenomF32 s = (VenomF32)x / sq;
            VenomF32 v = 1.0f - (VenomF32)y / sq;
            VenomColor c = hsv_to_rgb(picker->hue, s, v, 1.0f);
            VenomPaint p = venom_paint_fill(c);
            VenomRectF r = { (VenomF32)x, (VenomF32)y, 4, 4 };
            venom_canvas_draw_rect(canvas, r, &p);
        }
    }
    
    /* S/V cursor */
    VenomF32 cx = picker->saturation * sq;
    VenomF32 cy = (1.0f - picker->value) * sq;
    VenomPaint wp = venom_paint_stroke((VenomColor){255,255,255,255}, 2.0f);
    VenomPaint bp = venom_paint_stroke((VenomColor){0,0,0,255}, 1.0f);
    venom_canvas_draw_circle(canvas, cx, cy, 6, &wp);
    venom_canvas_draw_circle(canvas, cx, cy, 7, &bp);
    
    /* Hue bar */
    VenomF32 hx = sq + 12;
    for (int y = 0; y < (int)sq; y += 2) {
        VenomF32 h = (VenomF32)y / sq * 360.0f;
        VenomColor c = hsv_to_rgb(h, 1.0f, 1.0f, 1.0f);
        VenomPaint p = venom_paint_fill(c);
        VenomRectF r = { hx, (VenomF32)y, HUE_WIDTH, 2 };
        venom_canvas_draw_rect(canvas, r, &p);
    }
    
    /* Hue cursor */
    VenomF32 hy = picker->hue / 360.0f * sq;
    VenomRectF hc = { hx - 2, hy - 2, HUE_WIDTH + 4, 4 };
    venom_canvas_draw_rect(canvas, hc, &wp);
    
    /* Alpha bar */
    if (picker->show_alpha) {
        VenomF32 ay = sq + 8;
        /* Checkerboard */
        for (int x = 0; x < (int)sq; x += 8) {
            VenomColor c = ((x / 8) % 2 == 0) ? (VenomColor){200,200,200,255} : (VenomColor){255,255,255,255};
            VenomPaint p = venom_paint_fill(c);
            VenomRectF r = { (VenomF32)x, ay, 8, ALPHA_HEIGHT };
            venom_canvas_draw_rect(canvas, r, &p);
        }
        /* Alpha gradient */
        for (int x = 0; x < (int)sq; x += 2) {
            VenomColor c = picker->selected_color;
            c.a = (VenomU8)((VenomF32)x / sq * 255);
            VenomPaint p = venom_paint_fill(c);
            VenomRectF r = { (VenomF32)x, ay, 2, ALPHA_HEIGHT };
            venom_canvas_draw_rect(canvas, r, &p);
        }
        /* Alpha cursor */
        VenomF32 ax = picker->alpha * sq;
        VenomRectF ac = { ax - 2, ay - 2, 4, ALPHA_HEIGHT + 4 };
        venom_canvas_draw_rect(canvas, ac, &bp);
    }
    
    /* Preview */
    VenomF32 py = sq + (picker->show_alpha ? ALPHA_HEIGHT + 16 : 8);
    VenomRectF prev = { 0, py, PREVIEW_SIZE, PREVIEW_SIZE };
    VenomPaint pp = venom_paint_fill(picker->selected_color);
    venom_canvas_draw_rect(canvas, prev, &pp);
    VenomPaint pbp = venom_paint_stroke((VenomColor){0,0,0,64}, 1.0f);
    venom_canvas_draw_rect(canvas, prev, &pbp);
    
    /* Hex value */
    char hex[16];
    snprintf(hex, sizeof(hex), "#%02X%02X%02X", 
             picker->selected_color.r, picker->selected_color.g, picker->selected_color.b);
    VenomPaint tp = venom_paint_fill((VenomColor){33,33,33,255});
    venom_canvas_draw_text(canvas, hex, PREVIEW_SIZE + 8, py + PREVIEW_SIZE / 2 + 5, NULL, &tp);
}

static VenomBool color_picker_on_event(VenomWidget* widget, const VenomEvent* event) {
    VenomColorPicker* picker = (VenomColorPicker*)widget;
    VenomF32 sq = PICKER_SIZE;
    
    if (event->type == VENOM_EVENT_MOUSE_BUTTON_DOWN && event->mouse.button == VENOM_MOUSE_BUTTON_LEFT) {
        VenomF32 mx = (VenomF32)event->mouse.x;
        VenomF32 my = (VenomF32)event->mouse.y;
        
        /* S/V square */
        if (mx >= 0 && mx < sq && my >= 0 && my < sq) {
            picker->dragging_sv = VENOM_TRUE;
            picker->saturation = mx / sq;
            picker->value = 1.0f - my / sq;
            update_color(picker);
            widget->needs_redraw = VENOM_TRUE;
            return VENOM_TRUE;
        }
        
        /* Hue bar */
        if (mx >= sq + 12 && mx < sq + 12 + HUE_WIDTH && my >= 0 && my < sq) {
            picker->dragging_hue = VENOM_TRUE;
            picker->hue = my / sq * 360.0f;
            update_color(picker);
            widget->needs_redraw = VENOM_TRUE;
            return VENOM_TRUE;
        }
        
        /* Alpha bar */
        if (picker->show_alpha && my >= sq + 8 && my < sq + 8 + ALPHA_HEIGHT && mx >= 0 && mx < sq) {
            picker->dragging_alpha = VENOM_TRUE;
            picker->alpha = mx / sq;
            update_color(picker);
            widget->needs_redraw = VENOM_TRUE;
            return VENOM_TRUE;
        }
    }
    
    if (event->type == VENOM_EVENT_MOUSE_MOVE) {
        VenomF32 mx = (VenomF32)event->mouse.x;
        VenomF32 my = (VenomF32)event->mouse.y;
        
        if (picker->dragging_sv) {
            picker->saturation = fminf(fmaxf(mx / sq, 0), 1);
            picker->value = 1.0f - fminf(fmaxf(my / sq, 0), 1);
            update_color(picker);
            widget->needs_redraw = VENOM_TRUE;
            return VENOM_TRUE;
        }
        
        if (picker->dragging_hue) {
            picker->hue = fminf(fmaxf(my / sq * 360.0f, 0), 360);
            update_color(picker);
            widget->needs_redraw = VENOM_TRUE;
            return VENOM_TRUE;
        }
        
        if (picker->dragging_alpha) {
            picker->alpha = fminf(fmaxf(mx / sq, 0), 1);
            update_color(picker);
            widget->needs_redraw = VENOM_TRUE;
            return VENOM_TRUE;
        }
    }
    
    if (event->type == VENOM_EVENT_MOUSE_BUTTON_UP) {
        picker->dragging_sv = picker->dragging_hue = picker->dragging_alpha = VENOM_FALSE;
    }
    
    return VENOM_FALSE;
}

const VenomWidgetClass venom_color_picker_class = {
    .class_name = "VenomColorPicker",
    .instance_size = sizeof(VenomColorPicker),
    .parent_class = &venom_widget_class,
    .init = color_picker_init,
    .destroy = color_picker_destroy,
    .measure = color_picker_measure,
    .draw = color_picker_draw,
    .on_event = color_picker_on_event,
};

VenomResultPtr venom_color_picker_create(void) { return venom_widget_create(&venom_color_picker_class); }

void venom_color_picker_set_color(VenomColorPicker* picker, VenomColor color) {
    if (picker) {
        picker->selected_color = color;
        rgb_to_hsv(color, &picker->hue, &picker->saturation, &picker->value);
        picker->alpha = color.a / 255.0f;
        venom_widget_invalidate((VenomWidget*)picker);
    }
}

VenomColor venom_color_picker_get_color(const VenomColorPicker* picker) {
    return picker ? picker->selected_color : (VenomColor){0,0,0,0};
}

void venom_color_picker_set_mode(VenomColorPicker* picker, VenomColorPickerMode mode) {
    if (picker) { picker->mode = mode; venom_widget_invalidate((VenomWidget*)picker); }
}

void venom_color_picker_set_show_alpha(VenomColorPicker* picker, VenomBool show) {
    if (picker) { picker->show_alpha = show; venom_widget_invalidate((VenomWidget*)picker); }
}

void venom_color_picker_set_on_change(VenomColorPicker* picker, VenomColorPickerCallback cb, void* data) {
    if (picker) { picker->on_change = cb; picker->callback_data = data; }
}

VenomWidget* _venom_color_picker_build(const VenomColorPickerConfig* config) {
    VenomResultPtr r = venom_color_picker_create();
    if (!r.ok) return NULL;
    VenomColorPicker* p = (VenomColorPicker*)r.value;
    if (config->initial_color.a > 0) venom_color_picker_set_color(p, config->initial_color);
    p->mode = config->mode;
    p->show_alpha = config->show_alpha;
    p->on_change = config->on_change;
    p->callback_data = config->data;
    return (VenomWidget*)p;
}
