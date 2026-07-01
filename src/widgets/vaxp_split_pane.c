/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_split_pane.c - Split pane implementation
 */

#include "vaxp/widgets/vaxp_split_pane.h"
#include "vaxp/core/vaxp_memory.h"

#define DEFAULT_DIVIDER_SIZE 6.0f
#define DEFAULT_MIN_SIZE 50.0f

static void split_pane_init(VaxpWidget* widget) {
    VaxpSplitPane* pane = (VaxpSplitPane*)widget;
    
    pane->first = NULL;
    pane->second = NULL;
    
    pane->direction = VAXP_SPLIT_HORIZONTAL;
    pane->position = 0.5f;
    pane->min_first = DEFAULT_MIN_SIZE;
    pane->min_second = DEFAULT_MIN_SIZE;
    pane->divider_size = DEFAULT_DIVIDER_SIZE;
    
    pane->dragging = VAXP_FALSE;
    pane->divider_color = (VaxpColor){ 224, 224, 224, 255 };
    pane->divider_hover = (VaxpColor){ 63, 81, 181, 255 };
    pane->hovering = VAXP_FALSE;
}

static void split_pane_destroy(VaxpWidget* widget) {
    VaxpSplitPane* pane = (VaxpSplitPane*)widget;
    
    if (pane->first) {
        pane->first->parent = NULL;
        vaxp_unref(pane->first);
    }
    if (pane->second) {
        pane->second->parent = NULL;
        vaxp_unref(pane->second);
    }
    
    vaxp_widget_class.destroy(widget);
}

static void split_pane_measure(VaxpWidget* widget, VaxpF32 available_width, VaxpF32 available_height,
                               VaxpF32* out_width, VaxpF32* out_height) {
    *out_width = available_width;
    *out_height = available_height;
}

static VaxpF32 get_split_position(VaxpSplitPane* pane, VaxpF32 total_size) {
    VaxpF32 pos;
    if (pane->position <= 1.0f) {
        pos = total_size * pane->position;
    } else {
        pos = pane->position;
    }
    
    if (pos < pane->min_first) pos = pane->min_first;
    if (pos > total_size - pane->min_second - pane->divider_size) {
        pos = total_size - pane->min_second - pane->divider_size;
    }
    
    return pos;
}

static void split_pane_layout(VaxpWidget* widget, VaxpRectF bounds) {
    VaxpSplitPane* pane = (VaxpSplitPane*)widget;
    widget->bounds = bounds;
    
    VaxpBool horiz = (pane->direction == VAXP_SPLIT_HORIZONTAL);
    VaxpF32 total = horiz ? bounds.width : bounds.height;
    VaxpF32 split_pos = get_split_position(pane, total);
    
    if (pane->first && pane->first->visible) {
        VaxpRectF first_bounds;
        if (horiz) {
            first_bounds = (VaxpRectF){ 0, 0, split_pos, bounds.height };
        } else {
            first_bounds = (VaxpRectF){ 0, 0, bounds.width, split_pos };
        }
        vaxp_widget_layout(pane->first, first_bounds);
    }
    
    if (pane->second && pane->second->visible) {
        VaxpRectF second_bounds;
        if (horiz) {
            second_bounds = (VaxpRectF){ 
                split_pos + pane->divider_size, 0, 
                bounds.width - split_pos - pane->divider_size, bounds.height 
            };
        } else {
            second_bounds = (VaxpRectF){ 
                0, split_pos + pane->divider_size,
                bounds.width, bounds.height - split_pos - pane->divider_size 
            };
        }
        vaxp_widget_layout(pane->second, second_bounds);
    }
}

static void split_pane_draw(VaxpWidget* widget, VaxpCanvas* canvas) {
    VaxpSplitPane* pane = (VaxpSplitPane*)widget;
    
    VaxpBool horiz = (pane->direction == VAXP_SPLIT_HORIZONTAL);
    VaxpF32 total = horiz ? widget->bounds.width : widget->bounds.height;
    VaxpF32 split_pos = get_split_position(pane, total);
    
    /* Draw first panel */
    if (pane->first && pane->first->visible) {
        vaxp_canvas_save(canvas);
        vaxp_canvas_translate(canvas, pane->first->bounds.x, pane->first->bounds.y);
        vaxp_widget_draw(pane->first, canvas);
        vaxp_canvas_restore(canvas);
    }
    
    /* Draw divider */
    VaxpRectF divider;
    if (horiz) {
        divider = (VaxpRectF){ split_pos, 0, pane->divider_size, widget->bounds.height };
    } else {
        divider = (VaxpRectF){ 0, split_pos, widget->bounds.width, pane->divider_size };
    }
    
    VaxpColor div_color = (pane->hovering || pane->dragging) ? pane->divider_hover : pane->divider_color;
    VaxpPaint div_paint = vaxp_paint_fill(div_color);
    vaxp_canvas_draw_rect(canvas, divider, &div_paint);
    
    /* Draw grip */
    VaxpF32 grip_x = divider.x + divider.width / 2;
    VaxpF32 grip_y = divider.y + divider.height / 2;
    VaxpPaint grip_paint = vaxp_paint_fill((VaxpColor){ 150, 150, 150, 255 });
    
    if (horiz) {
        for (int i = -2; i <= 2; i++) {
            vaxp_canvas_draw_circle(canvas, grip_x, grip_y + i * 6, 2, &grip_paint);
        }
    } else {
        for (int i = -2; i <= 2; i++) {
            vaxp_canvas_draw_circle(canvas, grip_x + i * 6, grip_y, 2, &grip_paint);
        }
    }
    
    /* Draw second panel */
    if (pane->second && pane->second->visible) {
        vaxp_canvas_save(canvas);
        vaxp_canvas_translate(canvas, pane->second->bounds.x, pane->second->bounds.y);
        vaxp_widget_draw(pane->second, canvas);
        vaxp_canvas_restore(canvas);
    }
}

static VaxpBool is_on_divider(VaxpSplitPane* pane, VaxpF32 mx, VaxpF32 my) {
    VaxpWidget* widget = (VaxpWidget*)pane;
    VaxpBool horiz = (pane->direction == VAXP_SPLIT_HORIZONTAL);
    VaxpF32 total = horiz ? widget->bounds.width : widget->bounds.height;
    VaxpF32 split_pos = get_split_position(pane, total);
    
    if (horiz) {
        return mx >= split_pos && mx <= split_pos + pane->divider_size;
    } else {
        return my >= split_pos && my <= split_pos + pane->divider_size;
    }
}

static VaxpBool split_pane_on_event(VaxpWidget* widget, const VaxpEvent* event) {
    VaxpSplitPane* pane = (VaxpSplitPane*)widget;
    
    switch (event->type) {
        case VAXP_EVENT_MOUSE_MOVE: {
            VaxpF32 mx = (VaxpF32)event->mouse.x;
            VaxpF32 my = (VaxpF32)event->mouse.y;
            
            if (pane->dragging) {
                VaxpBool horiz = (pane->direction == VAXP_SPLIT_HORIZONTAL);
                VaxpF32 total = horiz ? widget->bounds.width : widget->bounds.height;
                VaxpF32 pos = horiz ? mx : my;
                
                pane->position = pos / total;
                if (pane->position < 0.1f) pane->position = 0.1f;
                if (pane->position > 0.9f) pane->position = 0.9f;
                
                vaxp_widget_invalidate_layout(widget);
                return VAXP_TRUE;
            }
            
            VaxpBool on_div = is_on_divider(pane, mx, my);
            if (on_div != pane->hovering) {
                pane->hovering = on_div;
                widget->needs_redraw = VAXP_TRUE;
            }
            break;
        }
        
        case VAXP_EVENT_MOUSE_BUTTON_DOWN:
            if (event->mouse.button == VAXP_MOUSE_BUTTON_LEFT) {
                if (is_on_divider(pane, (VaxpF32)event->mouse.x, (VaxpF32)event->mouse.y)) {
                    pane->dragging = VAXP_TRUE;
                    return VAXP_TRUE;
                }
            }
            break;
            
        case VAXP_EVENT_MOUSE_BUTTON_UP:
            if (pane->dragging) {
                pane->dragging = VAXP_FALSE;
                return VAXP_TRUE;
            }
            break;
            
        case VAXP_EVENT_MOUSE_LEAVE:
            pane->hovering = VAXP_FALSE;
            pane->dragging = VAXP_FALSE;
            widget->needs_redraw = VAXP_TRUE;
            break;
            
        default:
            break;
    }
    
    /* Pass events to panels */
    if (pane->first && vaxp_widget_dispatch_event(pane->first, event)) return VAXP_TRUE;
    if (pane->second && vaxp_widget_dispatch_event(pane->second, event)) return VAXP_TRUE;
    
    return VAXP_FALSE;
}

const VaxpWidgetClass vaxp_split_pane_class = {
    .class_name = "VaxpSplitPane",
    .instance_size = sizeof(VaxpSplitPane),
    .parent_class = &vaxp_widget_class,
    .init = split_pane_init,
    .destroy = split_pane_destroy,
    .measure = split_pane_measure,
    .layout = split_pane_layout,
    .draw = split_pane_draw,
    .on_event = split_pane_on_event,
    .on_state_changed = NULL,
};

VaxpResultPtr vaxp_split_pane_create(void) {
    return vaxp_widget_create(&vaxp_split_pane_class);
}

VaxpResult vaxp_split_pane_set_first(VaxpSplitPane* pane, VaxpWidget* widget) {
    VAXP_ENSURE_NOT_NULL(pane);
    
    if (pane->first) {
        pane->first->parent = NULL;
        vaxp_unref(pane->first);
    }
    
    pane->first = widget;
    if (widget) {
        vaxp_ref(widget);
        widget->parent = (VaxpWidget*)pane;
    }
    
    return VAXP_OK_UNIT();
}

VaxpResult vaxp_split_pane_set_second(VaxpSplitPane* pane, VaxpWidget* widget) {
    VAXP_ENSURE_NOT_NULL(pane);
    
    if (pane->second) {
        pane->second->parent = NULL;
        vaxp_unref(pane->second);
    }
    
    pane->second = widget;
    if (widget) {
        vaxp_ref(widget);
        widget->parent = (VaxpWidget*)pane;
    }
    
    return VAXP_OK_UNIT();
}

void vaxp_split_pane_set_position(VaxpSplitPane* pane, VaxpF32 position) {
    if (pane) {
        pane->position = position;
        vaxp_widget_invalidate_layout((VaxpWidget*)pane);
    }
}

void vaxp_split_pane_set_direction(VaxpSplitPane* pane, VaxpSplitDirection dir) {
    if (pane) {
        pane->direction = dir;
        vaxp_widget_invalidate_layout((VaxpWidget*)pane);
    }
}

void vaxp_split_pane_set_min_sizes(VaxpSplitPane* pane, VaxpF32 min_first, VaxpF32 min_second) {
    if (pane) {
        pane->min_first = min_first;
        pane->min_second = min_second;
    }
}

VaxpWidget* _vaxp_split_pane_build(const VaxpSplitPaneConfig* config) {
    VaxpResultPtr result = vaxp_split_pane_create();
    if (!result.ok) return NULL;
    
    VaxpSplitPane* pane = (VaxpSplitPane*)result.value;
    
    if (config->first) vaxp_split_pane_set_first(pane, config->first);
    if (config->second) vaxp_split_pane_set_second(pane, config->second);
    pane->direction = config->direction;
    if (config->position > 0) pane->position = config->position;
    
    return (VaxpWidget*)pane;
}
