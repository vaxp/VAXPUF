/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_card.c - Card widget implementation
 */

#include "venom/widgets/venom_card.h"
#include "venom/core/venom_memory.h"

static void card_init(VenomWidget* widget) {
    VenomCard* card = (VenomCard*)widget;
    
    card->child = NULL;
    card->elevation = 2.0f;
    
    card->background_color = (VenomColor){ 255, 255, 255, 255 };
    card->shadow_color = (VenomColor){ 0, 0, 0, 40 };
    card->corner_radius = 8.0f;
    card->padding = 16.0f;
    card->outlined = VENOM_FALSE;
    card->border_color = (VenomColor){ 224, 224, 224, 255 };
}

static void card_destroy(VenomWidget* widget) {
    VenomCard* card = (VenomCard*)widget;
    
    if (card->child) {
        card->child->parent = NULL;
        venom_unref(card->child);
        card->child = NULL;
    }
    
    venom_widget_class.destroy(widget);
}

static void card_measure(VenomWidget* widget, VenomF32 available_width, VenomF32 available_height,
                         VenomF32* out_width, VenomF32* out_height) {
    VenomCard* card = (VenomCard*)widget;
    
    VenomF32 content_available_w = available_width - card->padding * 2;
    VenomF32 content_available_h = available_height - card->padding * 2;
    
    if (card->child) {
        VenomF32 child_w, child_h;
        venom_widget_measure(card->child, content_available_w, content_available_h, &child_w, &child_h);
        *out_width = child_w + card->padding * 2;
        *out_height = child_h + card->padding * 2;
    } else {
        *out_width = widget->layout.preferred_width > 0 ? widget->layout.preferred_width : available_width;
        *out_height = widget->layout.preferred_height > 0 ? widget->layout.preferred_height : 100;
    }
}

static void card_layout(VenomWidget* widget, VenomRectF bounds) {
    VenomCard* card = (VenomCard*)widget;
    widget->bounds = bounds;
    
    if (card->child && card->child->visible) {
        VenomRectF child_bounds = {
            card->padding, card->padding,
            bounds.width - card->padding * 2,
            bounds.height - card->padding * 2
        };
        venom_widget_layout(card->child, child_bounds);
    }
}

static void card_draw(VenomWidget* widget, VenomCanvas* canvas) {
    VenomCard* card = (VenomCard*)widget;
    
    VenomF32 w = widget->bounds.width;
    VenomF32 h = widget->bounds.height;
    
    /* Draw shadow (based on elevation) */
    if (!card->outlined && card->elevation > 0) {
        VenomF32 shadow_offset = card->elevation * 0.5f;
        VenomF32 shadow_blur = card->elevation * 1.5f;
        
        /* Multiple shadow layers for realistic effect */
        for (int i = 0; i < 3; i++) {
            VenomF32 offset = shadow_offset * (1 + i * 0.3f);
            VenomF32 alpha = 40 - i * 10;
            
            VenomColor sc = card->shadow_color;
            sc.a = (VenomU8)alpha;
            
            VenomRectF shadow = { offset, offset + shadow_blur * 0.3f, w, h };
            VenomPaint shadow_paint = venom_paint_fill(sc);
            venom_canvas_draw_rounded_rect(canvas, shadow, card->corner_radius + shadow_blur * 0.2f, &shadow_paint);
        }
    }
    
    /* Draw card background */
    VenomRectF bg = { 0, 0, w, h };
    VenomPaint bg_paint = venom_paint_fill(card->background_color);
    venom_canvas_draw_rounded_rect(canvas, bg, card->corner_radius, &bg_paint);
    
    /* Draw border for outlined cards */
    if (card->outlined) {
        VenomPaint border_paint = venom_paint_stroke(card->border_color, 1.0f);
        venom_canvas_draw_rounded_rect(canvas, bg, card->corner_radius, &border_paint);
    }
    
    /* Draw child */
    if (card->child && card->child->visible) {
        venom_canvas_save(canvas);
        venom_canvas_translate(canvas, card->child->bounds.x, card->child->bounds.y);
        venom_widget_draw(card->child, canvas);
        venom_canvas_restore(canvas);
    }
}

static VenomBool card_on_event(VenomWidget* widget, const VenomEvent* event) {
    VenomCard* card = (VenomCard*)widget;
    
    if (card->child && card->child->visible) {
        return venom_widget_dispatch_event(card->child, event);
    }
    
    return VENOM_FALSE;
}

const VenomWidgetClass venom_card_class = {
    .class_name = "VenomCard",
    .instance_size = sizeof(VenomCard),
    .parent_class = &venom_widget_class,
    .init = card_init,
    .destroy = card_destroy,
    .measure = card_measure,
    .layout = card_layout,
    .draw = card_draw,
    .on_event = card_on_event,
    .on_state_changed = NULL,
};

VenomResultPtr venom_card_create(void) {
    return venom_widget_create(&venom_card_class);
}

VenomResult venom_card_set_child(VenomCard* card, VenomWidget* child) {
    VENOM_ENSURE_NOT_NULL(card);
    
    if (card->child) {
        card->child->parent = NULL;
        venom_unref(card->child);
    }
    
    card->child = child;
    if (child) {
        venom_ref(child);
        child->parent = (VenomWidget*)card;
    }
    
    venom_widget_invalidate_layout((VenomWidget*)card);
    return VENOM_OK_UNIT();
}

void venom_card_set_elevation(VenomCard* card, VenomF32 elevation) {
    if (card) {
        card->elevation = elevation;
        venom_widget_invalidate((VenomWidget*)card);
    }
}

void venom_card_set_corner_radius(VenomCard* card, VenomF32 radius) {
    if (card) {
        card->corner_radius = radius;
        venom_widget_invalidate((VenomWidget*)card);
    }
}

void venom_card_set_color(VenomCard* card, VenomColor color) {
    if (card) {
        card->background_color = color;
        venom_widget_invalidate((VenomWidget*)card);
    }
}

void venom_card_set_outlined(VenomCard* card, VenomBool outlined) {
    if (card) {
        card->outlined = outlined;
        venom_widget_invalidate((VenomWidget*)card);
    }
}

VenomWidget* _venom_card_build(const VenomCardConfig* config) {
    VenomResultPtr result = venom_card_create();
    if (!result.ok) return NULL;
    
    VenomCard* card = (VenomCard*)result.value;
    
    if (config->child) venom_card_set_child(card, config->child);
    if (config->elevation > 0) card->elevation = config->elevation;
    if (config->corner_radius > 0) card->corner_radius = config->corner_radius;
    if (config->padding > 0) card->padding = config->padding;
    card->outlined = config->outlined;
    if (config->color.a > 0) card->background_color = config->color;
    
    return (VenomWidget*)card;
}
