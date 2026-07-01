/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_link.c - Clickable hyperlink implementation
 */

#include "vaxp/widgets/vaxp_link.h"
#include "vaxp/core/vaxp_memory.h"
#include <string.h>

static void link_init(VaxpWidget* widget) {
    VaxpLink* link = (VaxpLink*)widget;
    
    link->text = NULL;
    link->url = NULL;
    link->visited = VAXP_FALSE;
    link->underline_on_hover = VAXP_TRUE;
    
    link->on_click = NULL;
    link->callback_data = NULL;
    
    link->normal_color = (VaxpColor){ 25, 118, 210, 255 };
    link->visited_color = (VaxpColor){ 156, 39, 176, 255 };
    link->hover_color = (VaxpColor){ 21, 101, 192, 255 };
    link->hovering = VAXP_FALSE;
    
    widget->focusable = VAXP_TRUE;
}

static void link_destroy(VaxpWidget* widget) {
    VaxpLink* link = (VaxpLink*)widget;
    if (link->text) vaxp_free(link->text, strlen(link->text) + 1);
    if (link->url) vaxp_free(link->url, strlen(link->url) + 1);
    vaxp_widget_class.destroy(widget);
}

static void link_measure(VaxpWidget* widget, VaxpF32 available_width, VaxpF32 available_height,
                         VaxpF32* out_width, VaxpF32* out_height) {
    VaxpLink* link = (VaxpLink*)widget;
    (void)available_width; (void)available_height;
    
    *out_width = link->text ? (VaxpF32)strlen(link->text) * 8 : 0;
    *out_height = 20;
}

static void link_draw(VaxpWidget* widget, VaxpCanvas* canvas) {
    VaxpLink* link = (VaxpLink*)widget;
    
    if (!link->text) return;
    
    VaxpColor color;
    if (link->hovering) {
        color = link->hover_color;
    } else if (link->visited) {
        color = link->visited_color;
    } else {
        color = link->normal_color;
    }
    
    VaxpPaint text_paint = vaxp_paint_fill(color);
    vaxp_canvas_draw_text(canvas, link->text, 0, 15, NULL, &text_paint);
    
    /* Draw underline */
    if (link->hovering || !link->underline_on_hover) {
        VaxpF32 w = (VaxpF32)strlen(link->text) * 8;
        VaxpPaint underline = vaxp_paint_stroke(color, 1.0f);
        vaxp_canvas_draw_line(canvas, 0, 18, w, 18, &underline);
    }
}

static VaxpBool link_on_event(VaxpWidget* widget, const VaxpEvent* event) {
    VaxpLink* link = (VaxpLink*)widget;
    
    switch (event->type) {
        case VAXP_EVENT_MOUSE_ENTER:
            link->hovering = VAXP_TRUE;
            widget->needs_redraw = VAXP_TRUE;
            break;
            
        case VAXP_EVENT_MOUSE_LEAVE:
            link->hovering = VAXP_FALSE;
            widget->needs_redraw = VAXP_TRUE;
            break;
            
        case VAXP_EVENT_MOUSE_BUTTON_DOWN:
            if (event->mouse.button == VAXP_MOUSE_BUTTON_LEFT) {
                link->visited = VAXP_TRUE;
                widget->needs_redraw = VAXP_TRUE;
                
                if (link->on_click) {
                    link->on_click(link, link->url, link->callback_data);
                }
                return VAXP_TRUE;
            }
            break;
            
        default:
            break;
    }
    
    return VAXP_FALSE;
}

const VaxpWidgetClass vaxp_link_class = {
    .class_name = "VaxpLink",
    .instance_size = sizeof(VaxpLink),
    .parent_class = &vaxp_widget_class,
    .init = link_init,
    .destroy = link_destroy,
    .measure = link_measure,
    .layout = NULL,
    .draw = link_draw,
    .on_event = link_on_event,
    .on_state_changed = NULL,
};

VaxpResultPtr vaxp_link_create(void) {
    return vaxp_widget_create(&vaxp_link_class);
}

static void set_string(char** dest, const char* src) {
    if (*dest) {
        vaxp_free(*dest, strlen(*dest) + 1);
        *dest = NULL;
    }
    if (src) {
        VaxpSize len = strlen(src) + 1;
        *dest = (char*)vaxp_alloc(len);
        if (*dest) memcpy(*dest, src, len);
    }
}

void vaxp_link_set_text(VaxpLink* link, const char* text) {
    if (link) set_string(&link->text, text);
}

void vaxp_link_set_url(VaxpLink* link, const char* url) {
    if (link) set_string(&link->url, url);
}

void vaxp_link_set_on_click(VaxpLink* link, VaxpLinkCallback callback, void* data) {
    if (link) {
        link->on_click = callback;
        link->callback_data = data;
    }
}

VaxpWidget* _vaxp_link_build(const VaxpLinkConfig* config) {
    VaxpResultPtr result = vaxp_link_create();
    if (!result.ok) return NULL;
    
    VaxpLink* link = (VaxpLink*)result.value;
    
    if (config->text) vaxp_link_set_text(link, config->text);
    if (config->url) vaxp_link_set_url(link, config->url);
    link->on_click = config->on_click;
    link->callback_data = config->data;
    
    return (VaxpWidget*)link;
}
