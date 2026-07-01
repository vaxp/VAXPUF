/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_link.h - Clickable hyperlink widget
 */

#ifndef VAXP_LINK_H
#define VAXP_LINK_H

#include "vaxp/widgets/vaxp_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VaxpLink VaxpLink;
typedef void (*VaxpLinkCallback)(VaxpLink* link, const char* url, void* data);

struct VaxpLink {
    VaxpWidget base;
    
    char* text;
    char* url;
    VaxpBool visited;
    VaxpBool underline_on_hover;
    
    VaxpLinkCallback on_click;
    void* callback_data;
    
    VaxpColor normal_color;
    VaxpColor visited_color;
    VaxpColor hover_color;
    VaxpBool hovering;
};

VaxpResultPtr vaxp_link_create(void);
void vaxp_link_set_text(VaxpLink* link, const char* text);
void vaxp_link_set_url(VaxpLink* link, const char* url);
void vaxp_link_set_on_click(VaxpLink* link, VaxpLinkCallback callback, void* data);

extern const VaxpWidgetClass vaxp_link_class;

#define vaxp_link(...) _vaxp_link_build(&(VaxpLinkConfig){ __VA_ARGS__ })

typedef struct VaxpLinkConfig {
    const char* text;
    const char* url;
    VaxpLinkCallback on_click;
    void* data;
} VaxpLinkConfig;

VaxpWidget* _vaxp_link_build(const VaxpLinkConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VAXP_LINK_H */
