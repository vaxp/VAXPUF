#include "vaxp_terminal.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <termios.h>
#include "vaxp/widgets/vaxp_context_menu.h"

#define MAX_TERM_COLS 512
#define MAX_TERM_ROWS 256

static void term_init(VaxpWidget* self);
static void term_destroy(VaxpWidget* self);
static void term_draw(VaxpWidget* self, VaxpCanvas* canvas);
static VaxpBool term_on_event(VaxpWidget* self, const VaxpEvent* event);

const VaxpWidgetClass vaxp_terminal_class = {
    .class_name = "VaxpTerminal",
    .instance_size = sizeof(VaxpTerminal),
    .parent_class = &vaxp_widget_class,
    .init = term_init,
    .destroy = term_destroy,
    .draw = term_draw,
    .on_event = term_on_event,
};

static void term_init(VaxpWidget* self) {
    vaxp_widget_init_base(self, &vaxp_terminal_class);
    VaxpTerminal* term = (VaxpTerminal*)self;
    term->ring_buffer = NULL;
    term->alt_buffer = NULL;
    term->cols = 0;
    term->rows = 0;
    term->max_rows = 1000;
    term->screen_top_row = 0;
    term->history_count = 0;
    term->scroll_offset = 0;
    term->use_alt_buffer = VAXP_FALSE;
    term->cursor_x = 0;
    term->cursor_y = 0;
    term->saved_cursor_x = 0;
    term->saved_cursor_y = 0;
    term->cursor_visible = VAXP_TRUE;
    term->default_fg = vaxp_color_rgb(220, 220, 220);
    term->default_bg = vaxp_color_rgba(0, 0, 0, 100);
    term->current_fg = term->default_fg;
    term->current_bg = term->default_bg;
    term->current_flags = 0;
    term->font_size = 14.0f;
    term->char_width = 8.0f;
    term->char_height = 16.0f;
    term->pty_fd = -1;
    term->ansi_state = 0;
    term->ansi_is_private = VAXP_FALSE;
    term->utf8_state = 0;
    
    term->mouse_reporting_mode = 0;
    term->is_selecting = VAXP_FALSE;
    term->sel_start_x = -1; term->sel_start_y = -1;
    term->sel_end_x = -1; term->sel_end_y = -1;
    term->context_menu = NULL;
    
    self->focusable = VAXP_TRUE;
}

static void term_destroy(VaxpWidget* self) {
    VaxpTerminal* term = (VaxpTerminal*)self;
    if (term->ring_buffer) free(term->ring_buffer);
    if (term->alt_buffer) free(term->alt_buffer);
    if (term->context_menu) vaxp_unref(term->context_menu);
}

void vaxp_terminal_init_grid(VaxpTerminal* term, int cols, int rows) {
    if (term->ring_buffer) free(term->ring_buffer);
    if (term->alt_buffer) free(term->alt_buffer);
    
    term->cols = cols;
    term->rows = rows;
    term->ring_buffer = malloc(MAX_TERM_COLS * term->max_rows * sizeof(TermCell));
    term->alt_buffer = malloc(MAX_TERM_COLS * MAX_TERM_ROWS * sizeof(TermCell));
    
    for (int i = 0; i < MAX_TERM_COLS * term->max_rows; i++) {
        term->ring_buffer[i].ch[0] = ' ';
        term->ring_buffer[i].ch[1] = '\0';
        term->ring_buffer[i].fg = term->default_fg;
        term->ring_buffer[i].bg = term->default_bg;
        term->ring_buffer[i].flags = 0;
    }
}

static TermCell* get_cell(VaxpTerminal* term, int x, int y) {
    if (x < 0 || x >= term->cols || y < 0 || y >= term->rows) return NULL;
    if (term->use_alt_buffer && term->alt_buffer) {
        return &term->alt_buffer[y * MAX_TERM_COLS + x];
    }
    int logical_top = (term->screen_top_row - term->scroll_offset + term->max_rows) % term->max_rows;
    int ring_y = (logical_top + y) % term->max_rows;
    return &term->ring_buffer[ring_y * MAX_TERM_COLS + x];
}

static void term_scroll_up(VaxpTerminal* term) {
    if (term->use_alt_buffer && term->alt_buffer) {
        int row_bytes = MAX_TERM_COLS * sizeof(TermCell);
        memmove(term->alt_buffer, term->alt_buffer + MAX_TERM_COLS, row_bytes * (term->rows - 1));
        TermCell* last_row = term->alt_buffer + (term->rows - 1) * MAX_TERM_COLS;
        for (int i = 0; i < term->cols; i++) {
            last_row[i].ch[0] = ' ';
            last_row[i].ch[1] = '\0';
            last_row[i].fg = term->default_fg;
            last_row[i].bg = term->default_bg;
            last_row[i].flags = 0;
        }
        return;
    }
    
    if (!term->ring_buffer) return;
    
    term->screen_top_row = (term->screen_top_row + 1) % term->max_rows;
    if (term->history_count < term->max_rows - term->rows) {
        term->history_count++;
    }
    
    if (term->scroll_offset > 0) {
        term->scroll_offset++;
        if (term->scroll_offset >= term->max_rows - term->rows) {
            term->scroll_offset = term->max_rows - term->rows;
        }
    }
    
    if (term->sel_start_y >= 0) term->sel_start_y--;
    if (term->sel_end_y >= 0) term->sel_end_y--;
    
    /* Clear new bottom line */
    int bottom_y = (term->screen_top_row + term->rows - 1) % term->max_rows;
    for (int x = 0; x < term->cols; x++) {
        TermCell* cell = &term->ring_buffer[bottom_y * MAX_TERM_COLS + x];
        cell->ch[0] = ' ';
        cell->ch[1] = '\0';
        cell->fg = term->default_fg;
        cell->bg = term->default_bg;
        cell->flags = 0;
    }
}

static void term_put_utf8(VaxpTerminal* term, const char* str) {
    if (term->cursor_x >= term->cols) {
        term->cursor_x = 0;
        term->cursor_y++;
        if (term->cursor_y >= term->rows) {
            term_scroll_up(term);
            term->cursor_y = term->rows - 1;
        }
    }
    
    TermCell* cell = get_cell(term, term->cursor_x, term->cursor_y);
    if (cell) {
        strncpy(cell->ch, str, 4);
        cell->ch[4] = '\0';
        cell->fg = term->current_fg;
        cell->bg = term->current_bg;
        cell->flags = term->current_flags;
    }
    term->cursor_x++;
}

static void term_putc(VaxpTerminal* term, char c) {
    if (c == '\r') {
        term->cursor_x = 0;
        return;
    } else if (c == '\n') {
        term->cursor_y++;
        if (term->cursor_y >= term->rows) {
            term_scroll_up(term);
            term->cursor_y = term->rows - 1;
        }
        return;
    } else if (c == '\b') {
        if (term->cursor_x > 0) {
            term->cursor_x--;
        }
        return;
    }
    
    /* UTF-8 Parsing */
    if (term->utf8_state == 0) {
        if ((c & 0x80) == 0) {
            if (c >= 32 && c <= 126) {
                char str[2] = {c, '\0'};
                term_put_utf8(term, str);
            }
        } else if ((c & 0xE0) == 0xC0) {
            term->utf8_state = 1;
            term->utf8_buf[0] = c;
            term->utf8_idx = 1;
        } else if ((c & 0xF0) == 0xE0) {
            term->utf8_state = 2;
            term->utf8_buf[0] = c;
            term->utf8_idx = 1;
        } else if ((c & 0xF8) == 0xF0) {
            term->utf8_state = 3;
            term->utf8_buf[0] = c;
            term->utf8_idx = 1;
        }
    } else {
        term->utf8_buf[term->utf8_idx++] = c;
        term->utf8_state--;
        if (term->utf8_state == 0) {
            term->utf8_buf[term->utf8_idx] = '\0';
            term_put_utf8(term, term->utf8_buf);
        }
    }
}

static void process_sgr(VaxpTerminal* term) {
    if (term->ansi_param_count == 0) {
        term->current_fg = term->default_fg;
        term->current_bg = term->default_bg;
        term->current_flags = 0;
        return;
    }
    for (int i = 0; i < term->ansi_param_count; i++) {
        int p = term->ansi_params[i];
        if (p == 0) {
            term->current_fg = term->default_fg;
            term->current_bg = term->default_bg;
            term->current_flags = 0;
        } else if (p == 1) {
            term->current_flags |= VAXP_TERM_ATTR_BOLD;
        } else if (p == 3) {
            term->current_flags |= VAXP_TERM_ATTR_ITALIC;
        } else if (p == 4) {
            term->current_flags |= VAXP_TERM_ATTR_UNDERLINE;
        } else if (p == 7) {
            term->current_flags |= VAXP_TERM_ATTR_INVERSE;
        } else if (p == 22) {
            term->current_flags &= ~VAXP_TERM_ATTR_BOLD;
        } else if (p == 23) {
            term->current_flags &= ~VAXP_TERM_ATTR_ITALIC;
        } else if (p == 24) {
            term->current_flags &= ~VAXP_TERM_ATTR_UNDERLINE;
        } else if (p == 27) {
            term->current_flags &= ~VAXP_TERM_ATTR_INVERSE;
        } else if (p >= 30 && p <= 37) {
            VaxpColor colors[8] = {
                vaxp_color_rgb(0, 0, 0), vaxp_color_rgb(205, 0, 0),
                vaxp_color_rgb(0, 205, 0), vaxp_color_rgb(205, 205, 0),
                vaxp_color_rgb(0, 0, 238), vaxp_color_rgb(205, 0, 205),
                vaxp_color_rgb(0, 205, 205), vaxp_color_rgb(229, 229, 229)
            };
            term->current_fg = colors[p - 30];
        } else if (p == 38) {
            if (i + 4 < term->ansi_param_count && term->ansi_params[i+1] == 2) {
                term->current_fg = vaxp_color_rgb(term->ansi_params[i+2], term->ansi_params[i+3], term->ansi_params[i+4]);
                i += 4;
            } else if (i + 2 < term->ansi_param_count && term->ansi_params[i+1] == 5) {
                i += 2;
            }
        } else if (p == 39) {
            term->current_fg = term->default_fg;
        } else if (p >= 40 && p <= 47) {
            VaxpColor colors[8] = {
                vaxp_color_rgb(0, 0, 0), vaxp_color_rgb(205, 0, 0),
                vaxp_color_rgb(0, 205, 0), vaxp_color_rgb(205, 205, 0),
                vaxp_color_rgb(0, 0, 238), vaxp_color_rgb(205, 0, 205),
                vaxp_color_rgb(0, 205, 205), vaxp_color_rgb(229, 229, 229)
            };
            term->current_bg = colors[p - 40];
        } else if (p == 48) {
            if (i + 4 < term->ansi_param_count && term->ansi_params[i+1] == 2) {
                term->current_bg = vaxp_color_rgb(term->ansi_params[i+2], term->ansi_params[i+3], term->ansi_params[i+4]);
                i += 4;
            } else if (i + 2 < term->ansi_param_count && term->ansi_params[i+1] == 5) {
                i += 2;
            }
        } else if (p == 49) {
            term->current_bg = term->default_bg;
        }
    }
}

void vaxp_terminal_write(VaxpTerminal* term, const char* data, int len) {
    for (int i = 0; i < len; i++) {
        char c = data[i];
        
        if (term->ansi_state == 0) { /* NORMAL */
            if (c == '\x1b') {
                term->ansi_state = 1; /* ESC */
            } else {
                term_putc(term, c);
            }
        } else if (term->ansi_state == 1) { /* ESC */
            if (c == '[') {
                term->ansi_state = 2; /* CSI */
                term->ansi_is_private = VAXP_FALSE;
                term->ansi_param_count = 0;
                term->ansi_param_idx = 0;
                term->ansi_current_param[0] = '\0';
            } else if (c == ']') {
                term->ansi_state = 3; /* OSC */
                term->ansi_param_idx = 0;
                term->ansi_current_param[0] = '\0';
            } else {
                term->ansi_state = 0;
            }
        } else if (term->ansi_state == 2) { /* CSI */
            if (c == '?') {
                term->ansi_is_private = VAXP_TRUE;
            } else if (c >= '0' && c <= '9') {
                if (term->ansi_param_idx < 15) {
                    term->ansi_current_param[term->ansi_param_idx++] = c;
                    term->ansi_current_param[term->ansi_param_idx] = '\0';
                }
            } else if (c == ';') {
                if (term->ansi_param_count < 16 && term->ansi_param_idx > 0) {
                    term->ansi_params[term->ansi_param_count++] = atoi(term->ansi_current_param);
                }
                term->ansi_param_idx = 0;
                term->ansi_current_param[0] = '\0';
            } else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
                if (term->ansi_param_idx > 0 && term->ansi_param_count < 16) {
                    term->ansi_params[term->ansi_param_count++] = atoi(term->ansi_current_param);
                }
                
                if (c == 'm') {
                    process_sgr(term);
                } else if (c == 'h' || c == 'l') {
                    if (term->ansi_is_private) {
                        int p = term->ansi_param_count > 0 ? term->ansi_params[0] : 0;
                        if (p == 1049) {
                            if (c == 'h') {
                                if (!term->use_alt_buffer) {
                                    term->use_alt_buffer = VAXP_TRUE;
                                    term->saved_cursor_x = term->cursor_x;
                                    term->saved_cursor_y = term->cursor_y;
                                    for(int cy = 0; cy < term->rows; cy++) {
                                        for(int cx = 0; cx < term->cols; cx++) {
                                            TermCell* cell = get_cell(term, cx, cy);
                                            if (cell) {
                                                cell->ch[0] = ' '; cell->ch[1] = '\0';
                                                cell->fg = term->default_fg; cell->bg = term->default_bg;
                                                cell->flags = 0;
                                            }
                                        }
                                    }
                                    term->cursor_x = 0; term->cursor_y = 0;
                                }
                            } else {
                                if (term->use_alt_buffer) {
                                    term->use_alt_buffer = VAXP_FALSE;
                                    term->cursor_x = term->saved_cursor_x;
                                    term->cursor_y = term->saved_cursor_y;
                                }
                            }
                        } else if (p == 25) {
                            term->cursor_visible = (c == 'h') ? VAXP_TRUE : VAXP_FALSE;
                        } else if (p == 1000) {
                            term->mouse_reporting_mode = (c == 'h') ? 1000 : 0;
                        } else if (p == 1002) {
                            term->mouse_reporting_mode = (c == 'h') ? 1002 : 0;
                        }
                    }
                } else if (c == 'J') {
                    int p = term->ansi_param_count > 0 ? term->ansi_params[0] : 0;
                    if (p == 2) {
                        for (int y = 0; y < term->rows; y++) {
                            for (int x = 0; x < term->cols; x++) {
                                TermCell* cell = get_cell(term, x, y);
                                if (cell) {
                                    cell->ch[0] = ' '; cell->ch[1] = '\0';
                                    cell->fg = term->default_fg; cell->bg = term->default_bg;
                                    cell->flags = 0;
                                }
                            }
                        }
                        term->cursor_x = 0; term->cursor_y = 0;
                    } else if (p == 0) {
                        for (int y = term->cursor_y; y < term->rows; y++) {
                            int start_x = (y == term->cursor_y) ? term->cursor_x : 0;
                            for (int x = start_x; x < term->cols; x++) {
                                TermCell* cell = get_cell(term, x, y);
                                if (cell) {
                                    cell->ch[0] = ' '; cell->ch[1] = '\0';
                                    cell->fg = term->default_fg; cell->bg = term->default_bg;
                                    cell->flags = 0;
                                }
                            }
                        }
                    }
                } else if (c == 'K') {
                    int p = term->ansi_param_count > 0 ? term->ansi_params[0] : 0;
                    if (p == 0) {
                        for (int x = term->cursor_x; x < term->cols; x++) {
                            TermCell* cell = get_cell(term, x, term->cursor_y);
                            if (cell) {
                                cell->ch[0] = ' '; cell->ch[1] = '\0';
                                cell->fg = term->default_fg; cell->bg = term->default_bg;
                                cell->flags = 0;
                            }
                        }
                    } else if (p == 2) {
                        for (int x = 0; x < term->cols; x++) {
                            TermCell* cell = get_cell(term, x, term->cursor_y);
                            if (cell) {
                                cell->ch[0] = ' '; cell->ch[1] = '\0';
                                cell->fg = term->default_fg; cell->bg = term->default_bg;
                                cell->flags = 0;
                            }
                        }
                    }
                } else if (c == 'H' || c == 'f') {
                    int row = term->ansi_param_count > 0 ? term->ansi_params[0] : 1;
                    int col = term->ansi_param_count > 1 ? term->ansi_params[1] : 1;
                    term->cursor_y = (row > 0 ? row - 1 : 0);
                    term->cursor_x = (col > 0 ? col - 1 : 0);
                    if (term->cursor_y >= term->rows) term->cursor_y = term->rows - 1;
                    if (term->cursor_x >= term->cols) term->cursor_x = term->cols - 1;
                } else if (c == 'A') {
                    int n = term->ansi_param_count > 0 ? term->ansi_params[0] : 1;
                    term->cursor_y -= n;
                    if (term->cursor_y < 0) term->cursor_y = 0;
                } else if (c == 'B') {
                    int n = term->ansi_param_count > 0 ? term->ansi_params[0] : 1;
                    term->cursor_y += n;
                    if (term->cursor_y >= term->rows) term->cursor_y = term->rows - 1;
                } else if (c == 'C') {
                    int n = term->ansi_param_count > 0 ? term->ansi_params[0] : 1;
                    term->cursor_x += n;
                    if (term->cursor_x >= term->cols) term->cursor_x = term->cols - 1;
                } else if (c == 'D') {
                    int n = term->ansi_param_count > 0 ? term->ansi_params[0] : 1;
                    term->cursor_x -= n;
                    if (term->cursor_x < 0) term->cursor_x = 0;
                }
                
                term->ansi_state = 0;
            }
        } else if (term->ansi_state == 3) { /* OSC */
            if (c == '\a' || c == '\x07' || (c == '\\' && i > 0 && data[i-1] == '\x1b')) {
                term->ansi_state = 0;
                term->ansi_current_param[term->ansi_param_idx] = '\0';
                if (strncmp(term->ansi_current_param, "0;", 2) == 0 || strncmp(term->ansi_current_param, "2;", 2) == 0) {
                    if (term->on_title_changed) {
                        term->on_title_changed(term->title_user_data, term->ansi_current_param + 2);
                    }
                }
            } else if (c != '\x1b') {
                if (term->ansi_param_idx < 255) {
                    term->ansi_current_param[term->ansi_param_idx++] = c;
                }
            }
        }
    }
    vaxp_widget_invalidate((VaxpWidget*)term);
}

static VaxpBool is_cell_selected(VaxpTerminal* term, int x, int y) {
    if (term->sel_start_x < 0 || term->sel_start_y < 0) return VAXP_FALSE;
    int sy = term->sel_start_y, ey = term->sel_end_y;
    int sx = term->sel_start_x, ex = term->sel_end_x;
    if (sy > ey || (sy == ey && sx > ex)) {
        sy = term->sel_end_y; sx = term->sel_end_x;
        ey = term->sel_start_y; ex = term->sel_start_x;
    }
    if (y > sy && y < ey) return VAXP_TRUE;
    if (sy == ey) return y == sy && x >= sx && x <= ex;
    if (y == sy) return x >= sx;
    if (y == ey) return x <= ex;
    return VAXP_FALSE;
}

static void term_draw(VaxpWidget* self, VaxpCanvas* canvas) {
    VaxpTerminal* term = (VaxpTerminal*)self;
    VaxpRectF b = self->bounds;
    
    VaxpPaint bg_fill = vaxp_paint_fill(term->default_bg);
    vaxp_canvas_draw_rect(canvas, b, &bg_fill);
    
    float cell_h = term->font_size * 1.5f;
    float cell_w = term->font_size * 0.65f;
    term->char_width = cell_w;
    term->char_height = cell_h;
    
    /* Handle Dynamic Resize */
    int new_cols = (int)(b.width / cell_w);
    int new_rows = (int)(b.height / cell_h);
    if (new_cols > 0 && new_rows > 0 && new_cols <= MAX_TERM_COLS && new_rows <= MAX_TERM_ROWS) {
        if (new_cols != term->cols || new_rows != term->rows) {
            
            if (new_cols > term->cols) {
                for (int y = 0; y < term->max_rows; y++) {
                    for (int x = term->cols; x < new_cols; x++) {
                        TermCell* cell = &term->ring_buffer[y * MAX_TERM_COLS + x];
                        cell->ch[0] = ' '; cell->ch[1] = '\0';
                        cell->fg = term->default_fg; cell->bg = term->default_bg;
                        cell->flags = 0;
                    }
                }
            }
            
            if (term->cursor_y >= new_rows) {
                int shift = term->cursor_y - new_rows + 1;
                term->screen_top_row = (term->screen_top_row + shift) % term->max_rows;
                term->cursor_y = new_rows - 1;
            }
            
            term->cols = new_cols;
            term->rows = new_rows;
            if (term->cursor_x >= term->cols) term->cursor_x = term->cols - 1;
            if (term->cursor_y >= term->rows) term->cursor_y = term->rows - 1;
            
            if (term->pty_fd >= 0) {
                struct winsize ws;
                ws.ws_col = term->cols;
                ws.ws_row = term->rows;
                ws.ws_xpixel = 0;
                ws.ws_ypixel = 0;
                ioctl(term->pty_fd, TIOCSWINSZ, &ws);
            }
        }
    }
    
    for (int y = 0; y < term->rows; y++) {
        for (int x = 0; x < term->cols; x++) {
            TermCell* cell = get_cell(term, x, y);
            if (!cell) continue;
            
            float cx = x * cell_w;
            float cy = y * cell_h;
            
            VaxpColor c_fg = cell->fg;
            VaxpColor c_bg = cell->bg;
            if (cell->flags & VAXP_TERM_ATTR_INVERSE) {
                c_fg = cell->bg;
                c_bg = cell->fg;
            }
            
            if (is_cell_selected(term, x, y)) {
                c_bg = vaxp_color_rgba(50, 100, 200, 255);
            }
            
            if (c_bg.r != term->default_bg.r || c_bg.g != term->default_bg.g || 
                c_bg.b != term->default_bg.b || c_bg.a != term->default_bg.a || is_cell_selected(term, x, y)) {
                VaxpPaint bg_paint = vaxp_paint_fill(c_bg);
                vaxp_canvas_draw_rect(canvas, (VaxpRectF){cx, cy, cell_w, cell_h}, &bg_paint);
            }
            
            if (cell->ch[0] != ' ' && cell->ch[0] != '\0') {
                VaxpPaint text_paint = vaxp_paint_fill(c_fg);
                VaxpBool bold = (cell->flags & VAXP_TERM_ATTR_BOLD) ? VAXP_TRUE : VAXP_FALSE;
                VaxpBool italic = (cell->flags & VAXP_TERM_ATTR_ITALIC) ? VAXP_TRUE : VAXP_FALSE;
                VaxpFont font = {.family = "monospace", .size = term->font_size, .bold = bold, .italic = italic};
                vaxp_canvas_draw_text(canvas, cell->ch, cx, cy + cell_h - 4, &font, &text_paint);
                
                if (cell->flags & VAXP_TERM_ATTR_UNDERLINE) {
                    vaxp_canvas_draw_rect(canvas, (VaxpRectF){cx, cy + cell_h - 2, cell_w, 1}, &text_paint);
                }
            }
        }
    }
    
    if (term->cursor_visible && term->scroll_offset == 0 && vaxp_widget_has_state(self, VAXP_WIDGET_STATE_FOCUSED)) {
        float cx = term->cursor_x * cell_w;
        float cy = term->cursor_y * cell_h;
        VaxpPaint cursor_paint = vaxp_paint_fill(vaxp_color_rgba(255, 255, 255, 128));
        vaxp_canvas_draw_rect(canvas, (VaxpRectF){cx, cy, cell_w, cell_h}, &cursor_paint);
    }
    
    if (term->context_menu && ((VaxpContextMenu*)term->context_menu)->is_open) {
        term->context_menu->klass->draw(term->context_menu, canvas);
    }
}

static void term_copy_selection(VaxpTerminal* term) {
    if (term->sel_start_y < 0) return;
    
    int sy = term->sel_start_y, ey = term->sel_end_y;
    int sx = term->sel_start_x, ex = term->sel_end_x;
    if (sy > ey || (sy == ey && sx > ex)) {
        sy = term->sel_end_y; sx = term->sel_end_x;
        ey = term->sel_start_y; ex = term->sel_start_x;
    }
    
    char* buffer = malloc((ey - sy + 1) * term->cols * 5 + 1);
    buffer[0] = '\0';
    int buf_idx = 0;
    
    for (int y = sy; y <= ey; y++) {
        int start_x = (y == sy) ? sx : 0;
        int end_x = (y == ey) ? ex : term->cols - 1;
        int last_char_x = -1;
        for (int x = term->cols - 1; x >= start_x; x--) {
            TermCell* cell = get_cell(term, x, y);
            if (cell && cell->ch[0] != ' ' && cell->ch[0] != '\0') {
                last_char_x = x; break;
            }
        }
        if (last_char_x < end_x) end_x = last_char_x;
        if (end_x < start_x) end_x = start_x - 1;
        
        for (int x = start_x; x <= end_x; x++) {
            TermCell* cell = get_cell(term, x, y);
            if (cell && cell->ch[0] != '\0') {
                strcpy(&buffer[buf_idx], cell->ch);
                buf_idx += strlen(cell->ch);
            } else {
                buffer[buf_idx++] = ' ';
            }
        }
        if (y < ey) {
            buffer[buf_idx++] = '\n';
        }
    }
    buffer[buf_idx] = '\0';
    vaxp_clipboard_copy(buffer);
    free(buffer);
}

static void term_menu_on_copy(VaxpMenuItem* item, void* user_data) {
    VaxpTerminal* term = (VaxpTerminal*)user_data;
    term_copy_selection(term);
}

static void term_menu_on_paste(VaxpMenuItem* item, void* user_data) {
    VaxpTerminal* term = (VaxpTerminal*)user_data;
    char* text = NULL;
    if (vaxp_clipboard_paste(&text).ok && text) {
        if (term->pty_fd >= 0) (void)write(term->pty_fd, text, strlen(text));
        free(text);
    }
}

static VaxpBool term_on_event(VaxpWidget* self, const VaxpEvent* event) {
    VaxpTerminal* term = (VaxpTerminal*)self;
    
    if (term->context_menu && ((VaxpContextMenu*)term->context_menu)->is_open) {
        if (term->context_menu->klass->on_event(term->context_menu, event)) {
            return VAXP_TRUE;
        }
    }
    
    if (event->type == VAXP_EVENT_MOUSE_SCROLL) {
        if (!term->use_alt_buffer) {
            int lines = (int)(event->scroll.delta_y * 3);
            term->scroll_offset -= lines;
            if (term->scroll_offset < 0) term->scroll_offset = 0;
            if (term->scroll_offset > term->history_count) {
                term->scroll_offset = term->history_count;
            }
            vaxp_widget_invalidate(self);
            return VAXP_TRUE;
        }
    }
    
    if (event->type == VAXP_EVENT_MOUSE_BUTTON_DOWN) {
        vaxp_focus_set(self);
        int cx = (int)(event->mouse.x / term->char_width);
        int cy = (int)(event->mouse.y / term->char_height);
        
        if (event->mouse.button == VAXP_MOUSE_BUTTON_MIDDLE) {
            char* text = NULL;
            if (vaxp_clipboard_paste(&text).ok && text) {
                if (term->pty_fd >= 0) (void)write(term->pty_fd, text, strlen(text));
                free(text);
            }
            return VAXP_TRUE;
        }
        
        if (event->mouse.button == VAXP_MOUSE_BUTTON_RIGHT) {
            if (term->mouse_reporting_mode > 0 && term->pty_fd >= 0) {
                char buf[32];
                snprintf(buf, sizeof(buf), "\x1b[M%c%c%c", (char)(32 + 2), (char)(32 + cx + 1), (char)(32 + cy + 1));
                (void)write(term->pty_fd, buf, strlen(buf));
            } else {
                if (!term->context_menu) {
                    VaxpResultPtr res = vaxp_context_menu_create();
                    if (res.ok) {
                        term->context_menu = (VaxpWidget*)res.value;
                        vaxp_context_menu_add_item((VaxpContextMenu*)term->context_menu, "Copy (Ctrl+Shift+C)", term_menu_on_copy, term);
                        vaxp_context_menu_add_item((VaxpContextMenu*)term->context_menu, "Paste (Ctrl+Shift+V)", term_menu_on_paste, term);
                    }
                }
                if (term->context_menu) {
                    /* Show context menu at mouse location */
                    vaxp_context_menu_show((VaxpContextMenu*)term->context_menu, event->mouse.x, event->mouse.y);
                }
            }
            return VAXP_TRUE;
        }
        
        if (term->mouse_reporting_mode > 0 && term->pty_fd >= 0) {
            char buf[32];
            int btn = (event->mouse.button == VAXP_MOUSE_BUTTON_RIGHT) ? 2 : 0;
            snprintf(buf, sizeof(buf), "\x1b[M%c%c%c", (char)(32 + btn), (char)(32 + cx + 1), (char)(32 + cy + 1));
            (void)write(term->pty_fd, buf, strlen(buf));
        } else {
            term->is_selecting = VAXP_TRUE;
            term->sel_start_x = cx; term->sel_start_y = cy;
            term->sel_end_x = cx; term->sel_end_y = cy;
            vaxp_widget_invalidate(self);
        }
        return VAXP_TRUE;
    }
    
    if (event->type == VAXP_EVENT_MOUSE_MOVE) {
        int cx = (int)(event->mouse.x / term->char_width);
        int cy = (int)(event->mouse.y / term->char_height);
        if (term->mouse_reporting_mode == 1002 && term->pty_fd >= 0) {
            char buf[32];
            snprintf(buf, sizeof(buf), "\x1b[M%c%c%c", (char)(32 + 32), (char)(32 + cx + 1), (char)(32 + cy + 1));
            (void)write(term->pty_fd, buf, strlen(buf));
        } else if (term->is_selecting) {
            term->sel_end_x = cx; term->sel_end_y = cy;
            vaxp_widget_invalidate(self);
        }
    }
    
    if (event->type == VAXP_EVENT_MOUSE_BUTTON_UP) {
        if (term->mouse_reporting_mode > 0 && term->pty_fd >= 0) {
            int cx = (int)(event->mouse.x / term->char_width);
            int cy = (int)(event->mouse.y / term->char_height);
            char buf[32];
            snprintf(buf, sizeof(buf), "\x1b[M%c%c%c", (char)(32 + 3), (char)(32 + cx + 1), (char)(32 + cy + 1));
            (void)write(term->pty_fd, buf, strlen(buf));
        } else if (term->is_selecting) {
            term->is_selecting = VAXP_FALSE;
            term_copy_selection(term);
            vaxp_widget_invalidate(self);
        }
    }
    
    if (event->type == VAXP_EVENT_KEY_DOWN && term->pty_fd >= 0) {
        term->scroll_offset = 0;
        
        if ((event->key.modifiers & VAXP_KEYMOD_CTRL) && (event->key.modifiers & VAXP_KEYMOD_SHIFT)) {
            if (event->key.key == 'C' || event->key.key == 'c') {
                term_copy_selection(term);
                return VAXP_TRUE;
            } else if (event->key.key == 'V' || event->key.key == 'v') {
                char* text = NULL;
                if (vaxp_clipboard_paste(&text).ok && text) {
                    (void)write(term->pty_fd, text, strlen(text));
                    free(text);
                }
                return VAXP_TRUE;
            }
        }
        
        if (event->key.modifiers & VAXP_KEYMOD_CTRL) {
            char c = 0;
            if (event->key.key >= 'a' && event->key.key <= 'z') c = event->key.key - 'a' + 1;
            else if (event->key.key >= 'A' && event->key.key <= 'Z') c = event->key.key - 'A' + 1;
            else if (event->key.key == VAXP_KEY_SPACE) c = 0;
            else if (event->key.key == '\\') c = 28;
            else if (event->key.key == ']') c = 29;
            else if (event->key.key == '^') c = 30;
            else if (event->key.key == '_') c = 31;
            if (c > 0) {
                (void)write(term->pty_fd, &c, 1);
                term->sel_start_y = -1; vaxp_widget_invalidate(self);
                return VAXP_TRUE;
            }
        }
        
        if (event->key.key == VAXP_KEY_RETURN) {
            (void)write(term->pty_fd, "\n", 1);
            term->sel_start_y = -1; vaxp_widget_invalidate(self);
            return VAXP_TRUE;
        } else if (event->key.key == VAXP_KEY_BACKSPACE) {
            (void)write(term->pty_fd, "\b", 1);
            term->sel_start_y = -1; vaxp_widget_invalidate(self);
            return VAXP_TRUE;
        } else if (event->key.key == VAXP_KEY_ESCAPE) {
            (void)write(term->pty_fd, "\x1b", 1);
            term->sel_start_y = -1; vaxp_widget_invalidate(self);
            return VAXP_TRUE;
        } else {
            if (event->text.text[0] != '\0' && (unsigned char)event->text.text[0] >= 32) {
                (void)write(term->pty_fd, event->text.text, strlen(event->text.text));
                term->sel_start_y = -1; vaxp_widget_invalidate(self);
                return VAXP_TRUE;
            }
        }
    }
    
    return VAXP_FALSE;
}

VaxpResultPtr vaxp_terminal_create(void) {
    return vaxp_widget_create(&vaxp_terminal_class);
}

VaxpWidget* _vaxp_terminal_build(const VaxpTerminalConfig* config) {
    VaxpResultPtr res = vaxp_terminal_create();
    if (!res.ok) return NULL;
    
    VaxpTerminal* term = (VaxpTerminal*)res.value;
    vaxp_terminal_init_grid(term, config->cols, config->rows);
    term->font_size = config->font_size > 0 ? config->font_size : 14.0f;
    term->pty_fd = config->pty_fd;
    term->on_title_changed = config->on_title_changed;
    term->title_user_data = config->title_user_data;
    
    ((VaxpWidget*)term)->layout.flex_grow = 1;
    ((VaxpWidget*)term)->layout.flex_shrink = 1;
    ((VaxpWidget*)term)->layout.align_self = VAXP_ALIGN_STRETCH;
    
    return (VaxpWidget*)term;
}
