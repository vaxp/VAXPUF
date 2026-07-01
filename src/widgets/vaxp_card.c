/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_card.c - Card widget implementation
 */

#include "vaxp/widgets/vaxp_card.h"
#include "vaxp/core/vaxp_memory.h"

static void card_init(VaxpWidget* widget) {
    VaxpCard* card = (VaxpCard*)widget;
    
    card->child = NULL;
    card->elevation = 2.0f;
    
    card->background_color = (VaxpColor){ 255, 255, 255, 255 };
    card->shadow_color = (VaxpColor){ 0, 0, 0, 40 };
    card->corner_radius = 8.0f;
    card->padding = 16.0f;
    card->outlined = VAXP_FALSE;
    card->border_color = (VaxpColor){ 224, 224, 224, 255 };
}

static void card_destroy(VaxpWidget* widget) {
    VaxpCard* card = (VaxpCard*)widget;
    
    if (card->child) {
        card->child->parent = NULL;
        vaxp_unref(card->child);
        card->child = NULL;
    }
    
    vaxp_widget_class.destroy(widget);
}

static void card_measure(VaxpWidget* widget, VaxpF32 available_width, VaxpF32 available_height,
                         VaxpF32* out_width, VaxpF32* out_height) {
    VaxpCard* card = (VaxpCard*)widget;
    
    VaxpF32 content_available_w = available_width - card->padding * 2;
    VaxpF32 content_available_h = available_height - card->padding * 2;
    
    if (card->child) {
        VaxpF32 child_w, child_h;
        vaxp_widget_measure(card->child, content_available_w, content_available_h, &child_w, &child_h);
        *out_width = child_w + card->padding * 2;
        *out_height = child_h + card->padding * 2;
    } else {
        *out_width = widget->layout.preferred_width > 0 ? widget->layout.preferred_width : available_width;
        *out_height = widget->layout.preferred_height > 0 ? widget->layout.preferred_height : 100;
    }
}

static void card_layout(VaxpWidget* widget, VaxpRectF bounds) {
    VaxpCard* card = (VaxpCard*)widget;
    widget->bounds = bounds;
    
    if (card->child && card->child->visible) {
        VaxpRectF child_bounds = {
            card->padding, card->padding,
            bounds.width - card->padding * 2,
            bounds.height - card->padding * 2
        };
        vaxp_widget_layout(card->child, child_bounds);
    }
}

static void card_draw(VaxpWidget* widget, VaxpCanvas* canvas) {
    VaxpCard* card = (VaxpCard*)widget;
    
    VaxpF32 w = widget->bounds.width;
    VaxpF32 h = widget->bounds.height;
    
    /* Draw shadow (based on elevation) */
    if (!card->outlined && card->elevation > 0) {
        VaxpF32 shadow_offset = card->elevation * 0.5f;
        VaxpF32 shadow_blur = card->elevation * 1.5f;
        
        /* Multiple shadow layers for realistic effect */
        for (int i = 0; i < 3; i++) {
            VaxpF32 offset = shadow_offset * (1 + i * 0.3f);
            VaxpF32 alpha = 40 - i * 10;
            
            VaxpColor sc = card->shadow_color;
            sc.a = (VaxpU8)alpha;
            
            VaxpRectF shadow = { offset, offset + shadow_blur * 0.3f, w, h };
            VaxpPaint shadow_paint = vaxp_paint_fill(sc);
            vaxp_canvas_draw_rounded_rect(canvas, shadow, card->corner_radius + shadow_blur * 0.2f, &shadow_paint);
        }
    }
    
    /* Draw card background */
    VaxpRectF bg = { 0, 0, w, h };
    VaxpPaint bg_paint = vaxp_paint_fill(card->background_color);
    vaxp_canvas_draw_rounded_rect(canvas, bg, card->corner_radius, &bg_paint);
    
    /* Draw border for outlined cards */
    if (card->outlined) {
        VaxpPaint border_paint = vaxp_paint_stroke(card->border_color, 1.0f);
        vaxp_canvas_draw_rounded_rect(canvas, bg, card->corner_radius, &border_paint);
    }
    
    /* Draw child */
    if (card->child && card->child->visible) {
        vaxp_canvas_save(canvas);
        vaxp_canvas_translate(canvas, card->child->bounds.x, card->child->bounds.y);
        vaxp_widget_draw(card->child, canvas);
        vaxp_canvas_restore(canvas);
    }
}

static VaxpBool card_on_event(VaxpWidget* widget, const VaxpEvent* event) {
    VaxpCard* card = (VaxpCard*)widget;
    
    if (card->child && card->child->visible) {
        return vaxp_widget_dispatch_event(card->child, event);
    }
    
    return VAXP_FALSE;
}

const VaxpWidgetClass vaxp_card_class = {
    .class_name = "VaxpCard",
    .instance_size = sizeof(VaxpCard),
    .parent_class = &vaxp_widget_class,
    .init = card_init,
    .destroy = card_destroy,
    .measure = card_measure,
    .layout = card_layout,
    .draw = card_draw,
    .on_event = card_on_event,
    .on_state_changed = NULL,
};

VaxpResultPtr vaxp_card_create(void) {
    return vaxp_widget_create(&vaxp_card_class);
}

VaxpResult vaxp_card_set_child(VaxpCard* card, VaxpWidget* child) {
    VAXP_ENSURE_NOT_NULL(card);
    
    if (card->child) {
        card->child->parent = NULL;
        vaxp_unref(card->child);
    }
    
    card->child = child;
    if (child) {
        vaxp_ref(child);
        child->parent = (VaxpWidget*)card;
    }
    
    vaxp_widget_invalidate_layout((VaxpWidget*)card);
    return VAXP_OK_UNIT();
}

void vaxp_card_set_elevation(VaxpCard* card, VaxpF32 elevation) {
    if (card) {
        card->elevation = elevation;
        vaxp_widget_invalidate((VaxpWidget*)card);
    }
}

void vaxp_card_set_corner_radius(VaxpCard* card, VaxpF32 radius) {
    if (card) {
        card->corner_radius = radius;
        vaxp_widget_invalidate((VaxpWidget*)card);
    }
}

void vaxp_card_set_color(VaxpCard* card, VaxpColor color) {
    if (card) {
        card->background_color = color;
        vaxp_widget_invalidate((VaxpWidget*)card);
    }
}

void vaxp_card_set_outlined(VaxpCard* card, VaxpBool outlined) {
    if (card) {
        card->outlined = outlined;
        vaxp_widget_invalidate((VaxpWidget*)card);
    }
}

VaxpWidget* _vaxp_card_build(const VaxpCardConfig* config) {
    VaxpResultPtr result = vaxp_card_create();
    if (!result.ok) return NULL;
    
    VaxpCard* card = (VaxpCard*)result.value;
    
    if (config->child) vaxp_card_set_child(card, config->child);
    if (config->elevation > 0) card->elevation = config->elevation;
    if (config->corner_radius > 0) card->corner_radius = config->corner_radius;
    if (config->padding > 0) card->padding = config->padding;
    card->outlined = config->outlined;
    if (config->color.a > 0) card->background_color = config->color;
    
    return (VaxpWidget*)card;
}
