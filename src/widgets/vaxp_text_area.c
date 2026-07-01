/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_text_area.c - Multi-line text input implementation
 */

#include "vaxp/widgets/vaxp_text_area.h"
#include "vaxp/core/vaxp_memory.h"
#include <string.h>

#define INITIAL_CAPACITY 256
#define DEFAULT_LINE_HEIGHT 20.0f
#define DEFAULT_VISIBLE_LINES 5
#define CURSOR_BLINK_MS 500

static void text_area_init(VaxpWidget* widget) {
    VaxpTextArea* area = (VaxpTextArea*)widget;
    
    area->text = (char*)vaxp_alloc(INITIAL_CAPACITY);
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
    area->has_selection = VAXP_FALSE;
    
    area->scroll_x = 0;
    area->scroll_y = 0;
    area->content_height = 0;
    
    area->placeholder = NULL;
    area->read_only = VAXP_FALSE;
    area->wrap_text = VAXP_TRUE;
    area->visible_lines = DEFAULT_VISIBLE_LINES;
    area->line_height = DEFAULT_LINE_HEIGHT;
    
    area->text_color = (VaxpColor){ 33, 33, 33, 255 };
    area->background_color = (VaxpColor){ 255, 255, 255, 255 };
    area->border_color = (VaxpColor){ 189, 189, 189, 255 };
    area->placeholder_color = (VaxpColor){ 158, 158, 158, 255 };
    area->selection_color = (VaxpColor){ 63, 81, 181, 80 };
    area->cursor_color = (VaxpColor){ 63, 81, 181, 255 };
    area->padding = 12.0f;
    area->border_width = 1.0f;
    area->corner_radius = 4.0f;
    
    area->cursor_visible = VAXP_TRUE;
    area->cursor_blink_timer = 0;
    
    area->on_change = NULL;
    area->change_data = NULL;
    
    widget->focusable = VAXP_TRUE;
}

static void text_area_destroy(VaxpWidget* widget) {
    VaxpTextArea* area = (VaxpTextArea*)widget;
    
    if (area->text) {
        vaxp_free(area->text, area->text_capacity);
        area->text = NULL;
    }
    
    if (area->placeholder) {
        vaxp_free(area->placeholder, strlen(area->placeholder) + 1);
        area->placeholder = NULL;
    }
    
    vaxp_widget_class.destroy(widget);
}

static VaxpU32 count_lines(const char* text) {
    if (!text || !text[0]) return 1;
    
    VaxpU32 lines = 1;
    while (*text) {
        if (*text == '\n') lines++;
        text++;
    }
    return lines;
}

static void text_area_measure(VaxpWidget* widget, VaxpF32 available_width, VaxpF32 available_height,
                              VaxpF32* out_width, VaxpF32* out_height) {
    VaxpTextArea* area = (VaxpTextArea*)widget;
    (void)available_height;
    
    *out_width = widget->layout.preferred_width > 0 ? 
                 widget->layout.preferred_width : available_width;
    
    VaxpF32 content_height = area->visible_lines * area->line_height;
    *out_height = content_height + area->padding * 2 + area->border_width * 2;
}

static void text_area_draw(VaxpWidget* widget, VaxpCanvas* canvas) {
    VaxpTextArea* area = (VaxpTextArea*)widget;
    
    VaxpF32 w = widget->bounds.width;
    VaxpF32 h = widget->bounds.height;
    
    /* Draw background */
    VaxpRectF bg = { 0, 0, w, h };
    VaxpPaint bg_paint = vaxp_paint_fill(area->background_color);
    vaxp_canvas_draw_rounded_rect(canvas, bg, area->corner_radius, &bg_paint);
    
    /* Draw border */
    VaxpColor border_color = area->border_color;
    if (widget->state & VAXP_WIDGET_STATE_FOCUSED) {
        border_color = area->cursor_color;
    }
    VaxpPaint border_paint = vaxp_paint_stroke(border_color, area->border_width);
    vaxp_canvas_draw_rounded_rect(canvas, bg, area->corner_radius, &border_paint);
    
    /* Clip to content area */
    VaxpRectF content = { 
        area->padding, area->padding,
        w - area->padding * 2, h - area->padding * 2
    };
    vaxp_canvas_save(canvas);
    vaxp_canvas_clip_rect(canvas, content);
    vaxp_canvas_translate(canvas, area->padding - area->scroll_x, area->padding - area->scroll_y);
    
    /* Draw placeholder if empty */
    if ((!area->text || !area->text[0]) && area->placeholder) {
        VaxpPaint ph_paint = vaxp_paint_fill(area->placeholder_color);
        vaxp_canvas_draw_text(canvas, area->placeholder, 0, area->line_height - 4, NULL, &ph_paint);
    }
    
    /* Draw text line by line */
    if (area->text && area->text[0]) {
        VaxpPaint text_paint = vaxp_paint_fill(area->text_color);
        
        VaxpF32 y = area->line_height - 4;
        const char* line_start = area->text;
        const char* p = area->text;
        
        while (*p) {
            if (*p == '\n') {
                /* Draw this line */
                VaxpSize line_len = p - line_start;
                if (line_len > 0) {
                    char* line_buf = (char*)vaxp_alloc(line_len + 1);
                    if (line_buf) {
                        memcpy(line_buf, line_start, line_len);
                        line_buf[line_len] = '\0';
                        vaxp_canvas_draw_text(canvas, line_buf, 0, y, NULL, &text_paint);
                        vaxp_free(line_buf, line_len + 1);
                    }
                }
                y += area->line_height;
                line_start = p + 1;
            }
            p++;
        }
        
        /* Draw last line */
        if (line_start < p) {
            vaxp_canvas_draw_text(canvas, line_start, 0, y, NULL, &text_paint);
        }
        
        area->content_height = y;
    }
    
    /* Draw cursor */
    if ((widget->state & VAXP_WIDGET_STATE_FOCUSED) && area->cursor_visible && !area->read_only) {
        VaxpF32 cx = (VaxpF32)area->cursor_col * 8;  /* Approx char width */
        VaxpF32 cy = (VaxpF32)area->cursor_line * area->line_height;
        
        VaxpPaint cursor_paint = vaxp_paint_fill(area->cursor_color);
        VaxpRectF cursor_rect = { cx, cy, 2, area->line_height };
        vaxp_canvas_draw_rect(canvas, cursor_rect, &cursor_paint);
    }
    
    vaxp_canvas_restore(canvas);
}

/* UTF-8 helper */
static VaxpSize utf8_char_len(unsigned char c) {
    if (c < 0x80) return 1;
    if ((c & 0xE0) == 0xC0) return 2;
    if ((c & 0xF0) == 0xE0) return 3;
    if ((c & 0xF8) == 0xF0) return 4;
    return 1;
}

static void update_cursor_position(VaxpTextArea* area) {
    area->cursor_line = 0;
    area->cursor_col = 0;
    
    for (VaxpU32 i = 0; i < area->cursor_pos && i < area->text_len; ) {
        if (area->text[i] == '\n') {
            area->cursor_line++;
            area->cursor_col = 0;
            i++;
        } else {
            VaxpSize clen = utf8_char_len((unsigned char)area->text[i]);
            area->cursor_col++;
            i += clen;
        }
    }
}

static VaxpBool text_area_on_event(VaxpWidget* widget, const VaxpEvent* event) {
    VaxpTextArea* area = (VaxpTextArea*)widget;
    
    switch (event->type) {
        case VAXP_EVENT_MOUSE_BUTTON_DOWN:
            /* TODO: Calculate cursor position from click */
            return VAXP_TRUE;
            
        case VAXP_EVENT_KEY_DOWN: {
            if (area->read_only) {
                /* Allow navigation but not editing */
                switch (event->key.key) {
                    case VAXP_KEY_UP:
                    case VAXP_KEY_DOWN:
                    case VAXP_KEY_LEFT:
                    case VAXP_KEY_RIGHT:
                        break;
                    default:
                        return VAXP_FALSE;
                }
            }
            
            switch (event->key.key) {
                case VAXP_KEY_RETURN: {
                    /* Insert newline */
                    if (area->text_len + 1 < area->text_capacity) {
                        memmove(area->text + area->cursor_pos + 1,
                                area->text + area->cursor_pos,
                                area->text_len - area->cursor_pos + 1);
                        area->text[area->cursor_pos] = '\n';
                        area->cursor_pos++;
                        area->text_len++;
                        update_cursor_position(area);
                        widget->needs_redraw = VAXP_TRUE;
                        
                        if (area->on_change) {
                            area->on_change(area, area->text, area->change_data);
                        }
                    }
                    return VAXP_TRUE;
                }
                
                case VAXP_KEY_BACKSPACE: {
                    if (area->cursor_pos > 0) {
                        /* Find start of previous UTF-8 char */
                        VaxpU32 del_start = area->cursor_pos - 1;
                        while (del_start > 0 && (area->text[del_start] & 0xC0) == 0x80) {
                            del_start--;
                        }
                        
                        VaxpSize del_len = area->cursor_pos - del_start;
                        memmove(area->text + del_start,
                                area->text + area->cursor_pos,
                                area->text_len - area->cursor_pos + 1);
                        area->cursor_pos = del_start;
                        area->text_len -= del_len;
                        update_cursor_position(area);
                        widget->needs_redraw = VAXP_TRUE;
                        
                        if (area->on_change) {
                            area->on_change(area, area->text, area->change_data);
                        }
                    }
                    return VAXP_TRUE;
                }
                
                case VAXP_KEY_LEFT:
                    if (area->cursor_pos > 0) {
                        area->cursor_pos--;
                        while (area->cursor_pos > 0 && 
                               (area->text[area->cursor_pos] & 0xC0) == 0x80) {
                            area->cursor_pos--;
                        }
                        update_cursor_position(area);
                        widget->needs_redraw = VAXP_TRUE;
                    }
                    return VAXP_TRUE;
                    
                case VAXP_KEY_RIGHT:
                    if (area->cursor_pos < area->text_len) {
                        VaxpSize clen = utf8_char_len((unsigned char)area->text[area->cursor_pos]);
                        area->cursor_pos += clen;
                        if (area->cursor_pos > area->text_len) {
                            area->cursor_pos = area->text_len;
                        }
                        update_cursor_position(area);
                        widget->needs_redraw = VAXP_TRUE;
                    }
                    return VAXP_TRUE;
                    
                case VAXP_KEY_UP: {
                    /* Move up one line */
                    if (area->cursor_line > 0) {
                        /* Find start of current line */
                        VaxpU32 line_start = area->cursor_pos;
                        while (line_start > 0 && area->text[line_start - 1] != '\n') {
                            line_start--;
                        }
                        /* Find start of previous line */
                        VaxpU32 prev_line_end = line_start > 0 ? line_start - 1 : 0;
                        VaxpU32 prev_line_start = prev_line_end;
                        while (prev_line_start > 0 && area->text[prev_line_start - 1] != '\n') {
                            prev_line_start--;
                        }
                        /* Move to same column in previous line */
                        VaxpU32 col = area->cursor_pos - line_start;
                        VaxpU32 prev_line_len = prev_line_end - prev_line_start;
                        area->cursor_pos = prev_line_start + (col < prev_line_len ? col : prev_line_len);
                        update_cursor_position(area);
                        widget->needs_redraw = VAXP_TRUE;
                    }
                    return VAXP_TRUE;
                }
                
                case VAXP_KEY_DOWN: {
                    /* Find end of current line */
                    VaxpU32 line_end = area->cursor_pos;
                    while (line_end < area->text_len && area->text[line_end] != '\n') {
                        line_end++;
                    }
                    if (line_end < area->text_len) {
                        /* Find start of next line */
                        VaxpU32 next_line_start = line_end + 1;
                        VaxpU32 next_line_end = next_line_start;
                        while (next_line_end < area->text_len && area->text[next_line_end] != '\n') {
                            next_line_end++;
                        }
                        /* Find start of current line */
                        VaxpU32 line_start = area->cursor_pos;
                        while (line_start > 0 && area->text[line_start - 1] != '\n') {
                            line_start--;
                        }
                        VaxpU32 col = area->cursor_pos - line_start;
                        VaxpU32 next_line_len = next_line_end - next_line_start;
                        area->cursor_pos = next_line_start + (col < next_line_len ? col : next_line_len);
                        update_cursor_position(area);
                        widget->needs_redraw = VAXP_TRUE;
                    }
                    return VAXP_TRUE;
                }
                
                default:
                    break;
            }
            break;
        }
        
        case VAXP_EVENT_TEXT_INPUT: {
            if (area->read_only) return VAXP_FALSE;
            
            const char* input = event->text.text;
            VaxpSize input_len = strlen(input);
            
            if (input_len > 0 && area->text_len + input_len < area->text_capacity) {
                memmove(area->text + area->cursor_pos + input_len,
                        area->text + area->cursor_pos,
                        area->text_len - area->cursor_pos + 1);
                memcpy(area->text + area->cursor_pos, input, input_len);
                area->cursor_pos += input_len;
                area->text_len += input_len;
                update_cursor_position(area);
                widget->needs_redraw = VAXP_TRUE;
                
                if (area->on_change) {
                    area->on_change(area, area->text, area->change_data);
                }
            }
            return VAXP_TRUE;
        }
        
        case VAXP_EVENT_MOUSE_SCROLL: {
            area->scroll_y -= event->scroll.y * area->line_height;
            if (area->scroll_y < 0) area->scroll_y = 0;
            VaxpF32 max_scroll = area->content_height - (widget->bounds.height - area->padding * 2);
            if (max_scroll < 0) max_scroll = 0;
            if (area->scroll_y > max_scroll) area->scroll_y = max_scroll;
            widget->needs_redraw = VAXP_TRUE;
            return VAXP_TRUE;
        }
        
        default:
            break;
    }
    
    return VAXP_FALSE;
}

const VaxpWidgetClass vaxp_text_area_class = {
    .class_name = "VaxpTextArea",
    .instance_size = sizeof(VaxpTextArea),
    .parent_class = &vaxp_widget_class,
    .init = text_area_init,
    .destroy = text_area_destroy,
    .measure = text_area_measure,
    .layout = NULL,
    .draw = text_area_draw,
    .on_event = text_area_on_event,
    .on_state_changed = NULL,
};

VaxpResultPtr vaxp_text_area_create(void) {
    return vaxp_widget_create(&vaxp_text_area_class);
}

void vaxp_text_area_set_text(VaxpTextArea* area, const char* text) {
    if (!area) return;
    
    VaxpSize len = text ? strlen(text) : 0;
    
    if (len >= area->text_capacity) {
        VaxpSize new_cap = len + 1;
        char* new_buf = (char*)vaxp_alloc(new_cap);
        if (!new_buf) return;
        
        if (area->text) {
            vaxp_free(area->text, area->text_capacity);
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
    vaxp_widget_invalidate((VaxpWidget*)area);
}

const char* vaxp_text_area_get_text(const VaxpTextArea* area) {
    return area ? area->text : NULL;
}

void vaxp_text_area_set_placeholder(VaxpTextArea* area, const char* placeholder) {
    if (!area) return;
    
    if (area->placeholder) {
        vaxp_free(area->placeholder, strlen(area->placeholder) + 1);
        area->placeholder = NULL;
    }
    
    if (placeholder) {
        VaxpSize len = strlen(placeholder) + 1;
        area->placeholder = (char*)vaxp_alloc(len);
        if (area->placeholder) {
            memcpy(area->placeholder, placeholder, len);
        }
    }
}

void vaxp_text_area_set_read_only(VaxpTextArea* area, VaxpBool read_only) {
    if (area) area->read_only = read_only;
}

void vaxp_text_area_set_wrap(VaxpTextArea* area, VaxpBool wrap) {
    if (area) {
        area->wrap_text = wrap;
        vaxp_widget_invalidate((VaxpWidget*)area);
    }
}

void vaxp_text_area_set_max_length(VaxpTextArea* area, VaxpU32 max) {
    if (area) area->max_length = max;
}

void vaxp_text_area_set_on_change(VaxpTextArea* area, VaxpTextAreaCallback callback, void* data) {
    if (area) {
        area->on_change = callback;
        area->change_data = data;
    }
}

void vaxp_text_area_insert(VaxpTextArea* area, const char* text) {
    if (!area || !text) return;
    
    VaxpSize len = strlen(text);
    if (area->text_len + len >= area->text_capacity) {
        VaxpSize new_cap = area->text_capacity * 2 + len;
        char* new_buf = (char*)vaxp_alloc(new_cap);
        if (!new_buf) return;
        
        memcpy(new_buf, area->text, area->text_len + 1);
        vaxp_free(area->text, area->text_capacity);
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
    vaxp_widget_invalidate((VaxpWidget*)area);
}

void vaxp_text_area_clear(VaxpTextArea* area) {
    if (area && area->text) {
        area->text[0] = '\0';
        area->text_len = 0;
        area->cursor_pos = 0;
        area->cursor_line = 0;
        area->cursor_col = 0;
        vaxp_widget_invalidate((VaxpWidget*)area);
    }
}

VaxpU32 vaxp_text_area_get_line_count(const VaxpTextArea* area) {
    return area ? count_lines(area->text) : 0;
}

VaxpWidget* _vaxp_text_area_build(const VaxpTextAreaConfig* config) {
    VaxpResultPtr result = vaxp_text_area_create();
    if (!result.ok) return NULL;
    
    VaxpTextArea* area = (VaxpTextArea*)result.value;
    
    if (config->text) vaxp_text_area_set_text(area, config->text);
    if (config->placeholder) vaxp_text_area_set_placeholder(area, config->placeholder);
    area->read_only = config->read_only;
    area->wrap_text = config->wrap;
    if (config->lines > 0) area->visible_lines = config->lines;
    area->on_change = config->on_change;
    area->change_data = config->data;
    
    return (VaxpWidget*)area;
}
