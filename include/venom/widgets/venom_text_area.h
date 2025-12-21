/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_text_area.h - Multi-line text input widget
 */

#ifndef VENOM_TEXT_AREA_H
#define VENOM_TEXT_AREA_H

#include "venom/widgets/venom_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VenomTextArea VenomTextArea;

typedef void (*VenomTextAreaCallback)(VenomTextArea* area, const char* text, void* user_data);

struct VenomTextArea {
    VenomWidget base;
    
    /* Text content */
    char* text;
    VenomSize text_len;
    VenomSize text_capacity;
    VenomU32 max_length;
    
    /* Cursor */
    VenomU32 cursor_pos;         /* Byte position in text */
    VenomU32 cursor_line;        /* Current line number */
    VenomU32 cursor_col;         /* Column in current line */
    
    /* Selection */
    VenomU32 selection_start;
    VenomU32 selection_end;
    VenomBool has_selection;
    
    /* Scrolling */
    VenomF32 scroll_x;
    VenomF32 scroll_y;
    VenomF32 content_height;
    
    /* Display */
    char* placeholder;
    VenomBool read_only;
    VenomBool wrap_text;
    VenomU32 visible_lines;
    VenomF32 line_height;
    
    /* Styling */
    VenomColor text_color;
    VenomColor background_color;
    VenomColor border_color;
    VenomColor placeholder_color;
    VenomColor selection_color;
    VenomColor cursor_color;
    VenomF32 padding;
    VenomF32 border_width;
    VenomF32 corner_radius;
    
    /* State */
    VenomBool cursor_visible;
    VenomU32 cursor_blink_timer;
    
    /* Callbacks */
    VenomTextAreaCallback on_change;
    void* change_data;
};

VenomResultPtr venom_text_area_create(void);
void venom_text_area_set_text(VenomTextArea* area, const char* text);
const char* venom_text_area_get_text(const VenomTextArea* area);
void venom_text_area_set_placeholder(VenomTextArea* area, const char* placeholder);
void venom_text_area_set_read_only(VenomTextArea* area, VenomBool read_only);
void venom_text_area_set_wrap(VenomTextArea* area, VenomBool wrap);
void venom_text_area_set_max_length(VenomTextArea* area, VenomU32 max);
void venom_text_area_set_on_change(VenomTextArea* area, VenomTextAreaCallback callback, void* data);
void venom_text_area_insert(VenomTextArea* area, const char* text);
void venom_text_area_clear(VenomTextArea* area);
VenomU32 venom_text_area_get_line_count(const VenomTextArea* area);

extern const VenomWidgetClass venom_text_area_class;

#define venom_text_area(...) _venom_text_area_build(&(VenomTextAreaConfig){ __VA_ARGS__ })

typedef struct VenomTextAreaConfig {
    const char* text;
    const char* placeholder;
    VenomBool read_only;
    VenomBool wrap;
    VenomU32 lines;              /* Visible lines */
    VenomTextAreaCallback on_change;
    void* data;
} VenomTextAreaConfig;

VenomWidget* _venom_text_area_build(const VenomTextAreaConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VENOM_TEXT_AREA_H */
