/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_breadcrumb.c - Breadcrumb implementation
 */

#include "venom/widgets/venom_breadcrumb.h"
#include "venom/core/venom_memory.h"
#include <string.h>

#define INITIAL_CAPACITY 8
#define DEFAULT_PADDING 8.0f

static void breadcrumb_init(VenomWidget* widget) {
    VenomBreadcrumb* bc = (VenomBreadcrumb*)widget;
    
    bc->items = NULL;
    bc->item_count = 0;
    bc->item_capacity = 0;
    
    bc->separator = NULL;
    bc->hover_index = -1;
    bc->on_navigate = NULL;
    bc->callback_data = NULL;
    
    bc->text_color = (VenomColor){ 63, 81, 181, 255 };
    bc->hover_color = (VenomColor){ 48, 63, 159, 255 };
    bc->separator_color = (VenomColor){ 158, 158, 158, 255 };
    bc->item_padding = DEFAULT_PADDING;
    
    /* Default separator */
    const char* def_sep = " / ";
    VenomSize len = strlen(def_sep) + 1;
    bc->separator = (char*)venom_alloc(len);
    if (bc->separator) memcpy(bc->separator, def_sep, len);
}

static void breadcrumb_destroy(VenomWidget* widget) {
    VenomBreadcrumb* bc = (VenomBreadcrumb*)widget;
    
    for (VenomU32 i = 0; i < bc->item_count; i++) {
        if (bc->items[i].label) venom_free(bc->items[i].label, strlen(bc->items[i].label) + 1);
        if (bc->items[i].path) venom_free(bc->items[i].path, strlen(bc->items[i].path) + 1);
    }
    
    if (bc->items) venom_free(bc->items, bc->item_capacity * sizeof(VenomBreadcrumbItem));
    if (bc->separator) venom_free(bc->separator, strlen(bc->separator) + 1);
    
    venom_widget_class.destroy(widget);
}

static void breadcrumb_measure(VenomWidget* widget, VenomF32 available_width, VenomF32 available_height,
                               VenomF32* out_width, VenomF32* out_height) {
    VenomBreadcrumb* bc = (VenomBreadcrumb*)widget;
    (void)available_width; (void)available_height;
    
    VenomF32 w = 0;
    VenomF32 sep_w = bc->separator ? (VenomF32)strlen(bc->separator) * 8 : 0;
    
    for (VenomU32 i = 0; i < bc->item_count; i++) {
        if (bc->items[i].label) {
            w += (VenomF32)strlen(bc->items[i].label) * 8 + bc->item_padding * 2;
        }
        if (i < bc->item_count - 1) w += sep_w;
    }
    
    *out_width = w;
    *out_height = 32.0f;
}

static void breadcrumb_draw(VenomWidget* widget, VenomCanvas* canvas) {
    VenomBreadcrumb* bc = (VenomBreadcrumb*)widget;
    
    VenomF32 x = 0;
    VenomF32 h = 32.0f;
    VenomF32 sep_w = bc->separator ? (VenomF32)strlen(bc->separator) * 8 : 0;
    
    for (VenomU32 i = 0; i < bc->item_count; i++) {
        VenomF32 item_w = bc->items[i].label ? (VenomF32)strlen(bc->items[i].label) * 8 : 0;
        
        /* Draw item */
        VenomBool is_last = (i == bc->item_count - 1);
        VenomBool is_hover = ((VenomI32)i == bc->hover_index);
        
        VenomColor color = is_last ? (VenomColor){ 33, 33, 33, 255 } : 
                           (is_hover ? bc->hover_color : bc->text_color);
        VenomPaint text_paint = venom_paint_fill(color);
        
        if (bc->items[i].label) {
            venom_canvas_draw_text(canvas, bc->items[i].label, x + bc->item_padding, h / 2 + 5, NULL, &text_paint);
        }
        
        x += item_w + bc->item_padding * 2;
        
        /* Draw separator */
        if (!is_last && bc->separator) {
            VenomPaint sep_paint = venom_paint_fill(bc->separator_color);
            venom_canvas_draw_text(canvas, bc->separator, x, h / 2 + 5, NULL, &sep_paint);
            x += sep_w;
        }
    }
}

static VenomBool breadcrumb_on_event(VenomWidget* widget, const VenomEvent* event) {
    VenomBreadcrumb* bc = (VenomBreadcrumb*)widget;
    
    switch (event->type) {
        case VENOM_EVENT_MOUSE_MOVE: {
            VenomF32 mx = (VenomF32)event->mouse.x;
            VenomF32 x = 0;
            VenomF32 sep_w = bc->separator ? (VenomF32)strlen(bc->separator) * 8 : 0;
            VenomI32 new_hover = -1;
            
            for (VenomU32 i = 0; i < bc->item_count; i++) {
                VenomF32 item_w = bc->items[i].label ? (VenomF32)strlen(bc->items[i].label) * 8 + bc->item_padding * 2 : 0;
                
                if (mx >= x && mx < x + item_w) {
                    new_hover = (VenomI32)i;
                    break;
                }
                
                x += item_w;
                if (i < bc->item_count - 1) x += sep_w;
            }
            
            if (new_hover != bc->hover_index) {
                bc->hover_index = new_hover;
                widget->needs_redraw = VENOM_TRUE;
            }
            break;
        }
        
        case VENOM_EVENT_MOUSE_BUTTON_DOWN:
            if (event->mouse.button == VENOM_MOUSE_BUTTON_LEFT && 
                bc->hover_index >= 0 && bc->hover_index < (VenomI32)bc->item_count - 1) {
                if (bc->on_navigate) {
                    bc->on_navigate(bc, (VenomU32)bc->hover_index, 
                                    bc->items[bc->hover_index].path, bc->callback_data);
                }
                return VENOM_TRUE;
            }
            break;
            
        case VENOM_EVENT_MOUSE_LEAVE:
            bc->hover_index = -1;
            widget->needs_redraw = VENOM_TRUE;
            break;
            
        default:
            break;
    }
    
    return VENOM_FALSE;
}

const VenomWidgetClass venom_breadcrumb_class = {
    .class_name = "VenomBreadcrumb",
    .instance_size = sizeof(VenomBreadcrumb),
    .parent_class = &venom_widget_class,
    .init = breadcrumb_init,
    .destroy = breadcrumb_destroy,
    .measure = breadcrumb_measure,
    .layout = NULL,
    .draw = breadcrumb_draw,
    .on_event = breadcrumb_on_event,
    .on_state_changed = NULL,
};

VenomResultPtr venom_breadcrumb_create(void) {
    return venom_widget_create(&venom_breadcrumb_class);
}

VenomResult venom_breadcrumb_add_item(VenomBreadcrumb* bc, const char* label, const char* path) {
    VENOM_ENSURE_NOT_NULL(bc);
    
    if (bc->item_count >= bc->item_capacity) {
        VenomU32 new_cap = bc->item_capacity == 0 ? INITIAL_CAPACITY : bc->item_capacity * 2;
        VenomBreadcrumbItem* new_items = (VenomBreadcrumbItem*)venom_alloc(new_cap * sizeof(VenomBreadcrumbItem));
        if (!new_items) return VENOM_ERR_UNIT(VENOM_ERROR_OUT_OF_MEMORY);
        
        if (bc->items) {
            memcpy(new_items, bc->items, bc->item_count * sizeof(VenomBreadcrumbItem));
            venom_free(bc->items, bc->item_capacity * sizeof(VenomBreadcrumbItem));
        }
        
        bc->items = new_items;
        bc->item_capacity = new_cap;
    }
    
    VenomBreadcrumbItem* item = &bc->items[bc->item_count];
    memset(item, 0, sizeof(VenomBreadcrumbItem));
    
    if (label) {
        VenomSize len = strlen(label) + 1;
        item->label = (char*)venom_alloc(len);
        if (item->label) memcpy(item->label, label, len);
    }
    
    if (path) {
        VenomSize len = strlen(path) + 1;
        item->path = (char*)venom_alloc(len);
        if (item->path) memcpy(item->path, path, len);
    }
    
    bc->item_count++;
    return VENOM_OK_UNIT();
}

void venom_breadcrumb_pop(VenomBreadcrumb* bc) {
    if (bc && bc->item_count > 0) {
        bc->item_count--;
        VenomBreadcrumbItem* item = &bc->items[bc->item_count];
        if (item->label) venom_free(item->label, strlen(item->label) + 1);
        if (item->path) venom_free(item->path, strlen(item->path) + 1);
        venom_widget_invalidate((VenomWidget*)bc);
    }
}

void venom_breadcrumb_clear(VenomBreadcrumb* bc) {
    if (!bc) return;
    
    for (VenomU32 i = 0; i < bc->item_count; i++) {
        if (bc->items[i].label) venom_free(bc->items[i].label, strlen(bc->items[i].label) + 1);
        if (bc->items[i].path) venom_free(bc->items[i].path, strlen(bc->items[i].path) + 1);
    }
    bc->item_count = 0;
}

void venom_breadcrumb_set_separator(VenomBreadcrumb* bc, const char* sep) {
    if (!bc) return;
    
    if (bc->separator) venom_free(bc->separator, strlen(bc->separator) + 1);
    bc->separator = NULL;
    
    if (sep) {
        VenomSize len = strlen(sep) + 1;
        bc->separator = (char*)venom_alloc(len);
        if (bc->separator) memcpy(bc->separator, sep, len);
    }
}

void venom_breadcrumb_set_on_navigate(VenomBreadcrumb* bc, VenomBreadcrumbCallback callback, void* data) {
    if (bc) {
        bc->on_navigate = callback;
        bc->callback_data = data;
    }
}

void venom_breadcrumb_set_path(VenomBreadcrumb* bc, const char* path, char delimiter) {
    if (!bc || !path) return;
    
    venom_breadcrumb_clear(bc);
    
    const char* start = path;
    const char* p = path;
    char cumulative[512] = {0};
    
    while (*p) {
        if (*p == delimiter || *(p + 1) == '\0') {
            VenomSize len = (*p == delimiter) ? (VenomSize)(p - start) : (VenomSize)(p - start + 1);
            if (len > 0) {
                char label[256];
                if (len > 255) len = 255;
                memcpy(label, start, len);
                label[len] = '\0';
                
                if (cumulative[0]) {
                    VenomSize cum_len = strlen(cumulative);
                    cumulative[cum_len] = delimiter;
                    memcpy(cumulative + cum_len + 1, label, len + 1);
                } else {
                    memcpy(cumulative, label, len + 1);
                }
                
                venom_breadcrumb_add_item(bc, label, cumulative);
            }
            start = p + 1;
        }
        p++;
    }
}
