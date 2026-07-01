/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_breadcrumb.c - Breadcrumb implementation
 */

#include "vaxp/widgets/vaxp_breadcrumb.h"
#include "vaxp/core/vaxp_memory.h"
#include <string.h>

#define INITIAL_CAPACITY 8
#define DEFAULT_PADDING 8.0f

static void breadcrumb_init(VaxpWidget* widget) {
    VaxpBreadcrumb* bc = (VaxpBreadcrumb*)widget;
    
    bc->items = NULL;
    bc->item_count = 0;
    bc->item_capacity = 0;
    
    bc->separator = NULL;
    bc->hover_index = -1;
    bc->on_navigate = NULL;
    bc->callback_data = NULL;
    
    bc->text_color = (VaxpColor){ 63, 81, 181, 255 };
    bc->hover_color = (VaxpColor){ 48, 63, 159, 255 };
    bc->separator_color = (VaxpColor){ 158, 158, 158, 255 };
    bc->item_padding = DEFAULT_PADDING;
    
    /* Default separator */
    const char* def_sep = " / ";
    VaxpSize len = strlen(def_sep) + 1;
    bc->separator = (char*)vaxp_alloc(len);
    if (bc->separator) memcpy(bc->separator, def_sep, len);
}

static void breadcrumb_destroy(VaxpWidget* widget) {
    VaxpBreadcrumb* bc = (VaxpBreadcrumb*)widget;
    
    for (VaxpU32 i = 0; i < bc->item_count; i++) {
        if (bc->items[i].label) vaxp_free(bc->items[i].label, strlen(bc->items[i].label) + 1);
        if (bc->items[i].path) vaxp_free(bc->items[i].path, strlen(bc->items[i].path) + 1);
    }
    
    if (bc->items) vaxp_free(bc->items, bc->item_capacity * sizeof(VaxpBreadcrumbItem));
    if (bc->separator) vaxp_free(bc->separator, strlen(bc->separator) + 1);
    
    vaxp_widget_class.destroy(widget);
}

static void breadcrumb_measure(VaxpWidget* widget, VaxpF32 available_width, VaxpF32 available_height,
                               VaxpF32* out_width, VaxpF32* out_height) {
    VaxpBreadcrumb* bc = (VaxpBreadcrumb*)widget;
    (void)available_width; (void)available_height;
    
    VaxpF32 w = 0;
    VaxpF32 sep_w = bc->separator ? (VaxpF32)strlen(bc->separator) * 8 : 0;
    
    for (VaxpU32 i = 0; i < bc->item_count; i++) {
        if (bc->items[i].label) {
            w += (VaxpF32)strlen(bc->items[i].label) * 8 + bc->item_padding * 2;
        }
        if (i < bc->item_count - 1) w += sep_w;
    }
    
    *out_width = w;
    *out_height = 32.0f;
}

static void breadcrumb_draw(VaxpWidget* widget, VaxpCanvas* canvas) {
    VaxpBreadcrumb* bc = (VaxpBreadcrumb*)widget;
    
    VaxpF32 x = 0;
    VaxpF32 h = 32.0f;
    VaxpF32 sep_w = bc->separator ? (VaxpF32)strlen(bc->separator) * 8 : 0;
    
    for (VaxpU32 i = 0; i < bc->item_count; i++) {
        VaxpF32 item_w = bc->items[i].label ? (VaxpF32)strlen(bc->items[i].label) * 8 : 0;
        
        /* Draw item */
        VaxpBool is_last = (i == bc->item_count - 1);
        VaxpBool is_hover = ((VaxpI32)i == bc->hover_index);
        
        VaxpColor color = is_last ? (VaxpColor){ 33, 33, 33, 255 } : 
                           (is_hover ? bc->hover_color : bc->text_color);
        VaxpPaint text_paint = vaxp_paint_fill(color);
        
        if (bc->items[i].label) {
            vaxp_canvas_draw_text(canvas, bc->items[i].label, x + bc->item_padding, h / 2 + 5, NULL, &text_paint);
        }
        
        x += item_w + bc->item_padding * 2;
        
        /* Draw separator */
        if (!is_last && bc->separator) {
            VaxpPaint sep_paint = vaxp_paint_fill(bc->separator_color);
            vaxp_canvas_draw_text(canvas, bc->separator, x, h / 2 + 5, NULL, &sep_paint);
            x += sep_w;
        }
    }
}

static VaxpBool breadcrumb_on_event(VaxpWidget* widget, const VaxpEvent* event) {
    VaxpBreadcrumb* bc = (VaxpBreadcrumb*)widget;
    
    switch (event->type) {
        case VAXP_EVENT_MOUSE_MOVE: {
            VaxpF32 mx = (VaxpF32)event->mouse.x;
            VaxpF32 x = 0;
            VaxpF32 sep_w = bc->separator ? (VaxpF32)strlen(bc->separator) * 8 : 0;
            VaxpI32 new_hover = -1;
            
            for (VaxpU32 i = 0; i < bc->item_count; i++) {
                VaxpF32 item_w = bc->items[i].label ? (VaxpF32)strlen(bc->items[i].label) * 8 + bc->item_padding * 2 : 0;
                
                if (mx >= x && mx < x + item_w) {
                    new_hover = (VaxpI32)i;
                    break;
                }
                
                x += item_w;
                if (i < bc->item_count - 1) x += sep_w;
            }
            
            if (new_hover != bc->hover_index) {
                bc->hover_index = new_hover;
                widget->needs_redraw = VAXP_TRUE;
            }
            break;
        }
        
        case VAXP_EVENT_MOUSE_BUTTON_DOWN:
            if (event->mouse.button == VAXP_MOUSE_BUTTON_LEFT && 
                bc->hover_index >= 0 && bc->hover_index < (VaxpI32)bc->item_count - 1) {
                if (bc->on_navigate) {
                    bc->on_navigate(bc, (VaxpU32)bc->hover_index, 
                                    bc->items[bc->hover_index].path, bc->callback_data);
                }
                return VAXP_TRUE;
            }
            break;
            
        case VAXP_EVENT_MOUSE_LEAVE:
            bc->hover_index = -1;
            widget->needs_redraw = VAXP_TRUE;
            break;
            
        default:
            break;
    }
    
    return VAXP_FALSE;
}

const VaxpWidgetClass vaxp_breadcrumb_class = {
    .class_name = "VaxpBreadcrumb",
    .instance_size = sizeof(VaxpBreadcrumb),
    .parent_class = &vaxp_widget_class,
    .init = breadcrumb_init,
    .destroy = breadcrumb_destroy,
    .measure = breadcrumb_measure,
    .layout = NULL,
    .draw = breadcrumb_draw,
    .on_event = breadcrumb_on_event,
    .on_state_changed = NULL,
};

VaxpResultPtr vaxp_breadcrumb_create(void) {
    return vaxp_widget_create(&vaxp_breadcrumb_class);
}

VaxpResult vaxp_breadcrumb_add_item(VaxpBreadcrumb* bc, const char* label, const char* path) {
    VAXP_ENSURE_NOT_NULL(bc);
    
    if (bc->item_count >= bc->item_capacity) {
        VaxpU32 new_cap = bc->item_capacity == 0 ? INITIAL_CAPACITY : bc->item_capacity * 2;
        VaxpBreadcrumbItem* new_items = (VaxpBreadcrumbItem*)vaxp_alloc(new_cap * sizeof(VaxpBreadcrumbItem));
        if (!new_items) return VAXP_ERR_UNIT(VAXP_ERROR_OUT_OF_MEMORY);
        
        if (bc->items) {
            memcpy(new_items, bc->items, bc->item_count * sizeof(VaxpBreadcrumbItem));
            vaxp_free(bc->items, bc->item_capacity * sizeof(VaxpBreadcrumbItem));
        }
        
        bc->items = new_items;
        bc->item_capacity = new_cap;
    }
    
    VaxpBreadcrumbItem* item = &bc->items[bc->item_count];
    memset(item, 0, sizeof(VaxpBreadcrumbItem));
    
    if (label) {
        VaxpSize len = strlen(label) + 1;
        item->label = (char*)vaxp_alloc(len);
        if (item->label) memcpy(item->label, label, len);
    }
    
    if (path) {
        VaxpSize len = strlen(path) + 1;
        item->path = (char*)vaxp_alloc(len);
        if (item->path) memcpy(item->path, path, len);
    }
    
    bc->item_count++;
    return VAXP_OK_UNIT();
}

void vaxp_breadcrumb_pop(VaxpBreadcrumb* bc) {
    if (bc && bc->item_count > 0) {
        bc->item_count--;
        VaxpBreadcrumbItem* item = &bc->items[bc->item_count];
        if (item->label) vaxp_free(item->label, strlen(item->label) + 1);
        if (item->path) vaxp_free(item->path, strlen(item->path) + 1);
        vaxp_widget_invalidate((VaxpWidget*)bc);
    }
}

void vaxp_breadcrumb_clear(VaxpBreadcrumb* bc) {
    if (!bc) return;
    
    for (VaxpU32 i = 0; i < bc->item_count; i++) {
        if (bc->items[i].label) vaxp_free(bc->items[i].label, strlen(bc->items[i].label) + 1);
        if (bc->items[i].path) vaxp_free(bc->items[i].path, strlen(bc->items[i].path) + 1);
    }
    bc->item_count = 0;
}

void vaxp_breadcrumb_set_separator(VaxpBreadcrumb* bc, const char* sep) {
    if (!bc) return;
    
    if (bc->separator) vaxp_free(bc->separator, strlen(bc->separator) + 1);
    bc->separator = NULL;
    
    if (sep) {
        VaxpSize len = strlen(sep) + 1;
        bc->separator = (char*)vaxp_alloc(len);
        if (bc->separator) memcpy(bc->separator, sep, len);
    }
}

void vaxp_breadcrumb_set_on_navigate(VaxpBreadcrumb* bc, VaxpBreadcrumbCallback callback, void* data) {
    if (bc) {
        bc->on_navigate = callback;
        bc->callback_data = data;
    }
}

void vaxp_breadcrumb_set_path(VaxpBreadcrumb* bc, const char* path, char delimiter) {
    if (!bc || !path) return;
    
    vaxp_breadcrumb_clear(bc);
    
    const char* start = path;
    const char* p = path;
    char cumulative[512] = {0};
    
    while (*p) {
        if (*p == delimiter || *(p + 1) == '\0') {
            VaxpSize len = (*p == delimiter) ? (VaxpSize)(p - start) : (VaxpSize)(p - start + 1);
            if (len > 0) {
                char label[256];
                if (len > 255) len = 255;
                memcpy(label, start, len);
                label[len] = '\0';
                
                if (cumulative[0]) {
                    VaxpSize cum_len = strlen(cumulative);
                    cumulative[cum_len] = delimiter;
                    memcpy(cumulative + cum_len + 1, label, len + 1);
                } else {
                    memcpy(cumulative, label, len + 1);
                }
                
                vaxp_breadcrumb_add_item(bc, label, cumulative);
            }
            start = p + 1;
        }
        p++;
    }
}
