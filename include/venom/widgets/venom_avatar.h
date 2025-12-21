/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_avatar.h - Avatar widget (circular image/initials)
 */

#ifndef VENOM_AVATAR_H
#define VENOM_AVATAR_H

#include "venom/widgets/venom_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VenomAvatar {
    VenomWidget base;
    
    char* image_path;
    char* initials;              /* Fallback when no image */
    char* alt_text;
    
    VenomF32 size;
    VenomColor background_color;
    VenomColor text_color;
    VenomColor border_color;
    VenomF32 border_width;
    
    /* Indicator (online status) */
    VenomBool show_indicator;
    VenomColor indicator_color;
    
} VenomAvatar;

VenomResultPtr venom_avatar_create(void);
void venom_avatar_set_image(VenomAvatar* avatar, const char* path);
void venom_avatar_set_initials(VenomAvatar* avatar, const char* initials);
void venom_avatar_set_size(VenomAvatar* avatar, VenomF32 size);
void venom_avatar_set_color(VenomAvatar* avatar, VenomColor bg);
void venom_avatar_set_indicator(VenomAvatar* avatar, VenomBool show, VenomColor color);

extern const VenomWidgetClass venom_avatar_class;

#define venom_avatar(...) _venom_avatar_build(&(VenomAvatarConfig){ __VA_ARGS__ })

typedef struct VenomAvatarConfig {
    const char* image;
    const char* initials;
    VenomF32 size;
    VenomColor color;
} VenomAvatarConfig;

VenomWidget* _venom_avatar_build(const VenomAvatarConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VENOM_AVATAR_H */
