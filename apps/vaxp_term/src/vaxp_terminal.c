#define _GNU_SOURCE
#include "vaxp_terminal.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <termios.h>
#include "vaxp/widgets/vaxp_context_menu.h"
#include "vaxp/widgets/vaxp_image.h"

#define MAX_TERM_COLS 512
#define MAX_TERM_ROWS 256

static void term_init(VaxpWidget* self);
static void term_destroy(VaxpWidget* self);
static void term_draw(VaxpWidget* self, VaxpCanvas* canvas);
static VaxpBool term_on_event(VaxpWidget* self, const VaxpEvent* event);

/* Simple Base64 Decoder */
static const int b64_index[256] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 62, 63, 62, 62, 63,
   52, 53, 54, 55, 56, 57, 58, 59, 60, 61,  0,  0,  0,  0,  0,  0,
    0,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
   15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,  0,  0,  0,  0, 63,
    0, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
   41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
};

static unsigned char* base64_decode(const char* data, size_t input_length, size_t* output_length) {
    if (input_length == 0) { *output_length = 0; return NULL; }
    if (input_length % 4 != 0) { *output_length = 0; return NULL; }
    size_t output_len = input_length / 4 * 3;
    if (data[input_length - 1] == '=') output_len--;
    if (data[input_length - 2] == '=') output_len--;
    
    unsigned char* decoded = (unsigned char*)malloc(output_len);
    if (!decoded) return NULL;
    
    for (size_t i = 0, j = 0; i < input_length;) {
        uint32_t a = data[i] == '=' ? 0 & i++ : b64_index[(unsigned char)data[i++]];
        uint32_t b = data[i] == '=' ? 0 & i++ : b64_index[(unsigned char)data[i++]];
        uint32_t c = data[i] == '=' ? 0 & i++ : b64_index[(unsigned char)data[i++]];
        uint32_t d = data[i] == '=' ? 0 & i++ : b64_index[(unsigned char)data[i++]];
        
        uint32_t triple = (a << 18) + (b << 12) + (c << 6) + d;
        if (j < output_len) decoded[j++] = (triple >> 16) & 0xFF;
        if (j < output_len) decoded[j++] = (triple >> 8) & 0xFF;
        if (j < output_len) decoded[j++] = (triple) & 0xFF;
    }
    
    *output_length = output_len;
    return decoded;
}

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
    
    term->ansi_payload = NULL;
    term->ansi_payload_len = 0;
    term->ansi_payload_cap = 0;
    term->images = NULL;
    
    self->focusable = VAXP_TRUE;
}

static void term_destroy(VaxpWidget* self) {
    VaxpTerminal* term = (VaxpTerminal*)self;
    if (term->ring_buffer) free(term->ring_buffer);
    if (term->alt_buffer) free(term->alt_buffer);
    if (term->context_menu) vaxp_unref(term->context_menu);
    if (term->ansi_payload) free(term->ansi_payload);
    
    TermImage* img = term->images;
    while (img) {
        TermImage* next = img->next;
        if (img->image) vaxp_unref(img->image);
        free(img);
        img = next;
    }
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
    
    /* Shift images up */
    TermImage** img_ptr = &term->images;
    while (*img_ptr) {
        TermImage* img = *img_ptr;
        img->y--;
        if (img->y + img->height_cells <= -term->max_rows) {
            *img_ptr = img->next;
            if (img->image) vaxp_unref(img->image);
            free(img);
        } else {
            img_ptr = &img->next;
        }
    }
    
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

static void term_scroll_down(VaxpTerminal* term) {
    if (term->use_alt_buffer && term->alt_buffer) {
        int row_bytes = MAX_TERM_COLS * sizeof(TermCell);
        memmove(term->alt_buffer + MAX_TERM_COLS, term->alt_buffer, row_bytes * (term->rows - 1));
        TermCell* first_row = term->alt_buffer;
        for (int i = 0; i < term->cols; i++) {
            first_row[i].ch[0] = ' ';
            first_row[i].ch[1] = '\0';
            first_row[i].fg = term->default_fg;
            first_row[i].bg = term->default_bg;
            first_row[i].flags = 0;
        }
        return;
    }
    
    if (!term->ring_buffer) return;
    
    term->screen_top_row = (term->screen_top_row - 1 + term->max_rows) % term->max_rows;
    if (term->history_count > 0) {
        term->history_count--;
    }
    
    if (term->scroll_offset > 0) {
        term->scroll_offset--;
    }
    
    if (term->sel_start_y >= 0) term->sel_start_y++;
    if (term->sel_end_y >= 0) term->sel_end_y++;
    
    /* Shift images down */
    TermImage** img_ptr = &term->images;
    while (*img_ptr) {
        TermImage* img = *img_ptr;
        img->y++;
        if (img->y >= term->max_rows) {
            *img_ptr = img->next;
            if (img->image) vaxp_unref(img->image);
            free(img);
        } else {
            img_ptr = &img->next;
        }
    }
    
    /* Clear new top line */
    int top_y = term->screen_top_row;
    for (int x = 0; x < term->cols; x++) {
        TermCell* cell = &term->ring_buffer[top_y * MAX_TERM_COLS + x];
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
        if (term->use_acs && str[1] == '\0' && str[0] >= 96 && str[0] <= 126) {
            const char* utf8 = " ";
            switch(str[0]) {
                case 'x': utf8 = "\xE2\x94\x82"; break; /* │ */
                case 'q': utf8 = "\xE2\x94\x80"; break; /* ─ */
                case 'l': utf8 = "\xE2\x94\x8C"; break; /* ┌ */
                case 'm': utf8 = "\xE2\x94\x94"; break; /* └ */
                case 'k': utf8 = "\xE2\x94\x90"; break; /* ┐ */
                case 'j': utf8 = "\xE2\x94\x98"; break; /* ┘ */
                case 'a': utf8 = "\xE2\x96\x92"; break; /* ▒ */
                case 't': utf8 = "\xE2\x94\x9C"; break; /* ├ */
                case 'u': utf8 = "\xE2\x94\xA4"; break; /* ┤ */
                case 'v': utf8 = "\xE2\x94\xB4"; break; /* ┴ */
                case 'w': utf8 = "\xE2\x94\xAC"; break; /* ┬ */
                case 'n': utf8 = "\xE2\x95\xBC"; break; /* ┼ */
                default: utf8 = str; break;
            }
            strncpy(cell->ch, utf8, 4);
        } else {
            strncpy(cell->ch, str, 4);
        }
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
    } else if (c == '\t') {
        term->cursor_x = (term->cursor_x + 8) & ~7;
        if (term->cursor_x >= term->cols) term->cursor_x = term->cols - 1;
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

static void process_kitty_image(VaxpTerminal* term, const char* payload, int len) {
    /* Payload starts with 'G' or 'G ' for Kitty Image Protocol */
    if (len < 2 || payload[0] != 'G') return;
    
    const char* p = payload + 1;
    if (*p == ' ') p++;
    
    /* Parse key=value pairs up to ';' */
    int format = 0;
    int a = 0;
    int id = 0;
    
    while (*p && *p != ';') {
        char key = *p++;
        if (*p == '=') p++;
        int val = 0;
        while (*p >= '0' && *p <= '9') {
            val = val * 10 + (*p - '0');
            p++;
        }
        if (key == 'a') a = val;
        else if (key == 'f') format = val;
        else if (key == 'i') id = val;
        
        if (*p == ',') p++;
    }
    
    if (*p == ';') p++;
    
    /* Decode base64 */
    size_t out_len = 0;
    size_t input_len = strlen(p);
    fprintf(stderr, "VAXP_TERM DEBUG: process_kitty_image format=%d a=%d id=%d base64_len=%zu\n", format, a, id, input_len);
    
    unsigned char* decoded = base64_decode(p, input_len, &out_len);
    if (!decoded) {
        fprintf(stderr, "VAXP_TERM DEBUG: base64_decode failed (len mod 4 = %zu)\n", input_len % 4);
        return;
    }
    
    if (decoded && out_len > 0) {
        fprintf(stderr, "VAXP_TERM DEBUG: base64_decode success, out_len=%zu\n", out_len);
        /* Only support PNG (f=100) and RGB (f=24) / RGBA (f=32) for now */
        if (format == 100 || format == 24 || format == 32) {
            char tmp_path[256];
            snprintf(tmp_path, sizeof(tmp_path), "/tmp/vaxp_img_%d.png", id);
            FILE* f = fopen(tmp_path, "wb");
            if (f) {
                fwrite(decoded, 1, out_len, f);
                fclose(f);
                fprintf(stderr, "VAXP_TERM DEBUG: Wrote decoded data to %s\n", tmp_path);
                
                VaxpResultPtr res = vaxp_image_load_file(tmp_path);
                if (res.ok && res.value) {
                    VaxpImageData* img_data = (VaxpImageData*)res.value;
                    VaxpU32 w = 0, h = 0;
                    vaxp_image_get_size(img_data, &w, &h);
                    fprintf(stderr, "VAXP_TERM DEBUG: Image loaded. size=%ux%u\n", w, h);
                    
                    TermImage* timg = malloc(sizeof(TermImage));
                    timg->id = id;
                    timg->image = (VaxpImage*)img_data;
                    timg->x = term->cursor_x;
                    timg->y = term->cursor_y; /* Relative to current screen top */
                    
                    /* Calculate size in cells */
                    timg->width_cells = (int)(w / term->char_width) + 1;
                    timg->height_cells = (int)(h / term->char_height) + 1;
                    fprintf(stderr, "VAXP_TERM DEBUG: Added image to list at %d,%d (%dx%d cells)\n", timg->x, timg->y, timg->width_cells, timg->height_cells);
                    
                    timg->next = term->images;
                    term->images = timg;
                } else {
                    fprintf(stderr, "VAXP_TERM DEBUG: vaxp_image_load_file failed! res.ok=%d error=%d\n", res.ok, res.error);
                }
                unlink(tmp_path);
            } else {
                fprintf(stderr, "VAXP_TERM DEBUG: Failed to open %s for writing\n", tmp_path);
            }
        } else {
            fprintf(stderr, "VAXP_TERM DEBUG: Unsupported format %d\n", format);
        }
        free(decoded);
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
            } else if (c == '_') {
                term->ansi_state = 5; /* APC */
                term->ansi_payload_len = 0;
            } else if (c == 'P') {
                term->ansi_state = 6; /* DCS */
                term->ansi_payload_len = 0;
            } else if (c == '(' || c == ')') {
                term->ansi_state = 4; /* Charset */
            } else if (c == '7') {
                term->saved_cursor_x = term->cursor_x;
                term->saved_cursor_y = term->cursor_y;
                term->ansi_state = 0;
            } else if (c == '8') {
                term->cursor_x = term->saved_cursor_x;
                term->cursor_y = term->saved_cursor_y;
                term->ansi_state = 0;
            } else if (c == 'D') {
                term->cursor_y++;
                if (term->cursor_y >= term->rows) {
                    term_scroll_up(term);
                    term->cursor_y = term->rows - 1;
                }
                term->ansi_state = 0;
            } else if (c == 'M') {
                term->cursor_y--;
                if (term->cursor_y < 0) {
                    term_scroll_down(term);
                    term->cursor_y = 0;
                }
                term->ansi_state = 0;
            } else if (c == 'Z') {
                if (term->pty_fd >= 0) {
                    (void)write(term->pty_fd, "\x1b[?1;2c", 7);
                }
                term->ansi_state = 0;
            } else {
                term->ansi_state = 0;
            }
        } else if (term->ansi_state == 4) { /* Charset */
            if (c == '0') term->use_acs = VAXP_TRUE;
            else if (c == 'B') term->use_acs = VAXP_FALSE;
            term->ansi_state = 0;
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
                } else if (c == 'G' || c == '`') {
                    int col = term->ansi_param_count > 0 ? term->ansi_params[0] : 1;
                    term->cursor_x = (col > 0 ? col - 1 : 0);
                    if (term->cursor_x >= term->cols) term->cursor_x = term->cols - 1;
                } else if (c == 'd') {
                    int row = term->ansi_param_count > 0 ? term->ansi_params[0] : 1;
                    term->cursor_y = (row > 0 ? row - 1 : 0);
                    if (term->cursor_y >= term->rows) term->cursor_y = term->rows - 1;
                } else if (c == 's') {
                    term->saved_cursor_x = term->cursor_x;
                    term->saved_cursor_y = term->cursor_y;
                } else if (c == 'u') {
                    term->cursor_x = term->saved_cursor_x;
                    term->cursor_y = term->saved_cursor_y;
                } else if (c == 'P') {
                    int n = term->ansi_param_count > 0 ? term->ansi_params[0] : 1;
                    for (int x = term->cursor_x; x < term->cols - n; x++) {
                        TermCell* dst = get_cell(term, x, term->cursor_y);
                        TermCell* src = get_cell(term, x + n, term->cursor_y);
                        if (dst && src) *dst = *src;
                    }
                    for (int x = term->cols - n; x < term->cols; x++) {
                        TermCell* cell = get_cell(term, x, term->cursor_y);
                        if (cell) {
                            cell->ch[0] = ' '; cell->ch[1] = '\0';
                            cell->fg = term->default_fg; cell->bg = term->default_bg;
                            cell->flags = 0;
                        }
                    }
                } else if (c == 'L') {
                    int n = term->ansi_param_count > 0 ? term->ansi_params[0] : 1;
                    for (int y = term->rows - 1; y >= term->cursor_y + n; y--) {
                        for (int x = 0; x < term->cols; x++) {
                            TermCell* dst = get_cell(term, x, y);
                            TermCell* src = get_cell(term, x, y - n);
                            if (dst && src) *dst = *src;
                        }
                    }
                    for (int y = term->cursor_y; y < term->cursor_y + n && y < term->rows; y++) {
                        for (int x = 0; x < term->cols; x++) {
                            TermCell* cell = get_cell(term, x, y);
                            if (cell) {
                                cell->ch[0] = ' '; cell->ch[1] = '\0';
                                cell->fg = term->default_fg; cell->bg = term->default_bg;
                                cell->flags = 0;
                            }
                        }
                    }
                } else if (c == 'M') {
                    int n = term->ansi_param_count > 0 ? term->ansi_params[0] : 1;
                    for (int y = term->cursor_y; y < term->rows - n; y++) {
                        for (int x = 0; x < term->cols; x++) {
                            TermCell* dst = get_cell(term, x, y);
                            TermCell* src = get_cell(term, x, y + n);
                            if (dst && src) *dst = *src;
                        }
                    }
                    for (int y = term->rows - n; y < term->rows; y++) {
                        for (int x = 0; x < term->cols; x++) {
                            TermCell* cell = get_cell(term, x, y);
                            if (cell) {
                                cell->ch[0] = ' '; cell->ch[1] = '\0';
                                cell->fg = term->default_fg; cell->bg = term->default_bg;
                                cell->flags = 0;
                            }
                        }
                    }
                } else if (c == 'c') {
                    if (term->pty_fd >= 0) {
                        /* Primary DA (Device Attributes) response: VT100 with Advanced Video Option */
                        (void)write(term->pty_fd, "\x1b[?1;2c", 7);
                    }
                } else if (c == 'n') {
                    if (term->pty_fd >= 0) {
                        int p = term->ansi_param_count > 0 ? term->ansi_params[0] : 0;
                        if (p == 5) {
                            /* DSR: Operating Status -> OK */
                            (void)write(term->pty_fd, "\x1b[0n", 4);
                        } else if (p == 6) {
                            /* CPR: Cursor Position Report */
                            char buf[32];
                            snprintf(buf, sizeof(buf), "\x1b[%d;%dR", term->cursor_y + 1, term->cursor_x + 1);
                            (void)write(term->pty_fd, buf, strlen(buf));
                        }
                    }
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
        } else if (term->ansi_state == 5) { /* APC (Kitty) */
            if (c == '\a' || c == '\x07') {
                term->ansi_state = 0;
                if (term->ansi_payload_len > 0) {
                    process_kitty_image(term, term->ansi_payload, term->ansi_payload_len);
                }
                term->ansi_payload_len = 0;
            } else if (c == '\x1b') {
                term->ansi_state = 51;
            } else {
                if (term->ansi_payload_len >= term->ansi_payload_cap - 1) {
                    term->ansi_payload_cap = term->ansi_payload_cap == 0 ? 1024 : term->ansi_payload_cap * 2;
                    term->ansi_payload = realloc(term->ansi_payload, term->ansi_payload_cap);
                }
                if (term->ansi_payload) {
                    term->ansi_payload[term->ansi_payload_len++] = c;
                    term->ansi_payload[term->ansi_payload_len] = '\0';
                }
            }
        } else if (term->ansi_state == 51) {
            if (c == '\\') {
                term->ansi_state = 0;
                if (term->ansi_payload_len > 0) {
                    process_kitty_image(term, term->ansi_payload, term->ansi_payload_len);
                }
                term->ansi_payload_len = 0;
            } else {
                /* Not a terminator, add ESC and c to payload */
                if (term->ansi_payload_len + 1 >= term->ansi_payload_cap - 1) {
                    term->ansi_payload_cap = term->ansi_payload_cap == 0 ? 1024 : term->ansi_payload_cap * 2;
                    term->ansi_payload = realloc(term->ansi_payload, term->ansi_payload_cap);
                }
                if (term->ansi_payload) {
                    term->ansi_payload[term->ansi_payload_len++] = '\x1b';
                    term->ansi_payload[term->ansi_payload_len++] = c;
                    term->ansi_payload[term->ansi_payload_len] = '\0';
                }
                term->ansi_state = 5;
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
    
    char row_str[MAX_TERM_COLS + 1];
    char match_mask[MAX_TERM_COLS];
    
    for (int y = 0; y < term->rows; y++) {
        memset(match_mask, 0, term->cols);
        if (term->search_active && term->search_query_len > 0) {
            /* Build row string for searching */
            for (int x = 0; x < term->cols; x++) {
                TermCell* cell = get_cell(term, x, y);
                row_str[x] = (cell && cell->ch[0] != '\0') ? cell->ch[0] : ' ';
            }
            row_str[term->cols] = '\0';
            
            /* Case-insensitive search using strcasestr */
            char* p = row_str;
            while ((p = strcasestr(p, term->search_query)) != NULL) {
                int idx = p - row_str;
                for (int i = 0; i < term->search_query_len && (idx + i) < term->cols; i++) {
                    match_mask[idx + i] = 1;
                }
                p += term->search_query_len;
            }
        }
        
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
            } else if (match_mask[x]) {
                c_bg = vaxp_color_rgba(255, 255, 0, 255);
                c_fg = vaxp_color_rgba(0, 0, 0, 255);
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
    /* Draw images */
    for (TermImage* img = term->images; img != NULL; img = img->next) {
        int view_y = img->y + term->scroll_offset;
        if (view_y + img->height_cells > 0 && view_y < term->rows) {
            float img_cx = img->x * cell_w;
            float img_cy = view_y * cell_h;
            vaxp_canvas_draw_image(canvas, img->image, img_cx, img_cy);
        }
    }
    
    if (term->cursor_visible && term->scroll_offset == 0 && vaxp_widget_has_state(self, VAXP_WIDGET_STATE_FOCUSED) && !term->search_active) {
        float cx = term->cursor_x * cell_w;
        float cy = term->cursor_y * cell_h;
        VaxpPaint cursor_paint = vaxp_paint_fill(vaxp_color_rgba(255, 255, 255, 128));
        vaxp_canvas_draw_rect(canvas, (VaxpRectF){cx, cy, cell_w, cell_h}, &cursor_paint);
    }
    
    if (term->search_active) {
        /* Draw search bar at the bottom */
        float bar_h = 30.0f;
        VaxpRectF bar_rect = {0, b.height - bar_h, b.width, bar_h};
        VaxpPaint bar_bg = vaxp_paint_fill(vaxp_color_rgba(40, 44, 52, 255));
        vaxp_canvas_draw_rect(canvas, bar_rect, &bar_bg);
        
        char search_text[512];
        snprintf(search_text, sizeof(search_text), "Search: %s_", term->search_query);
        
        VaxpFont font = {.family = "monospace", .size = term->font_size, .bold = VAXP_TRUE};
        VaxpPaint text_paint = vaxp_paint_fill(vaxp_color_rgba(200, 220, 255, 255));
        vaxp_canvas_draw_text(canvas, search_text, 10, b.height - bar_h/2 + term->font_size/2, &font, &text_paint);
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
        if (term->search_active) {
            if (event->key.key == VAXP_KEY_ESCAPE) {
                term->search_active = VAXP_FALSE;
                vaxp_widget_invalidate(self);
                return VAXP_TRUE;
            } else if (event->key.key == VAXP_KEY_BACKSPACE) {
                if (term->search_query_len > 0) {
                    term->search_query[--term->search_query_len] = '\0';
                    vaxp_widget_invalidate(self);
                }
                return VAXP_TRUE;
            } else if (event->key.key >= 32 && event->key.key <= 126) {
                if (term->search_query_len < sizeof(term->search_query) - 1) {
                    term->search_query[term->search_query_len++] = (char)event->key.key;
                    term->search_query[term->search_query_len] = '\0';
                    vaxp_widget_invalidate(self);
                }
                return VAXP_TRUE;
            } else if (event->key.key == VAXP_KEY_RETURN) {
                /* For now just hide search on enter, or we can use it to jump to next */
                term->search_active = VAXP_FALSE;
                vaxp_widget_invalidate(self);
                return VAXP_TRUE;
            }
        }
        
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
            } else if (event->key.key == 'F' || event->key.key == 'f') {
                term->search_active = VAXP_TRUE;
                term->search_query[0] = '\0';
                term->search_query_len = 0;
                vaxp_widget_invalidate(self);
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
        } else if (event->key.key == VAXP_KEY_TAB) {
            (void)write(term->pty_fd, "\t", 1);
            term->sel_start_y = -1; vaxp_widget_invalidate(self);
            return VAXP_TRUE;
        } else if (event->key.key == VAXP_KEY_UP) {
            (void)write(term->pty_fd, "\x1b[A", 3);
            term->sel_start_y = -1; vaxp_widget_invalidate(self);
            return VAXP_TRUE;
        } else if (event->key.key == VAXP_KEY_DOWN) {
            (void)write(term->pty_fd, "\x1b[B", 3);
            term->sel_start_y = -1; vaxp_widget_invalidate(self);
            return VAXP_TRUE;
        } else if (event->key.key == VAXP_KEY_RIGHT) {
            (void)write(term->pty_fd, "\x1b[C", 3);
            term->sel_start_y = -1; vaxp_widget_invalidate(self);
            return VAXP_TRUE;
        } else if (event->key.key == VAXP_KEY_LEFT) {
            (void)write(term->pty_fd, "\x1b[D", 3);
            term->sel_start_y = -1; vaxp_widget_invalidate(self);
            return VAXP_TRUE;
        } else if (event->key.key == VAXP_KEY_HOME) {
            (void)write(term->pty_fd, "\x1b[H", 3);
            term->sel_start_y = -1; vaxp_widget_invalidate(self);
            return VAXP_TRUE;
        } else if (event->key.key == VAXP_KEY_END) {
            (void)write(term->pty_fd, "\x1b[F", 3);
            term->sel_start_y = -1; vaxp_widget_invalidate(self);
            return VAXP_TRUE;
        } else if (event->key.key == VAXP_KEY_PAGE_UP) {
            (void)write(term->pty_fd, "\x1b[5~", 4);
            term->sel_start_y = -1; vaxp_widget_invalidate(self);
            return VAXP_TRUE;
        } else if (event->key.key == VAXP_KEY_PAGE_DOWN) {
            (void)write(term->pty_fd, "\x1b[6~", 4);
            term->sel_start_y = -1; vaxp_widget_invalidate(self);
            return VAXP_TRUE;
        } else if (event->key.key == VAXP_KEY_INSERT) {
            (void)write(term->pty_fd, "\x1b[2~", 4);
            term->sel_start_y = -1; vaxp_widget_invalidate(self);
            return VAXP_TRUE;
        } else if (event->key.key == VAXP_KEY_DELETE) {
            (void)write(term->pty_fd, "\x1b[3~", 4);
            term->sel_start_y = -1; vaxp_widget_invalidate(self);
            return VAXP_TRUE;
        } else if (event->key.key == VAXP_KEY_F1) {
            (void)write(term->pty_fd, "\x1bOP", 3); term->sel_start_y = -1; vaxp_widget_invalidate(self); return VAXP_TRUE;
        } else if (event->key.key == VAXP_KEY_F2) {
            (void)write(term->pty_fd, "\x1bOQ", 3); term->sel_start_y = -1; vaxp_widget_invalidate(self); return VAXP_TRUE;
        } else if (event->key.key == VAXP_KEY_F3) {
            (void)write(term->pty_fd, "\x1bOR", 3); term->sel_start_y = -1; vaxp_widget_invalidate(self); return VAXP_TRUE;
        } else if (event->key.key == VAXP_KEY_F4) {
            (void)write(term->pty_fd, "\x1bOS", 3); term->sel_start_y = -1; vaxp_widget_invalidate(self); return VAXP_TRUE;
        } else if (event->key.key == VAXP_KEY_F5) {
            (void)write(term->pty_fd, "\x1b[15~", 5); term->sel_start_y = -1; vaxp_widget_invalidate(self); return VAXP_TRUE;
        } else if (event->key.key == VAXP_KEY_F6) {
            (void)write(term->pty_fd, "\x1b[17~", 5); term->sel_start_y = -1; vaxp_widget_invalidate(self); return VAXP_TRUE;
        } else if (event->key.key == VAXP_KEY_F7) {
            (void)write(term->pty_fd, "\x1b[18~", 5); term->sel_start_y = -1; vaxp_widget_invalidate(self); return VAXP_TRUE;
        } else if (event->key.key == VAXP_KEY_F8) {
            (void)write(term->pty_fd, "\x1b[19~", 5); term->sel_start_y = -1; vaxp_widget_invalidate(self); return VAXP_TRUE;
        } else if (event->key.key == VAXP_KEY_F9) {
            (void)write(term->pty_fd, "\x1b[20~", 5); term->sel_start_y = -1; vaxp_widget_invalidate(self); return VAXP_TRUE;
        } else if (event->key.key == VAXP_KEY_F10) {
            (void)write(term->pty_fd, "\x1b[21~", 5); term->sel_start_y = -1; vaxp_widget_invalidate(self); return VAXP_TRUE;
        } else if (event->key.key == VAXP_KEY_F11) {
            (void)write(term->pty_fd, "\x1b[23~", 5); term->sel_start_y = -1; vaxp_widget_invalidate(self); return VAXP_TRUE;
        } else if (event->key.key == VAXP_KEY_F12) {
            (void)write(term->pty_fd, "\x1b[24~", 5); term->sel_start_y = -1; vaxp_widget_invalidate(self); return VAXP_TRUE;
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
