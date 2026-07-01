/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_container.c - Container widget with flexbox layout
 */

#include "vaxp/widgets/vaxp_container.h"
#include "vaxp/core/vaxp_memory.h"

/* ============================================================================
 * CONTAINER CLASS METHODS
 * ============================================================================ */

static void container_init(VaxpWidget* widget) {
    VaxpContainer* c = (VaxpContainer*)widget;
    
    c->direction = VAXP_FLEX_ROW;
    c->justify = VAXP_JUSTIFY_START;
    c->align_items = VAXP_ALIGN_START;
    c->gap = 0;
    c->bg_color = VAXP_COLOR_TRANSPARENT;
    c->corner_radius = 0;
    c->has_background = VAXP_FALSE;
}

static void container_measure(VaxpWidget* widget, VaxpF32 available_width, VaxpF32 available_height,
                               VaxpF32* out_width, VaxpF32* out_height) {
    VaxpContainer* c = (VaxpContainer*)widget;
    VaxpBool is_row = (c->direction == VAXP_FLEX_ROW || c->direction == VAXP_FLEX_ROW_REVERSE);
    
    VaxpF32 main_axis = 0;
    VaxpF32 cross_axis = 0;
    VaxpU32 child_count = vaxp_widget_child_count(widget);
    
    for (VaxpU32 i = 0; i < child_count; i++) {
        VaxpWidget* child = vaxp_widget_child_at(widget, i);
        if (!child->visible) continue;
        
        VaxpF32 cw, ch;
        vaxp_widget_measure(child, available_width, available_height, &cw, &ch);
        
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
    VaxpF32 pad_h = widget->layout.padding.left + widget->layout.padding.right;
    VaxpF32 pad_v = widget->layout.padding.top + widget->layout.padding.bottom;
    
    if (is_row) {
        *out_width = main_axis + pad_h;
        *out_height = cross_axis + pad_v;
    } else {
        *out_width = cross_axis + pad_h;
        *out_height = main_axis + pad_v;
    }
}

static void container_layout(VaxpWidget* widget, VaxpRectF bounds) {
    /* First do base layout */
    vaxp_widget_class.layout(widget, bounds);
    
    VaxpContainer* c = (VaxpContainer*)widget;
    VaxpU32 child_count = vaxp_widget_child_count(widget);
    if (child_count == 0) return;
    
    VaxpBool is_row = (c->direction == VAXP_FLEX_ROW || c->direction == VAXP_FLEX_ROW_REVERSE);
    VaxpBool is_reverse = (c->direction == VAXP_FLEX_ROW_REVERSE || c->direction == VAXP_FLEX_COLUMN_REVERSE);
    
    VaxpRectF content = widget->content_rect;
    
    /* Measure children and calculate totals */
    VaxpF32 total_main = 0;
    VaxpF32 total_flex_grow = 0;
    VaxpU32 visible_count = 0;
    
    VaxpF32* sizes = (VaxpF32*)vaxp_alloc(sizeof(VaxpF32) * 2 * child_count);
    if (!sizes) return;
    VaxpF32* widths = sizes;
    VaxpF32* heights = sizes + child_count;
    
    for (VaxpU32 i = 0; i < child_count; i++) {
        VaxpWidget* child = vaxp_widget_child_at(widget, i);
        if (!child->visible) {
            widths[i] = heights[i] = 0;
            continue;
        }
        
        vaxp_widget_measure(child, content.width, content.height, &widths[i], &heights[i]);
        
        /* If child has flex_grow, don't count its measured size in total_main */
        /* It will get space from free_space distribution */
        if (child->layout.flex_grow > 0) {
            if (is_row) widths[i] = 0;  /* Will be filled by flex */
            else heights[i] = 0;
        }
        
        total_main += is_row ? widths[i] : heights[i];
        total_flex_grow += child->layout.flex_grow;
        visible_count++;
    }
    
    /* Calculate available space */
    VaxpF32 main_size = is_row ? content.width : content.height;
    VaxpF32 gap_space = visible_count > 1 ? c->gap * (visible_count - 1) : 0;
    VaxpF32 free_space = main_size - total_main - gap_space;
    
    /* Distribute remaining space to flex_grow widgets */
    if (total_flex_grow > 0 && free_space > 0) {
        VaxpF32 per_grow = free_space / total_flex_grow;
        for (VaxpU32 i = 0; i < child_count; i++) {
            VaxpWidget* child = vaxp_widget_child_at(widget, i);
            if (!child->visible) continue;
            
            if (child->layout.flex_grow > 0) {
                VaxpF32 size = per_grow * child->layout.flex_grow;
                if (is_row) {
                    widths[i] = size;
                } else {
                    heights[i] = size;
                }
            }
        }
    }
    
    /* Calculate starting position based on justify */
    VaxpF32 pos = is_row ? content.x : content.y;
    VaxpF32 spacing = 0;
    
    switch (c->justify) {
        case VAXP_JUSTIFY_CENTER:
            pos += free_space / 2;
            break;
        case VAXP_JUSTIFY_END:
            pos += free_space;
            break;
        case VAXP_JUSTIFY_SPACE_BETWEEN:
            if (visible_count > 1) {
                spacing = free_space / (visible_count - 1);
            }
            break;
        case VAXP_JUSTIFY_SPACE_AROUND:
            spacing = free_space / visible_count;
            pos += spacing / 2;
            break;
        case VAXP_JUSTIFY_SPACE_EVENLY:
            spacing = free_space / (visible_count + 1);
            pos += spacing;
            break;
        default:
            break;
    }
    
    /* Layout children */
    for (VaxpU32 i = 0; i < child_count; i++) {
        VaxpU32 idx = is_reverse ? (child_count - 1 - i) : i;
        VaxpWidget* child = vaxp_widget_child_at(widget, idx);
        if (!child->visible) continue;
        
        VaxpF32 w = widths[idx];
        VaxpF32 h = heights[idx];
        VaxpF32 x, y;
        
        if (is_row) {
            x = pos;
            pos += w + c->gap + spacing;
            
            /* Cross axis alignment */
            switch (c->align_items) {
                case VAXP_ALIGN_CENTER:
                    y = content.y + (content.height - h) / 2;
                    break;
                case VAXP_ALIGN_END:
                    y = content.y + content.height - h;
                    break;
                case VAXP_ALIGN_STRETCH:
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
                case VAXP_ALIGN_CENTER:
                    x = content.x + (content.width - w) / 2;
                    break;
                case VAXP_ALIGN_END:
                    x = content.x + content.width - w;
                    break;
                case VAXP_ALIGN_STRETCH:
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
        
        VaxpRectF child_bounds = { x, y, w, h };
        printf("[Container Layout] Child %u: (%s) at (%.0f,%.0f) size %.0fx%.0f\n", 
               idx, child->klass ? child->klass->class_name : "?", x, y, w, h);
        vaxp_widget_layout(child, child_bounds);
    }
    
    vaxp_free(sizes, sizeof(VaxpF32) * 2 * child_count);
}

static void container_draw(VaxpWidget* widget, VaxpCanvas* canvas) {
    VaxpContainer* c = (VaxpContainer*)widget;
    
    /* Draw background if set */
    if (c->has_background) {
        VaxpRectF rect = { 0, 0, widget->bounds.width, widget->bounds.height };
        VaxpPaint paint = vaxp_paint_fill(c->bg_color);
        
        if (c->corner_radius > 0) {
            vaxp_canvas_draw_rounded_rect(canvas, rect, c->corner_radius, &paint);
        } else {
            vaxp_canvas_draw_rect(canvas, rect, &paint);
        }
    }
    
    /* Children are drawn by the widget system after this */
}

static void container_destroy(VaxpWidget* widget) {
    /* Call base class destroy to free children */
    vaxp_widget_class.destroy(widget);
}

/* ============================================================================
 * CONTAINER CLASS
 * ============================================================================ */

const VaxpWidgetClass vaxp_container_class = {
    .class_name = "VaxpContainer",
    .instance_size = sizeof(VaxpContainer),
    .parent_class = &vaxp_widget_class,
    .init = container_init,
    .destroy = container_destroy,  /* Now properly calls base destroy */
    .measure = container_measure,
    .layout = container_layout,
    .draw = container_draw,
    .on_event = NULL,  /* Will use parent's at runtime */
    .on_state_changed = NULL,  /* Will use parent's at runtime */
};

/* ============================================================================
 * PUBLIC API
 * ============================================================================ */

VaxpResultPtr vaxp_container_create(void) {
    return vaxp_widget_create(&vaxp_container_class);
}

VaxpResultPtr vaxp_container_create_row(void) {
    VaxpResultPtr result = vaxp_container_create();
    if (result.ok) {
        VaxpContainer* c = (VaxpContainer*)result.value;
        c->direction = VAXP_FLEX_ROW;
    }
    return result;
}

VaxpResultPtr vaxp_container_create_column(void) {
    VaxpResultPtr result = vaxp_container_create();
    if (result.ok) {
        VaxpContainer* c = (VaxpContainer*)result.value;
        c->direction = VAXP_FLEX_COLUMN;
    }
    return result;
}

void vaxp_container_set_direction(VaxpContainer* container, VaxpFlexDirection direction) {
    if (!container) return;
    container->direction = direction;
    vaxp_widget_invalidate_layout((VaxpWidget*)container);
}

void vaxp_container_set_justify(VaxpContainer* container, VaxpJustify justify) {
    if (!container) return;
    container->justify = justify;
    vaxp_widget_invalidate_layout((VaxpWidget*)container);
}

void vaxp_container_set_align(VaxpContainer* container, VaxpAlign align) {
    if (!container) return;
    container->align_items = align;
    vaxp_widget_invalidate_layout((VaxpWidget*)container);
}

void vaxp_container_set_gap(VaxpContainer* container, VaxpF32 gap) {
    if (!container) return;
    container->gap = gap;
    vaxp_widget_invalidate_layout((VaxpWidget*)container);
}

void vaxp_container_set_background(VaxpContainer* container, VaxpColor color) {
    if (!container) return;
    container->bg_color = color;
    container->has_background = VAXP_TRUE;
    vaxp_widget_invalidate((VaxpWidget*)container);
}

void vaxp_container_set_corner_radius(VaxpContainer* container, VaxpF32 radius) {
    if (!container) return;
    container->corner_radius = radius;
    vaxp_widget_invalidate((VaxpWidget*)container);
}
