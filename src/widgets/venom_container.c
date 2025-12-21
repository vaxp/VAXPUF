/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_container.c - Container widget with flexbox layout
 */

#include "venom/widgets/venom_container.h"
#include "venom/core/venom_memory.h"

/* ============================================================================
 * CONTAINER CLASS METHODS
 * ============================================================================ */

static void container_init(VenomWidget* widget) {
    VenomContainer* c = (VenomContainer*)widget;
    
    c->direction = VENOM_FLEX_ROW;
    c->justify = VENOM_JUSTIFY_START;
    c->align_items = VENOM_ALIGN_START;
    c->gap = 0;
    c->bg_color = VENOM_COLOR_TRANSPARENT;
    c->corner_radius = 0;
    c->has_background = VENOM_FALSE;
}

static void container_measure(VenomWidget* widget, VenomF32 available_width, VenomF32 available_height,
                               VenomF32* out_width, VenomF32* out_height) {
    VenomContainer* c = (VenomContainer*)widget;
    VenomBool is_row = (c->direction == VENOM_FLEX_ROW || c->direction == VENOM_FLEX_ROW_REVERSE);
    
    VenomF32 main_axis = 0;
    VenomF32 cross_axis = 0;
    VenomU32 child_count = venom_widget_child_count(widget);
    
    for (VenomU32 i = 0; i < child_count; i++) {
        VenomWidget* child = venom_widget_child_at(widget, i);
        if (!child->visible) continue;
        
        VenomF32 cw, ch;
        venom_widget_measure(child, available_width, available_height, &cw, &ch);
        
        /* Add margin */
        cw += child->layout.margin.left + child->layout.margin.right;
        ch += child->layout.margin.top + child->layout.margin.bottom;
        
        if (is_row) {
            main_axis += cw;
            if (ch > cross_axis) cross_axis = ch;
        } else {
            main_axis += ch;
            if (cw > cross_axis) cross_axis = cw;
        }
    }
    
    /* Add gaps */
    if (child_count > 1) {
        main_axis += c->gap * (child_count - 1);
    }
    
    /* Add padding */
    VenomF32 pad_h = widget->layout.padding.left + widget->layout.padding.right;
    VenomF32 pad_v = widget->layout.padding.top + widget->layout.padding.bottom;
    
    if (is_row) {
        *out_width = main_axis + pad_h;
        *out_height = cross_axis + pad_v;
    } else {
        *out_width = cross_axis + pad_h;
        *out_height = main_axis + pad_v;
    }
}

static void container_layout(VenomWidget* widget, VenomRectF bounds) {
    /* First do base layout */
    venom_widget_class.layout(widget, bounds);
    
    VenomContainer* c = (VenomContainer*)widget;
    VenomU32 child_count = venom_widget_child_count(widget);
    if (child_count == 0) return;
    
    VenomBool is_row = (c->direction == VENOM_FLEX_ROW || c->direction == VENOM_FLEX_ROW_REVERSE);
    VenomBool is_reverse = (c->direction == VENOM_FLEX_ROW_REVERSE || c->direction == VENOM_FLEX_COLUMN_REVERSE);
    
    VenomRectF content = widget->content_rect;
    
    /* Measure children and calculate totals */
    VenomF32 total_main = 0;
    VenomF32 total_flex_grow = 0;
    VenomU32 visible_count = 0;
    
    VenomF32* sizes = (VenomF32*)venom_alloc(sizeof(VenomF32) * 2 * child_count);
    if (!sizes) return;
    VenomF32* widths = sizes;
    VenomF32* heights = sizes + child_count;
    
    for (VenomU32 i = 0; i < child_count; i++) {
        VenomWidget* child = venom_widget_child_at(widget, i);
        if (!child->visible) {
            widths[i] = heights[i] = 0;
            continue;
        }
        
        venom_widget_measure(child, content.width, content.height, &widths[i], &heights[i]);
        
        total_main += is_row ? widths[i] : heights[i];
        total_flex_grow += child->layout.flex_grow;
        visible_count++;
    }
    
    /* Calculate available space */
    VenomF32 main_size = is_row ? content.width : content.height;
    VenomF32 gap_space = visible_count > 1 ? c->gap * (visible_count - 1) : 0;
    VenomF32 free_space = main_size - total_main - gap_space;
    
    /* Distribute extra space if flex_grow is used */
    if (free_space > 0 && total_flex_grow > 0) {
        VenomF32 per_grow = free_space / total_flex_grow;
        for (VenomU32 i = 0; i < child_count; i++) {
            VenomWidget* child = venom_widget_child_at(widget, i);
            if (!child->visible) continue;
            
            VenomF32 extra = per_grow * child->layout.flex_grow;
            if (is_row) {
                widths[i] += extra;
            } else {
                heights[i] += extra;
            }
        }
        free_space = 0;
    }
    
    /* Calculate starting position based on justify */
    VenomF32 pos = is_row ? content.x : content.y;
    VenomF32 spacing = 0;
    
    switch (c->justify) {
        case VENOM_JUSTIFY_CENTER:
            pos += free_space / 2;
            break;
        case VENOM_JUSTIFY_END:
            pos += free_space;
            break;
        case VENOM_JUSTIFY_SPACE_BETWEEN:
            if (visible_count > 1) {
                spacing = free_space / (visible_count - 1);
            }
            break;
        case VENOM_JUSTIFY_SPACE_AROUND:
            spacing = free_space / visible_count;
            pos += spacing / 2;
            break;
        case VENOM_JUSTIFY_SPACE_EVENLY:
            spacing = free_space / (visible_count + 1);
            pos += spacing;
            break;
        default:
            break;
    }
    
    /* Layout children */
    for (VenomU32 i = 0; i < child_count; i++) {
        VenomU32 idx = is_reverse ? (child_count - 1 - i) : i;
        VenomWidget* child = venom_widget_child_at(widget, idx);
        if (!child->visible) continue;
        
        VenomF32 w = widths[idx];
        VenomF32 h = heights[idx];
        VenomF32 x, y;
        
        if (is_row) {
            x = pos;
            pos += w + c->gap + spacing;
            
            /* Cross axis alignment */
            switch (c->align_items) {
                case VENOM_ALIGN_CENTER:
                    y = content.y + (content.height - h) / 2;
                    break;
                case VENOM_ALIGN_END:
                    y = content.y + content.height - h;
                    break;
                case VENOM_ALIGN_STRETCH:
                    y = content.y;
                    h = content.height;
                    break;
                default:
                    y = content.y;
                    break;
            }
        } else {
            y = pos;
            pos += h + c->gap + spacing;
            
            switch (c->align_items) {
                case VENOM_ALIGN_CENTER:
                    x = content.x + (content.width - w) / 2;
                    break;
                case VENOM_ALIGN_END:
                    x = content.x + content.width - w;
                    break;
                case VENOM_ALIGN_STRETCH:
                    x = content.x;
                    w = content.width;
                    break;
                default:
                    x = content.x;
                    break;
            }
        }
        
        /* Apply margin */
        x += child->layout.margin.left;
        y += child->layout.margin.top;
        w -= child->layout.margin.left + child->layout.margin.right;
        h -= child->layout.margin.top + child->layout.margin.bottom;
        
        /* Convert to local coordinates (relative to parent) */
        x -= bounds.x;
        y -= bounds.y;
        
        VenomRectF child_bounds = { x, y, w, h };
        venom_widget_layout(child, child_bounds);
    }
    
    venom_free(sizes, sizeof(VenomF32) * 2 * child_count);
}

static void container_draw(VenomWidget* widget, VenomCanvas* canvas) {
    VenomContainer* c = (VenomContainer*)widget;
    
    /* Draw background if set */
    if (c->has_background) {
        VenomRectF rect = { 0, 0, widget->bounds.width, widget->bounds.height };
        VenomPaint paint = venom_paint_fill(c->bg_color);
        
        if (c->corner_radius > 0) {
            venom_canvas_draw_rounded_rect(canvas, rect, c->corner_radius, &paint);
        } else {
            venom_canvas_draw_rect(canvas, rect, &paint);
        }
    }
    
    /* Children are drawn by the widget system after this */
}

/* ============================================================================
 * CONTAINER CLASS
 * ============================================================================ */

const VenomWidgetClass venom_container_class = {
    .class_name = "VenomContainer",
    .instance_size = sizeof(VenomContainer),
    .parent_class = &venom_widget_class,
    .init = container_init,
    .destroy = NULL,  /* Will use parent's at runtime */
    .measure = container_measure,
    .layout = container_layout,
    .draw = container_draw,
    .on_event = NULL,  /* Will use parent's at runtime */
    .on_state_changed = NULL,  /* Will use parent's at runtime */
};

/* ============================================================================
 * PUBLIC API
 * ============================================================================ */

VenomResultPtr venom_container_create(void) {
    return venom_widget_create(&venom_container_class);
}

VenomResultPtr venom_container_create_row(void) {
    VenomResultPtr result = venom_container_create();
    if (result.ok) {
        VenomContainer* c = (VenomContainer*)result.value;
        c->direction = VENOM_FLEX_ROW;
    }
    return result;
}

VenomResultPtr venom_container_create_column(void) {
    VenomResultPtr result = venom_container_create();
    if (result.ok) {
        VenomContainer* c = (VenomContainer*)result.value;
        c->direction = VENOM_FLEX_COLUMN;
    }
    return result;
}

void venom_container_set_direction(VenomContainer* container, VenomFlexDirection direction) {
    if (!container) return;
    container->direction = direction;
    venom_widget_invalidate_layout((VenomWidget*)container);
}

void venom_container_set_justify(VenomContainer* container, VenomJustify justify) {
    if (!container) return;
    container->justify = justify;
    venom_widget_invalidate_layout((VenomWidget*)container);
}

void venom_container_set_align(VenomContainer* container, VenomAlign align) {
    if (!container) return;
    container->align_items = align;
    venom_widget_invalidate_layout((VenomWidget*)container);
}

void venom_container_set_gap(VenomContainer* container, VenomF32 gap) {
    if (!container) return;
    container->gap = gap;
    venom_widget_invalidate_layout((VenomWidget*)container);
}

void venom_container_set_background(VenomContainer* container, VenomColor color) {
    if (!container) return;
    container->bg_color = color;
    container->has_background = VENOM_TRUE;
    venom_widget_invalidate((VenomWidget*)container);
}

void venom_container_set_corner_radius(VenomContainer* container, VenomF32 radius) {
    if (!container) return;
    container->corner_radius = radius;
    venom_widget_invalidate((VenomWidget*)container);
}
