/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_link.c - Clickable hyperlink implementation
 */

#include "venom/widgets/venom_link.h"
#include "venom/core/venom_memory.h"
#include <string.h>

static void link_init(VenomWidget* widget) {
    VenomLink* link = (VenomLink*)widget;
    
    link->text = NULL;
    link->url = NULL;
    link->visited = VENOM_FALSE;
    link->underline_on_hover = VENOM_TRUE;
    
    link->on_click = NULL;
    link->callback_data = NULL;
    
    link->normal_color = (VenomColor){ 25, 118, 210, 255 };
    link->visited_color = (VenomColor){ 156, 39, 176, 255 };
    link->hover_color = (VenomColor){ 21, 101, 192, 255 };
    link->hovering = VENOM_FALSE;
    
    widget->focusable = VENOM_TRUE;
}

static void link_destroy(VenomWidget* widget) {
    VenomLink* link = (VenomLink*)widget;
    if (link->text) venom_free(link->text, strlen(link->text) + 1);
    if (link->url) venom_free(link->url, strlen(link->url) + 1);
    venom_widget_class.destroy(widget);
}

static void link_measure(VenomWidget* widget, VenomF32 available_width, VenomF32 available_height,
                         VenomF32* out_width, VenomF32* out_height) {
    VenomLink* link = (VenomLink*)widget;
    (void)available_width; (void)available_height;
    
    *out_width = link->text ? (VenomF32)strlen(link->text) * 8 : 0;
    *out_height = 20;
}

static void link_draw(VenomWidget* widget, VenomCanvas* canvas) {
    VenomLink* link = (VenomLink*)widget;
    
    if (!link->text) return;
    
    VenomColor color;
    if (link->hovering) {
        color = link->hover_color;
    } else if (link->visited) {
        color = link->visited_color;
    } else {
        color = link->normal_color;
    }
    
    VenomPaint text_paint = venom_paint_fill(color);
    venom_canvas_draw_text(canvas, link->text, 0, 15, NULL, &text_paint);
    
    /* Draw underline */
    if (link->hovering || !link->underline_on_hover) {
        VenomF32 w = (VenomF32)strlen(link->text) * 8;
        VenomPaint underline = venom_paint_stroke(color, 1.0f);
        venom_canvas_draw_line(canvas, 0, 18, w, 18, &underline);
    }
}

static VenomBool link_on_event(VenomWidget* widget, const VenomEvent* event) {
    VenomLink* link = (VenomLink*)widget;
    
    switch (event->type) {
        case VENOM_EVENT_MOUSE_ENTER:
            link->hovering = VENOM_TRUE;
            widget->needs_redraw = VENOM_TRUE;
            break;
            
        case VENOM_EVENT_MOUSE_LEAVE:
            link->hovering = VENOM_FALSE;
            widget->needs_redraw = VENOM_TRUE;
            break;
            
        case VENOM_EVENT_MOUSE_BUTTON_DOWN:
            if (event->mouse.button == VENOM_MOUSE_BUTTON_LEFT) {
                link->visited = VENOM_TRUE;
                widget->needs_redraw = VENOM_TRUE;
                
                if (link->on_click) {
                    link->on_click(link, link->url, link->callback_data);
                }
                return VENOM_TRUE;
            }
            break;
            
        default:
            break;
    }
    
    return VENOM_FALSE;
}

const VenomWidgetClass venom_link_class = {
    .class_name = "VenomLink",
    .instance_size = sizeof(VenomLink),
    .parent_class = &venom_widget_class,
    .init = link_init,
    .destroy = link_destroy,
    .measure = link_measure,
    .layout = NULL,
    .draw = link_draw,
    .on_event = link_on_event,
    .on_state_changed = NULL,
};

VenomResultPtr venom_link_create(void) {
    return venom_widget_create(&venom_link_class);
}

static void set_string(char** dest, const char* src) {
    if (*dest) {
        venom_free(*dest, strlen(*dest) + 1);
        *dest = NULL;
    }
    if (src) {
        VenomSize len = strlen(src) + 1;
        *dest = (char*)venom_alloc(len);
        if (*dest) memcpy(*dest, src, len);
    }
}

void venom_link_set_text(VenomLink* link, const char* text) {
    if (link) set_string(&link->text, text);
}

void venom_link_set_url(VenomLink* link, const char* url) {
    if (link) set_string(&link->url, url);
}

void venom_link_set_on_click(VenomLink* link, VenomLinkCallback callback, void* data) {
    if (link) {
        link->on_click = callback;
        link->callback_data = data;
    }
}

VenomWidget* _venom_link_build(const VenomLinkConfig* config) {
    VenomResultPtr result = venom_link_create();
    if (!result.ok) return NULL;
    
    VenomLink* link = (VenomLink*)result.value;
    
    if (config->text) venom_link_set_text(link, config->text);
    if (config->url) venom_link_set_url(link, config->url);
    link->on_click = config->on_click;
    link->callback_data = config->data;
    
    return (VenomWidget*)link;
}
