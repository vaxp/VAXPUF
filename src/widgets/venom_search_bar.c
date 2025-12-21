/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_search_bar.c - Search input implementation
 */

#include "venom/widgets/venom_search_bar.h"
#include "venom/core/venom_memory.h"
#include <string.h>

#define INITIAL_CAPACITY 64
#define DEFAULT_HEIGHT 40.0f

static void search_bar_init(VenomWidget* widget) {
    VenomSearchBar* bar = (VenomSearchBar*)widget;
    
    bar->text = (char*)venom_alloc(INITIAL_CAPACITY);
    if (bar->text) {
        bar->text[0] = '\0';
        bar->text_capacity = INITIAL_CAPACITY;
    }
    bar->text_len = 0;
    
    bar->placeholder = NULL;
    bar->cursor_pos = 0;
    bar->show_clear = VENOM_TRUE;
    
    bar->on_search = NULL;
    bar->on_change = NULL;
    bar->callback_data = NULL;
    
    bar->background_color = (VenomColor){ 245, 245, 245, 255 };
    bar->border_color = (VenomColor){ 224, 224, 224, 255 };
    bar->text_color = (VenomColor){ 33, 33, 33, 255 };
    bar->placeholder_color = (VenomColor){ 158, 158, 158, 255 };
    bar->icon_color = (VenomColor){ 97, 97, 97, 255 };
    bar->corner_radius = 20.0f;
    bar->height = DEFAULT_HEIGHT;
    
    widget->focusable = VENOM_TRUE;
}

static void search_bar_destroy(VenomWidget* widget) {
    VenomSearchBar* bar = (VenomSearchBar*)widget;
    
    if (bar->text) venom_free(bar->text, bar->text_capacity);
    if (bar->placeholder) venom_free(bar->placeholder, strlen(bar->placeholder) + 1);
    
    venom_widget_class.destroy(widget);
}

static void search_bar_measure(VenomWidget* widget, VenomF32 available_width, VenomF32 available_height,
                               VenomF32* out_width, VenomF32* out_height) {
    VenomSearchBar* bar = (VenomSearchBar*)widget;
    (void)available_height;
    
    *out_width = widget->layout.preferred_width > 0 ? widget->layout.preferred_width : available_width;
    *out_height = bar->height;
}

static void search_bar_draw(VenomWidget* widget, VenomCanvas* canvas) {
    VenomSearchBar* bar = (VenomSearchBar*)widget;
    
    VenomF32 w = widget->bounds.width;
    VenomF32 h = bar->height;
    
    /* Draw background */
    VenomRectF bg = { 0, 0, w, h };
    VenomPaint bg_paint = venom_paint_fill(bar->background_color);
    venom_canvas_draw_rounded_rect(canvas, bg, bar->corner_radius, &bg_paint);
    
    /* Draw border if focused */
    if (widget->state & VENOM_WIDGET_STATE_FOCUSED) {
        VenomPaint border_paint = venom_paint_stroke((VenomColor){ 63, 81, 181, 255 }, 2.0f);
        venom_canvas_draw_rounded_rect(canvas, bg, bar->corner_radius, &border_paint);
    }
    
    /* Draw search icon */
    VenomPaint icon_paint = venom_paint_fill(bar->icon_color);
    venom_canvas_draw_text(canvas, "🔍", 16, h / 2 + 5, NULL, &icon_paint);
    
    /* Draw text or placeholder */
    VenomF32 text_x = 44;
    VenomF32 text_y = h / 2 + 5;
    
    if (bar->text && bar->text[0]) {
        VenomPaint text_paint = venom_paint_fill(bar->text_color);
        venom_canvas_draw_text(canvas, bar->text, text_x, text_y, NULL, &text_paint);
        
        /* Clear button */
        if (bar->show_clear) {
            VenomPaint clear_paint = venom_paint_fill(bar->icon_color);
            venom_canvas_draw_text(canvas, "✕", w - 30, text_y, NULL, &clear_paint);
        }
    } else if (bar->placeholder) {
        VenomPaint ph_paint = venom_paint_fill(bar->placeholder_color);
        venom_canvas_draw_text(canvas, bar->placeholder, text_x, text_y, NULL, &ph_paint);
    }
    
    /* Draw cursor if focused */
    if (widget->state & VENOM_WIDGET_STATE_FOCUSED) {
        VenomF32 cursor_x = text_x + bar->cursor_pos * 8;
        VenomPaint cursor_paint = venom_paint_fill((VenomColor){ 63, 81, 181, 255 });
        VenomRectF cursor = { cursor_x, h / 2 - 10, 2, 20 };
        venom_canvas_draw_rect(canvas, cursor, &cursor_paint);
    }
}

static VenomBool search_bar_on_event(VenomWidget* widget, const VenomEvent* event) {
    VenomSearchBar* bar = (VenomSearchBar*)widget;
    
    switch (event->type) {
        case VENOM_EVENT_MOUSE_BUTTON_DOWN:
            /* Set focus on click */
            widget->state |= VENOM_WIDGET_STATE_FOCUSED;
            widget->needs_redraw = VENOM_TRUE;
            
            /* Check clear button */
            if (bar->text && bar->text[0] && bar->show_clear) {
                if (event->mouse.x > widget->bounds.width - 40) {
                    venom_search_bar_clear(bar);
                    return VENOM_TRUE;
                }
            }
            return VENOM_TRUE;
            
        case VENOM_EVENT_KEY_DOWN:
            if (event->key.key == VENOM_KEY_RETURN && bar->on_search) {
                bar->on_search(bar, bar->text, bar->callback_data);
                return VENOM_TRUE;
            }
            
            if (event->key.key == VENOM_KEY_BACKSPACE && bar->cursor_pos > 0) {
                bar->cursor_pos--;
                memmove(bar->text + bar->cursor_pos, bar->text + bar->cursor_pos + 1, 
                        bar->text_len - bar->cursor_pos);
                bar->text_len--;
                widget->needs_redraw = VENOM_TRUE;
                
                if (bar->on_change) bar->on_change(bar, bar->text, bar->callback_data);
                return VENOM_TRUE;
            }
            
            /* Handle UTF-8 text input from XIM (supports all languages) */
            if (event->text.text[0] != '\0' && (unsigned char)event->text.text[0] >= 32) {
                const char* input = event->text.text;
                VenomSize len = strlen(input);
                
                /* Ensure we have space (UTF-8 can be up to 4 bytes per char) */
                if (bar->text_len + len < bar->text_capacity - 4) {
                    memmove(bar->text + bar->cursor_pos + len, bar->text + bar->cursor_pos,
                            bar->text_len - bar->cursor_pos + 1);
                    memcpy(bar->text + bar->cursor_pos, input, len);
                    bar->cursor_pos += len;
                    bar->text_len += len;
                    widget->needs_redraw = VENOM_TRUE;
                    
                    if (bar->on_change) bar->on_change(bar, bar->text, bar->callback_data);
                    return VENOM_TRUE;
                }
            }
            break;
        
        default:
            break;
    }
    
    return VENOM_FALSE;
}

const VenomWidgetClass venom_search_bar_class = {
    .class_name = "VenomSearchBar",
    .instance_size = sizeof(VenomSearchBar),
    .parent_class = &venom_widget_class,
    .init = search_bar_init,
    .destroy = search_bar_destroy,
    .measure = search_bar_measure,
    .layout = NULL,
    .draw = search_bar_draw,
    .on_event = search_bar_on_event,
    .on_state_changed = NULL,
};

VenomResultPtr venom_search_bar_create(void) {
    return venom_widget_create(&venom_search_bar_class);
}

void venom_search_bar_set_text(VenomSearchBar* bar, const char* text) {
    if (!bar) return;
    
    VenomSize len = text ? strlen(text) : 0;
    if (len >= bar->text_capacity) {
        char* new_buf = (char*)venom_alloc(len + 1);
        if (!new_buf) return;
        venom_free(bar->text, bar->text_capacity);
        bar->text = new_buf;
        bar->text_capacity = len + 1;
    }
    
    if (text) memcpy(bar->text, text, len);
    bar->text[len] = '\0';
    bar->text_len = len;
    bar->cursor_pos = len;
    venom_widget_invalidate((VenomWidget*)bar);
}

const char* venom_search_bar_get_text(const VenomSearchBar* bar) {
    return bar ? bar->text : NULL;
}

void venom_search_bar_set_placeholder(VenomSearchBar* bar, const char* ph) {
    if (!bar) return;
    
    if (bar->placeholder) venom_free(bar->placeholder, strlen(bar->placeholder) + 1);
    bar->placeholder = NULL;
    
    if (ph) {
        VenomSize len = strlen(ph) + 1;
        bar->placeholder = (char*)venom_alloc(len);
        if (bar->placeholder) memcpy(bar->placeholder, ph, len);
    }
}

void venom_search_bar_set_on_search(VenomSearchBar* bar, VenomSearchCallback callback, void* data) {
    if (bar) {
        bar->on_search = callback;
        bar->callback_data = data;
    }
}

void venom_search_bar_set_on_change(VenomSearchBar* bar, VenomSearchCallback callback, void* data) {
    if (bar) {
        bar->on_change = callback;
    }
}

void venom_search_bar_clear(VenomSearchBar* bar) {
    if (bar && bar->text) {
        bar->text[0] = '\0';
        bar->text_len = 0;
        bar->cursor_pos = 0;
        venom_widget_invalidate((VenomWidget*)bar);
        
        if (bar->on_change) bar->on_change(bar, bar->text, bar->callback_data);
    }
}

VenomWidget* _venom_search_bar_build(const VenomSearchBarConfig* config) {
    VenomResultPtr result = venom_search_bar_create();
    if (!result.ok) return NULL;
    
    VenomSearchBar* bar = (VenomSearchBar*)result.value;
    
    if (config->placeholder) venom_search_bar_set_placeholder(bar, config->placeholder);
    bar->on_search = config->on_search;
    bar->callback_data = config->data;
    
    return (VenomWidget*)bar;
}
