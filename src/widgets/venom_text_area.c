/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_text_area.c - Multi-line text input implementation
 */

#include "venom/widgets/venom_text_area.h"
#include "venom/core/venom_memory.h"
#include <string.h>

#define INITIAL_CAPACITY 256
#define DEFAULT_LINE_HEIGHT 20.0f
#define DEFAULT_VISIBLE_LINES 5
#define CURSOR_BLINK_MS 500

static void text_area_init(VenomWidget* widget) {
    VenomTextArea* area = (VenomTextArea*)widget;
    
    area->text = (char*)venom_alloc(INITIAL_CAPACITY);
    if (area->text) {
        area->text[0] = '\0';
        area->text_capacity = INITIAL_CAPACITY;
    }
    area->text_len = 0;
    area->max_length = 0;  /* No limit */
    
    area->cursor_pos = 0;
    area->cursor_line = 0;
    area->cursor_col = 0;
    
    area->selection_start = 0;
    area->selection_end = 0;
    area->has_selection = VENOM_FALSE;
    
    area->scroll_x = 0;
    area->scroll_y = 0;
    area->content_height = 0;
    
    area->placeholder = NULL;
    area->read_only = VENOM_FALSE;
    area->wrap_text = VENOM_TRUE;
    area->visible_lines = DEFAULT_VISIBLE_LINES;
    area->line_height = DEFAULT_LINE_HEIGHT;
    
    area->text_color = (VenomColor){ 33, 33, 33, 255 };
    area->background_color = (VenomColor){ 255, 255, 255, 255 };
    area->border_color = (VenomColor){ 189, 189, 189, 255 };
    area->placeholder_color = (VenomColor){ 158, 158, 158, 255 };
    area->selection_color = (VenomColor){ 63, 81, 181, 80 };
    area->cursor_color = (VenomColor){ 63, 81, 181, 255 };
    area->padding = 12.0f;
    area->border_width = 1.0f;
    area->corner_radius = 4.0f;
    
    area->cursor_visible = VENOM_TRUE;
    area->cursor_blink_timer = 0;
    
    area->on_change = NULL;
    area->change_data = NULL;
    
    widget->focusable = VENOM_TRUE;
}

static void text_area_destroy(VenomWidget* widget) {
    VenomTextArea* area = (VenomTextArea*)widget;
    
    if (area->text) {
        venom_free(area->text, area->text_capacity);
        area->text = NULL;
    }
    
    if (area->placeholder) {
        venom_free(area->placeholder, strlen(area->placeholder) + 1);
        area->placeholder = NULL;
    }
    
    venom_widget_class.destroy(widget);
}

static VenomU32 count_lines(const char* text) {
    if (!text || !text[0]) return 1;
    
    VenomU32 lines = 1;
    while (*text) {
        if (*text == '\n') lines++;
        text++;
    }
    return lines;
}

static void text_area_measure(VenomWidget* widget, VenomF32 available_width, VenomF32 available_height,
                              VenomF32* out_width, VenomF32* out_height) {
    VenomTextArea* area = (VenomTextArea*)widget;
    (void)available_height;
    
    *out_width = widget->layout.preferred_width > 0 ? 
                 widget->layout.preferred_width : available_width;
    
    VenomF32 content_height = area->visible_lines * area->line_height;
    *out_height = content_height + area->padding * 2 + area->border_width * 2;
}

static void text_area_draw(VenomWidget* widget, VenomCanvas* canvas) {
    VenomTextArea* area = (VenomTextArea*)widget;
    
    VenomF32 w = widget->bounds.width;
    VenomF32 h = widget->bounds.height;
    
    /* Draw background */
    VenomRectF bg = { 0, 0, w, h };
    VenomPaint bg_paint = venom_paint_fill(area->background_color);
    venom_canvas_draw_rounded_rect(canvas, bg, area->corner_radius, &bg_paint);
    
    /* Draw border */
    VenomColor border_color = area->border_color;
    if (widget->state & VENOM_WIDGET_STATE_FOCUSED) {
        border_color = area->cursor_color;
    }
    VenomPaint border_paint = venom_paint_stroke(border_color, area->border_width);
    venom_canvas_draw_rounded_rect(canvas, bg, area->corner_radius, &border_paint);
    
    /* Clip to content area */
    VenomRectF content = { 
        area->padding, area->padding,
        w - area->padding * 2, h - area->padding * 2
    };
    venom_canvas_save(canvas);
    venom_canvas_clip_rect(canvas, content);
    venom_canvas_translate(canvas, area->padding - area->scroll_x, area->padding - area->scroll_y);
    
    /* Draw placeholder if empty */
    if ((!area->text || !area->text[0]) && area->placeholder) {
        VenomPaint ph_paint = venom_paint_fill(area->placeholder_color);
        venom_canvas_draw_text(canvas, area->placeholder, 0, area->line_height - 4, NULL, &ph_paint);
    }
    
    /* Draw text line by line */
    if (area->text && area->text[0]) {
        VenomPaint text_paint = venom_paint_fill(area->text_color);
        
        VenomF32 y = area->line_height - 4;
        const char* line_start = area->text;
        const char* p = area->text;
        
        while (*p) {
            if (*p == '\n') {
                /* Draw this line */
                VenomSize line_len = p - line_start;
                if (line_len > 0) {
                    char* line_buf = (char*)venom_alloc(line_len + 1);
                    if (line_buf) {
                        memcpy(line_buf, line_start, line_len);
                        line_buf[line_len] = '\0';
                        venom_canvas_draw_text(canvas, line_buf, 0, y, NULL, &text_paint);
                        venom_free(line_buf, line_len + 1);
                    }
                }
                y += area->line_height;
                line_start = p + 1;
            }
            p++;
        }
        
        /* Draw last line */
        if (line_start < p) {
            venom_canvas_draw_text(canvas, line_start, 0, y, NULL, &text_paint);
        }
        
        area->content_height = y;
    }
    
    /* Draw cursor */
    if ((widget->state & VENOM_WIDGET_STATE_FOCUSED) && area->cursor_visible && !area->read_only) {
        VenomF32 cx = (VenomF32)area->cursor_col * 8;  /* Approx char width */
        VenomF32 cy = (VenomF32)area->cursor_line * area->line_height;
        
        VenomPaint cursor_paint = venom_paint_fill(area->cursor_color);
        VenomRectF cursor_rect = { cx, cy, 2, area->line_height };
        venom_canvas_draw_rect(canvas, cursor_rect, &cursor_paint);
    }
    
    venom_canvas_restore(canvas);
}

/* UTF-8 helper */
static VenomSize utf8_char_len(unsigned char c) {
    if (c < 0x80) return 1;
    if ((c & 0xE0) == 0xC0) return 2;
    if ((c & 0xF0) == 0xE0) return 3;
    if ((c & 0xF8) == 0xF0) return 4;
    return 1;
}

static void update_cursor_position(VenomTextArea* area) {
    area->cursor_line = 0;
    area->cursor_col = 0;
    
    for (VenomU32 i = 0; i < area->cursor_pos && i < area->text_len; ) {
        if (area->text[i] == '\n') {
            area->cursor_line++;
            area->cursor_col = 0;
            i++;
        } else {
            VenomSize clen = utf8_char_len((unsigned char)area->text[i]);
            area->cursor_col++;
            i += clen;
        }
    }
}

static VenomBool text_area_on_event(VenomWidget* widget, const VenomEvent* event) {
    VenomTextArea* area = (VenomTextArea*)widget;
    
    switch (event->type) {
        case VENOM_EVENT_MOUSE_BUTTON_DOWN:
            /* TODO: Calculate cursor position from click */
            return VENOM_TRUE;
            
        case VENOM_EVENT_KEY_DOWN: {
            if (area->read_only) {
                /* Allow navigation but not editing */
                switch (event->key.key) {
                    case VENOM_KEY_UP:
                    case VENOM_KEY_DOWN:
                    case VENOM_KEY_LEFT:
                    case VENOM_KEY_RIGHT:
                        break;
                    default:
                        return VENOM_FALSE;
                }
            }
            
            switch (event->key.key) {
                case VENOM_KEY_RETURN: {
                    /* Insert newline */
                    if (area->text_len + 1 < area->text_capacity) {
                        memmove(area->text + area->cursor_pos + 1,
                                area->text + area->cursor_pos,
                                area->text_len - area->cursor_pos + 1);
                        area->text[area->cursor_pos] = '\n';
                        area->cursor_pos++;
                        area->text_len++;
                        update_cursor_position(area);
                        widget->needs_redraw = VENOM_TRUE;
                        
                        if (area->on_change) {
                            area->on_change(area, area->text, area->change_data);
                        }
                    }
                    return VENOM_TRUE;
                }
                
                case VENOM_KEY_BACKSPACE: {
                    if (area->cursor_pos > 0) {
                        /* Find start of previous UTF-8 char */
                        VenomU32 del_start = area->cursor_pos - 1;
                        while (del_start > 0 && (area->text[del_start] & 0xC0) == 0x80) {
                            del_start--;
                        }
                        
                        VenomSize del_len = area->cursor_pos - del_start;
                        memmove(area->text + del_start,
                                area->text + area->cursor_pos,
                                area->text_len - area->cursor_pos + 1);
                        area->cursor_pos = del_start;
                        area->text_len -= del_len;
                        update_cursor_position(area);
                        widget->needs_redraw = VENOM_TRUE;
                        
                        if (area->on_change) {
                            area->on_change(area, area->text, area->change_data);
                        }
                    }
                    return VENOM_TRUE;
                }
                
                case VENOM_KEY_LEFT:
                    if (area->cursor_pos > 0) {
                        area->cursor_pos--;
                        while (area->cursor_pos > 0 && 
                               (area->text[area->cursor_pos] & 0xC0) == 0x80) {
                            area->cursor_pos--;
                        }
                        update_cursor_position(area);
                        widget->needs_redraw = VENOM_TRUE;
                    }
                    return VENOM_TRUE;
                    
                case VENOM_KEY_RIGHT:
                    if (area->cursor_pos < area->text_len) {
                        VenomSize clen = utf8_char_len((unsigned char)area->text[area->cursor_pos]);
                        area->cursor_pos += clen;
                        if (area->cursor_pos > area->text_len) {
                            area->cursor_pos = area->text_len;
                        }
                        update_cursor_position(area);
                        widget->needs_redraw = VENOM_TRUE;
                    }
                    return VENOM_TRUE;
                    
                case VENOM_KEY_UP: {
                    /* Move up one line */
                    if (area->cursor_line > 0) {
                        /* Find start of current line */
                        VenomU32 line_start = area->cursor_pos;
                        while (line_start > 0 && area->text[line_start - 1] != '\n') {
                            line_start--;
                        }
                        /* Find start of previous line */
                        VenomU32 prev_line_end = line_start > 0 ? line_start - 1 : 0;
                        VenomU32 prev_line_start = prev_line_end;
                        while (prev_line_start > 0 && area->text[prev_line_start - 1] != '\n') {
                            prev_line_start--;
                        }
                        /* Move to same column in previous line */
                        VenomU32 col = area->cursor_pos - line_start;
                        VenomU32 prev_line_len = prev_line_end - prev_line_start;
                        area->cursor_pos = prev_line_start + (col < prev_line_len ? col : prev_line_len);
                        update_cursor_position(area);
                        widget->needs_redraw = VENOM_TRUE;
                    }
                    return VENOM_TRUE;
                }
                
                case VENOM_KEY_DOWN: {
                    /* Find end of current line */
                    VenomU32 line_end = area->cursor_pos;
                    while (line_end < area->text_len && area->text[line_end] != '\n') {
                        line_end++;
                    }
                    if (line_end < area->text_len) {
                        /* Find start of next line */
                        VenomU32 next_line_start = line_end + 1;
                        VenomU32 next_line_end = next_line_start;
                        while (next_line_end < area->text_len && area->text[next_line_end] != '\n') {
                            next_line_end++;
                        }
                        /* Find start of current line */
                        VenomU32 line_start = area->cursor_pos;
                        while (line_start > 0 && area->text[line_start - 1] != '\n') {
                            line_start--;
                        }
                        VenomU32 col = area->cursor_pos - line_start;
                        VenomU32 next_line_len = next_line_end - next_line_start;
                        area->cursor_pos = next_line_start + (col < next_line_len ? col : next_line_len);
                        update_cursor_position(area);
                        widget->needs_redraw = VENOM_TRUE;
                    }
                    return VENOM_TRUE;
                }
                
                default:
                    break;
            }
            break;
        }
        
        case VENOM_EVENT_TEXT_INPUT: {
            if (area->read_only) return VENOM_FALSE;
            
            const char* input = event->text.text;
            VenomSize input_len = strlen(input);
            
            if (input_len > 0 && area->text_len + input_len < area->text_capacity) {
                memmove(area->text + area->cursor_pos + input_len,
                        area->text + area->cursor_pos,
                        area->text_len - area->cursor_pos + 1);
                memcpy(area->text + area->cursor_pos, input, input_len);
                area->cursor_pos += input_len;
                area->text_len += input_len;
                update_cursor_position(area);
                widget->needs_redraw = VENOM_TRUE;
                
                if (area->on_change) {
                    area->on_change(area, area->text, area->change_data);
                }
            }
            return VENOM_TRUE;
        }
        
        case VENOM_EVENT_MOUSE_SCROLL: {
            area->scroll_y -= event->scroll.y * area->line_height;
            if (area->scroll_y < 0) area->scroll_y = 0;
            VenomF32 max_scroll = area->content_height - (widget->bounds.height - area->padding * 2);
            if (max_scroll < 0) max_scroll = 0;
            if (area->scroll_y > max_scroll) area->scroll_y = max_scroll;
            widget->needs_redraw = VENOM_TRUE;
            return VENOM_TRUE;
        }
        
        default:
            break;
    }
    
    return VENOM_FALSE;
}

const VenomWidgetClass venom_text_area_class = {
    .class_name = "VenomTextArea",
    .instance_size = sizeof(VenomTextArea),
    .parent_class = &venom_widget_class,
    .init = text_area_init,
    .destroy = text_area_destroy,
    .measure = text_area_measure,
    .layout = NULL,
    .draw = text_area_draw,
    .on_event = text_area_on_event,
    .on_state_changed = NULL,
};

VenomResultPtr venom_text_area_create(void) {
    return venom_widget_create(&venom_text_area_class);
}

void venom_text_area_set_text(VenomTextArea* area, const char* text) {
    if (!area) return;
    
    VenomSize len = text ? strlen(text) : 0;
    
    if (len >= area->text_capacity) {
        VenomSize new_cap = len + 1;
        char* new_buf = (char*)venom_alloc(new_cap);
        if (!new_buf) return;
        
        if (area->text) {
            venom_free(area->text, area->text_capacity);
        }
        area->text = new_buf;
        area->text_capacity = new_cap;
    }
    
    if (text) {
        memcpy(area->text, text, len);
    }
    area->text[len] = '\0';
    area->text_len = len;
    area->cursor_pos = len;
    update_cursor_position(area);
    venom_widget_invalidate((VenomWidget*)area);
}

const char* venom_text_area_get_text(const VenomTextArea* area) {
    return area ? area->text : NULL;
}

void venom_text_area_set_placeholder(VenomTextArea* area, const char* placeholder) {
    if (!area) return;
    
    if (area->placeholder) {
        venom_free(area->placeholder, strlen(area->placeholder) + 1);
        area->placeholder = NULL;
    }
    
    if (placeholder) {
        VenomSize len = strlen(placeholder) + 1;
        area->placeholder = (char*)venom_alloc(len);
        if (area->placeholder) {
            memcpy(area->placeholder, placeholder, len);
        }
    }
}

void venom_text_area_set_read_only(VenomTextArea* area, VenomBool read_only) {
    if (area) area->read_only = read_only;
}

void venom_text_area_set_wrap(VenomTextArea* area, VenomBool wrap) {
    if (area) {
        area->wrap_text = wrap;
        venom_widget_invalidate((VenomWidget*)area);
    }
}

void venom_text_area_set_max_length(VenomTextArea* area, VenomU32 max) {
    if (area) area->max_length = max;
}

void venom_text_area_set_on_change(VenomTextArea* area, VenomTextAreaCallback callback, void* data) {
    if (area) {
        area->on_change = callback;
        area->change_data = data;
    }
}

void venom_text_area_insert(VenomTextArea* area, const char* text) {
    if (!area || !text) return;
    
    VenomSize len = strlen(text);
    if (area->text_len + len >= area->text_capacity) {
        VenomSize new_cap = area->text_capacity * 2 + len;
        char* new_buf = (char*)venom_alloc(new_cap);
        if (!new_buf) return;
        
        memcpy(new_buf, area->text, area->text_len + 1);
        venom_free(area->text, area->text_capacity);
        area->text = new_buf;
        area->text_capacity = new_cap;
    }
    
    memmove(area->text + area->cursor_pos + len,
            area->text + area->cursor_pos,
            area->text_len - area->cursor_pos + 1);
    memcpy(area->text + area->cursor_pos, text, len);
    area->cursor_pos += len;
    area->text_len += len;
    update_cursor_position(area);
    venom_widget_invalidate((VenomWidget*)area);
}

void venom_text_area_clear(VenomTextArea* area) {
    if (area && area->text) {
        area->text[0] = '\0';
        area->text_len = 0;
        area->cursor_pos = 0;
        area->cursor_line = 0;
        area->cursor_col = 0;
        venom_widget_invalidate((VenomWidget*)area);
    }
}

VenomU32 venom_text_area_get_line_count(const VenomTextArea* area) {
    return area ? count_lines(area->text) : 0;
}

VenomWidget* _venom_text_area_build(const VenomTextAreaConfig* config) {
    VenomResultPtr result = venom_text_area_create();
    if (!result.ok) return NULL;
    
    VenomTextArea* area = (VenomTextArea*)result.value;
    
    if (config->text) venom_text_area_set_text(area, config->text);
    if (config->placeholder) venom_text_area_set_placeholder(area, config->placeholder);
    area->read_only = config->read_only;
    area->wrap_text = config->wrap;
    if (config->lines > 0) area->visible_lines = config->lines;
    area->on_change = config->on_change;
    area->change_data = config->data;
    
    return (VenomWidget*)area;
}
