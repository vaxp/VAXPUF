/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_search_bar.c - Search input implementation
 */

#include "vaxp/widgets/vaxp_search_bar.h"
#include "vaxp/core/vaxp_memory.h"
#include <string.h>

#define INITIAL_CAPACITY 64
#define DEFAULT_HEIGHT 40.0f

static void search_bar_init(VaxpWidget* widget) {
    VaxpSearchBar* bar = (VaxpSearchBar*)widget;
    
    bar->text = (char*)vaxp_alloc(INITIAL_CAPACITY);
    if (bar->text) {
        bar->text[0] = '\0';
        bar->text_capacity = INITIAL_CAPACITY;
    }
    bar->text_len = 0;
    
    bar->placeholder = NULL;
    bar->cursor_pos = 0;
    bar->show_clear = VAXP_TRUE;
    
    bar->on_search = NULL;
    bar->on_change = NULL;
    bar->callback_data = NULL;
    
    bar->background_color = (VaxpColor){ 245, 245, 245, 255 };
    bar->border_color = (VaxpColor){ 224, 224, 224, 255 };
    bar->text_color = (VaxpColor){ 33, 33, 33, 255 };
    bar->placeholder_color = (VaxpColor){ 158, 158, 158, 255 };
    bar->icon_color = (VaxpColor){ 97, 97, 97, 255 };
    bar->corner_radius = 20.0f;
    bar->height = DEFAULT_HEIGHT;
    
    widget->focusable = VAXP_TRUE;
}

static void search_bar_destroy(VaxpWidget* widget) {
    VaxpSearchBar* bar = (VaxpSearchBar*)widget;
    
    if (bar->text) vaxp_free(bar->text, bar->text_capacity);
    if (bar->placeholder) vaxp_free(bar->placeholder, strlen(bar->placeholder) + 1);
    
    vaxp_widget_class.destroy(widget);
}

static void search_bar_measure(VaxpWidget* widget, VaxpF32 available_width, VaxpF32 available_height,
                               VaxpF32* out_width, VaxpF32* out_height) {
    VaxpSearchBar* bar = (VaxpSearchBar*)widget;
    (void)available_height;
    
    *out_width = widget->layout.preferred_width > 0 ? widget->layout.preferred_width : available_width;
    *out_height = bar->height;
}

static void search_bar_draw(VaxpWidget* widget, VaxpCanvas* canvas) {
    VaxpSearchBar* bar = (VaxpSearchBar*)widget;
    
    VaxpF32 w = widget->bounds.width;
    VaxpF32 h = bar->height;
    
    /* Draw background */
    VaxpRectF bg = { 0, 0, w, h };
    VaxpPaint bg_paint = vaxp_paint_fill(bar->background_color);
    vaxp_canvas_draw_rounded_rect(canvas, bg, bar->corner_radius, &bg_paint);
    
    /* Draw border if focused */
    if (widget->state & VAXP_WIDGET_STATE_FOCUSED) {
        VaxpPaint border_paint = vaxp_paint_stroke((VaxpColor){ 63, 81, 181, 255 }, 2.0f);
        vaxp_canvas_draw_rounded_rect(canvas, bg, bar->corner_radius, &border_paint);
    }
    
    /* Draw search icon */
    VaxpPaint icon_paint = vaxp_paint_fill(bar->icon_color);
    vaxp_canvas_draw_text(canvas, "🔍", 16, h / 2 + 5, NULL, &icon_paint);
    
    /* Draw text or placeholder */
    VaxpF32 text_x = 44;
    VaxpF32 text_y = h / 2 + 5;
    
    if (bar->text && bar->text[0]) {
        VaxpPaint text_paint = vaxp_paint_fill(bar->text_color);
        vaxp_canvas_draw_text(canvas, bar->text, text_x, text_y, NULL, &text_paint);
        
        /* Clear button */
        if (bar->show_clear) {
            VaxpPaint clear_paint = vaxp_paint_fill(bar->icon_color);
            vaxp_canvas_draw_text(canvas, "✕", w - 30, text_y, NULL, &clear_paint);
        }
    } else if (bar->placeholder) {
        VaxpPaint ph_paint = vaxp_paint_fill(bar->placeholder_color);
        vaxp_canvas_draw_text(canvas, bar->placeholder, text_x, text_y, NULL, &ph_paint);
    }
    
    /* Draw cursor if focused */
    if (widget->state & VAXP_WIDGET_STATE_FOCUSED) {
        VaxpF32 cursor_x = text_x + bar->cursor_pos * 8;
        VaxpPaint cursor_paint = vaxp_paint_fill((VaxpColor){ 63, 81, 181, 255 });
        VaxpRectF cursor = { cursor_x, h / 2 - 10, 2, 20 };
        vaxp_canvas_draw_rect(canvas, cursor, &cursor_paint);
    }
}

static VaxpBool search_bar_on_event(VaxpWidget* widget, const VaxpEvent* event) {
    VaxpSearchBar* bar = (VaxpSearchBar*)widget;
    
    switch (event->type) {
        case VAXP_EVENT_MOUSE_BUTTON_DOWN:
            /* Set focus on click */
            widget->state |= VAXP_WIDGET_STATE_FOCUSED;
            widget->needs_redraw = VAXP_TRUE;
            
            /* Check clear button */
            if (bar->text && bar->text[0] && bar->show_clear) {
                if (event->mouse.x > widget->bounds.width - 40) {
                    vaxp_search_bar_clear(bar);
                    return VAXP_TRUE;
                }
            }
            return VAXP_TRUE;
            
        case VAXP_EVENT_KEY_DOWN:
            if (event->key.key == VAXP_KEY_RETURN && bar->on_search) {
                bar->on_search(bar, bar->text, bar->callback_data);
                return VAXP_TRUE;
            }
            
            if (event->key.key == VAXP_KEY_BACKSPACE && bar->cursor_pos > 0) {
                bar->cursor_pos--;
                memmove(bar->text + bar->cursor_pos, bar->text + bar->cursor_pos + 1, 
                        bar->text_len - bar->cursor_pos);
                bar->text_len--;
                widget->needs_redraw = VAXP_TRUE;
                
                if (bar->on_change) bar->on_change(bar, bar->text, bar->callback_data);
                return VAXP_TRUE;
            }
            
            /* Handle UTF-8 text input from XIM (supports all languages) */
            if (event->text.text[0] != '\0' && (unsigned char)event->text.text[0] >= 32) {
                const char* input = event->text.text;
                VaxpSize len = strlen(input);
                
                /* Ensure we have space (UTF-8 can be up to 4 bytes per char) */
                if (bar->text_len + len < bar->text_capacity - 4) {
                    memmove(bar->text + bar->cursor_pos + len, bar->text + bar->cursor_pos,
                            bar->text_len - bar->cursor_pos + 1);
                    memcpy(bar->text + bar->cursor_pos, input, len);
                    bar->cursor_pos += len;
                    bar->text_len += len;
                    widget->needs_redraw = VAXP_TRUE;
                    
                    if (bar->on_change) bar->on_change(bar, bar->text, bar->callback_data);
                    return VAXP_TRUE;
                }
            }
            break;
        
        default:
            break;
    }
    
    return VAXP_FALSE;
}

const VaxpWidgetClass vaxp_search_bar_class = {
    .class_name = "VaxpSearchBar",
    .instance_size = sizeof(VaxpSearchBar),
    .parent_class = &vaxp_widget_class,
    .init = search_bar_init,
    .destroy = search_bar_destroy,
    .measure = search_bar_measure,
    .layout = NULL,
    .draw = search_bar_draw,
    .on_event = search_bar_on_event,
    .on_state_changed = NULL,
};

VaxpResultPtr vaxp_search_bar_create(void) {
    return vaxp_widget_create(&vaxp_search_bar_class);
}

void vaxp_search_bar_set_text(VaxpSearchBar* bar, const char* text) {
    if (!bar) return;
    
    VaxpSize len = text ? strlen(text) : 0;
    if (len >= bar->text_capacity) {
        char* new_buf = (char*)vaxp_alloc(len + 1);
        if (!new_buf) return;
        vaxp_free(bar->text, bar->text_capacity);
        bar->text = new_buf;
        bar->text_capacity = len + 1;
    }
    
    if (text) memcpy(bar->text, text, len);
    bar->text[len] = '\0';
    bar->text_len = len;
    bar->cursor_pos = len;
    vaxp_widget_invalidate((VaxpWidget*)bar);
}

const char* vaxp_search_bar_get_text(const VaxpSearchBar* bar) {
    return bar ? bar->text : NULL;
}

void vaxp_search_bar_set_placeholder(VaxpSearchBar* bar, const char* ph) {
    if (!bar) return;
    
    if (bar->placeholder) vaxp_free(bar->placeholder, strlen(bar->placeholder) + 1);
    bar->placeholder = NULL;
    
    if (ph) {
        VaxpSize len = strlen(ph) + 1;
        bar->placeholder = (char*)vaxp_alloc(len);
        if (bar->placeholder) memcpy(bar->placeholder, ph, len);
    }
}

void vaxp_search_bar_set_on_search(VaxpSearchBar* bar, VaxpSearchCallback callback, void* data) {
    if (bar) {
        bar->on_search = callback;
        bar->callback_data = data;
    }
}

void vaxp_search_bar_set_on_change(VaxpSearchBar* bar, VaxpSearchCallback callback, void* data) {
    if (bar) {
        bar->on_change = callback;
    }
}

void vaxp_search_bar_clear(VaxpSearchBar* bar) {
    if (bar && bar->text) {
        bar->text[0] = '\0';
        bar->text_len = 0;
        bar->cursor_pos = 0;
        vaxp_widget_invalidate((VaxpWidget*)bar);
        
        if (bar->on_change) bar->on_change(bar, bar->text, bar->callback_data);
    }
}

VaxpWidget* _vaxp_search_bar_build(const VaxpSearchBarConfig* config) {
    VaxpResultPtr result = vaxp_search_bar_create();
    if (!result.ok) return NULL;
    
    VaxpSearchBar* bar = (VaxpSearchBar*)result.value;
    
    if (config->placeholder) vaxp_search_bar_set_placeholder(bar, config->placeholder);
    bar->on_search = config->on_search;
    bar->callback_data = config->data;
    
    return (VaxpWidget*)bar;
}
