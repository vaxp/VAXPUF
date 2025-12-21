/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_rating.c - Star rating implementation
 */

#include "venom/widgets/venom_rating.h"
#include "venom/core/venom_memory.h"

#define DEFAULT_STAR_SIZE 24.0f
#define DEFAULT_SPACING 4.0f
#define DEFAULT_MAX 5

static void rating_init(VenomWidget* widget) {
    VenomRating* rating = (VenomRating*)widget;
    
    rating->value = 0;
    rating->max_value = DEFAULT_MAX;
    rating->allow_half = VENOM_TRUE;
    rating->read_only = VENOM_FALSE;
    
    rating->on_change = NULL;
    rating->callback_data = NULL;
    
    rating->star_size = DEFAULT_STAR_SIZE;
    rating->spacing = DEFAULT_SPACING;
    rating->filled_color = (VenomColor){ 255, 193, 7, 255 };
    rating->empty_color = (VenomColor){ 189, 189, 189, 255 };
    rating->hover_index = -1;
    
    widget->focusable = VENOM_TRUE;
}

static void rating_measure(VenomWidget* widget, VenomF32 available_width, VenomF32 available_height,
                           VenomF32* out_width, VenomF32* out_height) {
    VenomRating* rating = (VenomRating*)widget;
    (void)available_width; (void)available_height;
    
    *out_width = rating->max_value * (rating->star_size + rating->spacing) - rating->spacing;
    *out_height = rating->star_size;
}

static void rating_draw(VenomWidget* widget, VenomCanvas* canvas) {
    VenomRating* rating = (VenomRating*)widget;
    
    VenomF32 s = rating->star_size;
    VenomF32 spacing = rating->spacing;
    
    for (VenomU32 i = 0; i < rating->max_value; i++) {
        VenomF32 x = i * (s + spacing) + s / 2;
        VenomF32 y = s / 2;
        
        VenomF32 fill = 0;
        if ((VenomF32)i + 1 <= rating->value) {
            fill = 1.0f;
        } else if ((VenomF32)i < rating->value) {
            fill = rating->value - (VenomF32)i;
        }
        
        /* Draw empty star */
        VenomPaint empty_paint = venom_paint_fill(rating->empty_color);
        venom_canvas_draw_text(canvas, "★", x - s * 0.4f, y + s * 0.35f, NULL, &empty_paint);
        
        /* Draw filled portion */
        if (fill > 0) {
            VenomPaint filled_paint = venom_paint_fill(rating->filled_color);
            
            if (fill >= 1.0f) {
                venom_canvas_draw_text(canvas, "★", x - s * 0.4f, y + s * 0.35f, NULL, &filled_paint);
            } else {
                /* Partial fill - draw clipped */
                venom_canvas_save(canvas);
                VenomRectF clip = { i * (s + spacing), 0, s * fill, s };
                venom_canvas_clip_rect(canvas, clip);
                venom_canvas_draw_text(canvas, "★", x - s * 0.4f, y + s * 0.35f, NULL, &filled_paint);
                venom_canvas_restore(canvas);
            }
        }
    }
}

static VenomBool rating_on_event(VenomWidget* widget, const VenomEvent* event) {
    VenomRating* rating = (VenomRating*)widget;
    
    if (rating->read_only) return VENOM_FALSE;
    
    if (event->type == VENOM_EVENT_MOUSE_MOVE) {
        VenomF32 mx = (VenomF32)event->mouse.x;
        VenomF32 step = rating->star_size + rating->spacing;
        VenomI32 new_hover = (VenomI32)(mx / step);
        
        if (new_hover != rating->hover_index) {
            rating->hover_index = new_hover;
            widget->needs_redraw = VENOM_TRUE;
        }
    }
    
    if (event->type == VENOM_EVENT_MOUSE_BUTTON_DOWN && 
        event->mouse.button == VENOM_MOUSE_BUTTON_LEFT) {
        VenomF32 mx = (VenomF32)event->mouse.x;
        VenomF32 step = rating->star_size + rating->spacing;
        VenomF32 star_idx = mx / step;
        
        VenomF32 new_value;
        if (rating->allow_half) {
            VenomF32 within_star = (mx - ((VenomU32)star_idx * step)) / rating->star_size;
            new_value = (VenomF32)((VenomU32)star_idx) + (within_star > 0.5f ? 1.0f : 0.5f);
        } else {
            new_value = (VenomF32)((VenomU32)star_idx + 1);
        }
        
        if (new_value > (VenomF32)rating->max_value) new_value = (VenomF32)rating->max_value;
        
        if (new_value != rating->value) {
            rating->value = new_value;
            widget->needs_redraw = VENOM_TRUE;
            
            if (rating->on_change) {
                rating->on_change(rating, rating->value, rating->callback_data);
            }
        }
        return VENOM_TRUE;
    }
    
    if (event->type == VENOM_EVENT_MOUSE_LEAVE) {
        rating->hover_index = -1;
        widget->needs_redraw = VENOM_TRUE;
    }
    
    return VENOM_FALSE;
}

const VenomWidgetClass venom_rating_class = {
    .class_name = "VenomRating",
    .instance_size = sizeof(VenomRating),
    .parent_class = &venom_widget_class,
    .init = rating_init,
    .destroy = NULL,
    .measure = rating_measure,
    .layout = NULL,
    .draw = rating_draw,
    .on_event = rating_on_event,
    .on_state_changed = NULL,
};

VenomResultPtr venom_rating_create(void) {
    return venom_widget_create(&venom_rating_class);
}

void venom_rating_set_value(VenomRating* rating, VenomF32 value) {
    if (rating) {
        if (value < 0) value = 0;
        if (value > (VenomF32)rating->max_value) value = (VenomF32)rating->max_value;
        rating->value = value;
        venom_widget_invalidate((VenomWidget*)rating);
    }
}

VenomF32 venom_rating_get_value(const VenomRating* rating) {
    return rating ? rating->value : 0;
}

void venom_rating_set_max(VenomRating* rating, VenomU32 max) {
    if (rating && max > 0) {
        rating->max_value = max;
        if (rating->value > (VenomF32)max) rating->value = (VenomF32)max;
        venom_widget_invalidate((VenomWidget*)rating);
    }
}

void venom_rating_set_on_change(VenomRating* rating, VenomRatingCallback callback, void* data) {
    if (rating) {
        rating->on_change = callback;
        rating->callback_data = data;
    }
}

VenomWidget* _venom_rating_build(const VenomRatingConfig* config) {
    VenomResultPtr result = venom_rating_create();
    if (!result.ok) return NULL;
    
    VenomRating* rating = (VenomRating*)result.value;
    
    rating->value = config->value;
    if (config->max > 0) rating->max_value = config->max;
    rating->read_only = config->read_only;
    rating->on_change = config->on_change;
    rating->callback_data = config->data;
    
    return (VenomWidget*)rating;
}
