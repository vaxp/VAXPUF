#ifndef VAXP_TERMINAL_H
#define VAXP_TERMINAL_H

#include <vaxp/vaxpui.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VaxpTerminal VaxpTerminal;

/* Represents a single character cell in the terminal */
typedef struct {
    char ch[5]; /* UTF-8 char up to 4 bytes + null terminator */
    VaxpColor fg;
    VaxpColor bg;
} TermCell;

struct VaxpTerminal {
    VaxpWidget base;
    
    int cols;
    int rows;
    
    TermCell* ring_buffer;
    int max_rows;       /* Maximum rows in ring buffer (e.g. 1000) */
    int screen_top_row; /* Index in ring_buffer where the screen starts */
    
    /* Alternate Screen Buffer */
    TermCell* alt_buffer;
    VaxpBool use_alt_buffer;
    int saved_cursor_x;
    int saved_cursor_y;
    
    int cursor_x;
    int cursor_y;
    VaxpBool cursor_visible;
    
    /* UTF-8 Parsing State */
    int utf8_state;
    char utf8_buf[5];
    int utf8_idx;
    
    /* Colors */
    VaxpColor default_fg;
    VaxpColor default_bg;
    VaxpColor current_fg;
    VaxpColor current_bg;
    
    /* Font size */
    float font_size;
    float char_width;
    float char_height;
    
    /* PTY master fd to write input */
    int pty_fd;
    
    /* ANSI State Machine */
    int ansi_state;       /* 0=NORMAL, 1=ESC, 2=CSI, 3=OSC */
    VaxpBool ansi_is_private; /* true if sequence starts with '?' */
    int ansi_params[16];
    int ansi_param_count;
    char ansi_current_param[16];
    int ansi_param_idx;
};

VaxpResultPtr vaxp_terminal_create(void);

void vaxp_terminal_init_grid(VaxpTerminal* term, int cols, int rows);

/* Write raw data from PTY (handles very basic ANSI parsing) */
void vaxp_terminal_write(VaxpTerminal* term, const char* data, int len);

extern const VaxpWidgetClass vaxp_terminal_class;

/* Builder Macro */
#define vaxp_term_widget(...) _vaxp_terminal_build(&(VaxpTerminalConfig){ __VA_ARGS__ })

typedef struct {
    int cols;
    int rows;
    float font_size;
    int pty_fd;
} VaxpTerminalConfig;

VaxpWidget* _vaxp_terminal_build(const VaxpTerminalConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VAXP_TERMINAL_H */
