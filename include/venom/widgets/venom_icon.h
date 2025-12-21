/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_icon.h - Icon widget (emoji/text icons)
 */

#ifndef VENOM_ICON_H
#define VENOM_ICON_H

#include "venom/widgets/venom_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VenomIcon {
    VenomWidget base;
    
    char* icon;              /* Unicode emoji or icon font char */
    VenomF32 size;
    VenomColor color;
    
} VenomIcon;

VenomResultPtr venom_icon_create(void);
void venom_icon_set_icon(VenomIcon* icon, const char* icon_str);
void venom_icon_set_size(VenomIcon* icon, VenomF32 size);
void venom_icon_set_color(VenomIcon* icon, VenomColor color);

extern const VenomWidgetClass venom_icon_class;

#define venom_icon(...) _venom_icon_build(&(VenomIconConfig){ __VA_ARGS__ })

typedef struct VenomIconConfig {
    const char* icon;
    VenomF32 size;
    VenomColor color;
} VenomIconConfig;

VenomWidget* _venom_icon_build(const VenomIconConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VENOM_ICON_H */
