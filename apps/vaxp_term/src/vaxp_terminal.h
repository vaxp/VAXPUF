#ifndef VAXP_TERMINAL_H
#define VAXP_TERMINAL_H

#include <vaxp/vaxpui.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VaxpTerminal VaxpTerminal;

#define VAXP_TERM_ATTR_BOLD       1
#define VAXP_TERM_ATTR_ITALIC     2
#define VAXP_TERM_ATTR_UNDERLINE  4
#define VAXP_TERM_ATTR_INVERSE    8

typedef struct {
    char ch[5]; /* UTF-8 char up to 4 bytes + null terminator */
    VaxpColor fg;
    VaxpColor bg;
    uint8_t flags;
} TermCell;

typedef struct TermImage {
    uint32_t id;
    VaxpImage* image;
    int x; /* Column index */
    int y; /* Row index relative to current screen top */
    int width_cells;
    int height_cells;
    struct TermImage* next;
} TermImage;

struct VaxpTerminal {
    VaxpWidget base;
    
    int cols;
    int rows;
    
    TermCell* ring_buffer;
    int max_rows;       /* Maximum rows in ring buffer (e.g. 1000) */
    int screen_top_row; /* Index in ring_buffer where the screen active area starts */
    int history_count;  /* How many rows have been scrolled out */
    
    int scroll_offset;  /* How far back we have scrolled */
    
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
    
    /* Colors and Attributes */
    VaxpColor default_fg;
    VaxpColor default_bg;
    VaxpColor current_fg;
    VaxpColor current_bg;
    uint8_t current_flags;
    
    /* Font size */
    float font_size;
    float char_width;
    float char_height;
    
    /* PTY master fd */
    int pty_fd;
    
    /* ANSI State Machine */
    int ansi_state;       /* 0=NORMAL, 1=ESC, 2=CSI, 3=OSC, 5=APC (Kitty), 6=DCS (Sixel) */
    VaxpBool ansi_is_private;
    int ansi_params[16];
    int ansi_param_count;
    char ansi_current_param[256]; /* Increased for window titles */
    int ansi_param_idx;
    VaxpBool use_acs;
    
    char* ansi_payload; /* For large payloads like Kitty images */
    int ansi_payload_len;
    int ansi_payload_cap;
    
    TermImage* images; /* Linked list of terminal images */
    
    /* Mouse & Selection */
    int mouse_reporting_mode; /* 0=off, 1000=normal, 1002=cell motion */
    VaxpBool is_selecting;
    int sel_start_x, sel_start_y;
    int sel_end_x, sel_end_y;
    
    VaxpWidget* context_menu;
    
    void (*on_title_changed)(void* user_data, const char* title);
    void* title_user_data;
};

VaxpResultPtr vaxp_terminal_create(void);

void vaxp_terminal_init_grid(VaxpTerminal* term, int cols, int rows);
void vaxp_terminal_write(VaxpTerminal* term, const char* data, int len);

extern const VaxpWidgetClass vaxp_terminal_class;

#define vaxp_term_widget(...) _vaxp_terminal_build(&(VaxpTerminalConfig){ __VA_ARGS__ })

typedef struct {
    int cols;
    int rows;
    float font_size;
    int pty_fd;
    void (*on_title_changed)(void* user_data, const char* title);
    void* title_user_data;
} VaxpTerminalConfig;

VaxpWidget* _vaxp_terminal_build(const VaxpTerminalConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VAXP_TERMINAL_H */
