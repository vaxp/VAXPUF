/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_link.h - Clickable hyperlink widget
 */

#ifndef VENOM_LINK_H
#define VENOM_LINK_H

#include "venom/widgets/venom_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VenomLink VenomLink;
typedef void (*VenomLinkCallback)(VenomLink* link, const char* url, void* data);

struct VenomLink {
    VenomWidget base;
    
    char* text;
    char* url;
    VenomBool visited;
    VenomBool underline_on_hover;
    
    VenomLinkCallback on_click;
    void* callback_data;
    
    VenomColor normal_color;
    VenomColor visited_color;
    VenomColor hover_color;
    VenomBool hovering;
};

VenomResultPtr venom_link_create(void);
void venom_link_set_text(VenomLink* link, const char* text);
void venom_link_set_url(VenomLink* link, const char* url);
void venom_link_set_on_click(VenomLink* link, VenomLinkCallback callback, void* data);

extern const VenomWidgetClass venom_link_class;

#define venom_link(...) _venom_link_build(&(VenomLinkConfig){ __VA_ARGS__ })

typedef struct VenomLinkConfig {
    const char* text;
    const char* url;
    VenomLinkCallback on_click;
    void* data;
} VenomLinkConfig;

VenomWidget* _venom_link_build(const VenomLinkConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VENOM_LINK_H */
