/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_rating.c - Star rating implementation
 */

#include "vaxp/widgets/vaxp_rating.h"
#include "vaxp/core/vaxp_memory.h"

#define DEFAULT_STAR_SIZE 24.0f
#define DEFAULT_SPACING 4.0f
#define DEFAULT_MAX 5

static void rating_init(VaxpWidget* widget) {
    VaxpRating* rating = (VaxpRating*)widget;
    
    rating->value = 0;
    rating->max_value = DEFAULT_MAX;
    rating->allow_half = VAXP_TRUE;
    rating->read_only = VAXP_FALSE;
    
    rating->on_change = NULL;
    rating->callback_data = NULL;
    
    rating->star_size = DEFAULT_STAR_SIZE;
    rating->spacing = DEFAULT_SPACING;
    rating->filled_color = (VaxpColor){ 255, 193, 7, 255 };
    rating->empty_color = (VaxpColor){ 189, 189, 189, 255 };
    rating->hover_index = -1;
    
    widget->focusable = VAXP_TRUE;
}

static void rating_measure(VaxpWidget* widget, VaxpF32 available_width, VaxpF32 available_height,
                           VaxpF32* out_width, VaxpF32* out_height) {
    VaxpRating* rating = (VaxpRating*)widget;
    (void)available_width; (void)available_height;
    
    *out_width = rating->max_value * (rating->star_size + rating->spacing) - rating->spacing;
    *out_height = rating->star_size;
}

static void rating_draw(VaxpWidget* widget, VaxpCanvas* canvas) {
    VaxpRating* rating = (VaxpRating*)widget;
    
    VaxpF32 s = rating->star_size;
    VaxpF32 spacing = rating->spacing;
    
    for (VaxpU32 i = 0; i < rating->max_value; i++) {
        VaxpF32 x = i * (s + spacing) + s / 2;
        VaxpF32 y = s / 2;
        
        VaxpF32 fill = 0;
        if ((VaxpF32)i + 1 <= rating->value) {
            fill = 1.0f;
        } else if ((VaxpF32)i < rating->value) {
            fill = rating->value - (VaxpF32)i;
        }
        
        /* Draw empty star */
        VaxpPaint empty_paint = vaxp_paint_fill(rating->empty_color);
        vaxp_canvas_draw_text(canvas, "★", x - s * 0.4f, y + s * 0.35f, NULL, &empty_paint);
        
        /* Draw filled portion */
        if (fill > 0) {
            VaxpPaint filled_paint = vaxp_paint_fill(rating->filled_color);
            
            if (fill >= 1.0f) {
                vaxp_canvas_draw_text(canvas, "★", x - s * 0.4f, y + s * 0.35f, NULL, &filled_paint);
            } else {
                /* Partial fill - draw clipped */
                vaxp_canvas_save(canvas);
                VaxpRectF clip = { i * (s + spacing), 0, s * fill, s };
                vaxp_canvas_clip_rect(canvas, clip);
                vaxp_canvas_draw_text(canvas, "★", x - s * 0.4f, y + s * 0.35f, NULL, &filled_paint);
                vaxp_canvas_restore(canvas);
            }
        }
    }
}

static VaxpBool rating_on_event(VaxpWidget* widget, const VaxpEvent* event) {
    VaxpRating* rating = (VaxpRating*)widget;
    
    if (rating->read_only) return VAXP_FALSE;
    
    if (event->type == VAXP_EVENT_MOUSE_MOVE) {
        VaxpF32 mx = (VaxpF32)event->mouse.x;
        VaxpF32 step = rating->star_size + rating->spacing;
        VaxpI32 new_hover = (VaxpI32)(mx / step);
        
        if (new_hover != rating->hover_index) {
            rating->hover_index = new_hover;
            widget->needs_redraw = VAXP_TRUE;
        }
    }
    
    if (event->type == VAXP_EVENT_MOUSE_BUTTON_DOWN && 
        event->mouse.button == VAXP_MOUSE_BUTTON_LEFT) {
        VaxpF32 mx = (VaxpF32)event->mouse.x;
        VaxpF32 step = rating->star_size + rating->spacing;
        VaxpF32 star_idx = mx / step;
        
        VaxpF32 new_value;
        if (rating->allow_half) {
            VaxpF32 within_star = (mx - ((VaxpU32)star_idx * step)) / rating->star_size;
            new_value = (VaxpF32)((VaxpU32)star_idx) + (within_star > 0.5f ? 1.0f : 0.5f);
        } else {
            new_value = (VaxpF32)((VaxpU32)star_idx + 1);
        }
        
        if (new_value > (VaxpF32)rating->max_value) new_value = (VaxpF32)rating->max_value;
        
        if (new_value != rating->value) {
            rating->value = new_value;
            widget->needs_redraw = VAXP_TRUE;
            
            if (rating->on_change) {
                rating->on_change(rating, rating->value, rating->callback_data);
            }
        }
        return VAXP_TRUE;
    }
    
    if (event->type == VAXP_EVENT_MOUSE_LEAVE) {
        rating->hover_index = -1;
        widget->needs_redraw = VAXP_TRUE;
    }
    
    return VAXP_FALSE;
}

const VaxpWidgetClass vaxp_rating_class = {
    .class_name = "VaxpRating",
    .instance_size = sizeof(VaxpRating),
    .parent_class = &vaxp_widget_class,
    .init = rating_init,
    .destroy = NULL,
    .measure = rating_measure,
    .layout = NULL,
    .draw = rating_draw,
    .on_event = rating_on_event,
    .on_state_changed = NULL,
};

VaxpResultPtr vaxp_rating_create(void) {
    return vaxp_widget_create(&vaxp_rating_class);
}

void vaxp_rating_set_value(VaxpRating* rating, VaxpF32 value) {
    if (rating) {
        if (value < 0) value = 0;
        if (value > (VaxpF32)rating->max_value) value = (VaxpF32)rating->max_value;
        rating->value = value;
        vaxp_widget_invalidate((VaxpWidget*)rating);
    }
}

VaxpF32 vaxp_rating_get_value(const VaxpRating* rating) {
    return rating ? rating->value : 0;
}

void vaxp_rating_set_max(VaxpRating* rating, VaxpU32 max) {
    if (rating && max > 0) {
        rating->max_value = max;
        if (rating->value > (VaxpF32)max) rating->value = (VaxpF32)max;
        vaxp_widget_invalidate((VaxpWidget*)rating);
    }
}

void vaxp_rating_set_on_change(VaxpRating* rating, VaxpRatingCallback callback, void* data) {
    if (rating) {
        rating->on_change = callback;
        rating->callback_data = data;
    }
}

VaxpWidget* _vaxp_rating_build(const VaxpRatingConfig* config) {
    VaxpResultPtr result = vaxp_rating_create();
    if (!result.ok) return NULL;
    
    VaxpRating* rating = (VaxpRating*)result.value;
    
    rating->value = config->value;
    if (config->max > 0) rating->max_value = config->max;
    rating->read_only = config->read_only;
    rating->on_change = config->on_change;
    rating->callback_data = config->data;
    
    return (VaxpWidget*)rating;
}
