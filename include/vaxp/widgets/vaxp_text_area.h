/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_text_area.h - Multi-line text input widget
 */

#ifndef VAXP_TEXT_AREA_H
#define VAXP_TEXT_AREA_H

#include "vaxp/widgets/vaxp_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VaxpTextArea VaxpTextArea;

typedef void (*VaxpTextAreaCallback)(VaxpTextArea* area, const char* text, void* user_data);

struct VaxpTextArea {
    VaxpWidget base;
    
    /* Text content */
    char* text;
    VaxpSize text_len;
    VaxpSize text_capacity;
    VaxpU32 max_length;
    
    /* Cursor */
    VaxpU32 cursor_pos;         /* Byte position in text */
    VaxpU32 cursor_line;        /* Current line number */
    VaxpU32 cursor_col;         /* Column in current line */
    
    /* Selection */
    VaxpU32 selection_start;
    VaxpU32 selection_end;
    VaxpBool has_selection;
    
    /* Scrolling */
    VaxpF32 scroll_x;
    VaxpF32 scroll_y;
    VaxpF32 content_height;
    
    /* Display */
    char* placeholder;
    VaxpBool read_only;
    VaxpBool wrap_text;
    VaxpU32 visible_lines;
    VaxpF32 line_height;
    
    /* Styling */
    VaxpColor text_color;
    VaxpColor background_color;
    VaxpColor border_color;
    VaxpColor placeholder_color;
    VaxpColor selection_color;
    VaxpColor cursor_color;
    VaxpF32 padding;
    VaxpF32 border_width;
    VaxpF32 corner_radius;
    
    /* State */
    VaxpBool cursor_visible;
    VaxpU32 cursor_blink_timer;
    
    /* Callbacks */
    VaxpTextAreaCallback on_change;
    void* change_data;
};

VaxpResultPtr vaxp_text_area_create(void);
void vaxp_text_area_set_text(VaxpTextArea* area, const char* text);
const char* vaxp_text_area_get_text(const VaxpTextArea* area);
void vaxp_text_area_set_placeholder(VaxpTextArea* area, const char* placeholder);
void vaxp_text_area_set_read_only(VaxpTextArea* area, VaxpBool read_only);
void vaxp_text_area_set_wrap(VaxpTextArea* area, VaxpBool wrap);
void vaxp_text_area_set_max_length(VaxpTextArea* area, VaxpU32 max);
void vaxp_text_area_set_on_change(VaxpTextArea* area, VaxpTextAreaCallback callback, void* data);
void vaxp_text_area_insert(VaxpTextArea* area, const char* text);
void vaxp_text_area_clear(VaxpTextArea* area);
VaxpU32 vaxp_text_area_get_line_count(const VaxpTextArea* area);

extern const VaxpWidgetClass vaxp_text_area_class;

#define vaxp_text_area(...) _vaxp_text_area_build(&(VaxpTextAreaConfig){ __VA_ARGS__ })

typedef struct VaxpTextAreaConfig {
    const char* text;
    const char* placeholder;
    VaxpBool read_only;
    VaxpBool wrap;
    VaxpU32 lines;              /* Visible lines */
    VaxpTextAreaCallback on_change;
    void* data;
} VaxpTextAreaConfig;

VaxpWidget* _vaxp_text_area_build(const VaxpTextAreaConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VAXP_TEXT_AREA_H */
