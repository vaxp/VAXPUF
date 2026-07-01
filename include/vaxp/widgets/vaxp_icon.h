/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_icon.h - Icon widget (emoji/text icons)
 */

#ifndef VAXP_ICON_H
#define VAXP_ICON_H

#include "vaxp/widgets/vaxp_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VaxpIcon {
    VaxpWidget base;
    
    char* icon;              /* Unicode emoji or icon font char */
    VaxpF32 size;
    VaxpColor color;
    
} VaxpIcon;

VaxpResultPtr vaxp_icon_create(void);
void vaxp_icon_set_icon(VaxpIcon* icon, const char* icon_str);
void vaxp_icon_set_size(VaxpIcon* icon, VaxpF32 size);
void vaxp_icon_set_color(VaxpIcon* icon, VaxpColor color);

extern const VaxpWidgetClass vaxp_icon_class;

#define vaxp_icon(...) _vaxp_icon_build(&(VaxpIconConfig){ __VA_ARGS__ })

typedef struct VaxpIconConfig {
    const char* icon;
    VaxpF32 size;
    VaxpColor color;
} VaxpIconConfig;

VaxpWidget* _vaxp_icon_build(const VaxpIconConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VAXP_ICON_H */
