/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_avatar.h - Avatar widget (circular image/initials)
 */

#ifndef VAXP_AVATAR_H
#define VAXP_AVATAR_H

#include "vaxp/widgets/vaxp_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VaxpAvatar {
    VaxpWidget base;
    
    char* image_path;
    char* initials;              /* Fallback when no image */
    char* alt_text;
    
    VaxpF32 size;
    VaxpColor background_color;
    VaxpColor text_color;
    VaxpColor border_color;
    VaxpF32 border_width;
    
    /* Indicator (online status) */
    VaxpBool show_indicator;
    VaxpColor indicator_color;
    
} VaxpAvatar;

VaxpResultPtr vaxp_avatar_create(void);
void vaxp_avatar_set_image(VaxpAvatar* avatar, const char* path);
void vaxp_avatar_set_initials(VaxpAvatar* avatar, const char* initials);
void vaxp_avatar_set_size(VaxpAvatar* avatar, VaxpF32 size);
void vaxp_avatar_set_color(VaxpAvatar* avatar, VaxpColor bg);
void vaxp_avatar_set_indicator(VaxpAvatar* avatar, VaxpBool show, VaxpColor color);

extern const VaxpWidgetClass vaxp_avatar_class;

#define vaxp_avatar(...) _vaxp_avatar_build(&(VaxpAvatarConfig){ __VA_ARGS__ })

typedef struct VaxpAvatarConfig {
    const char* image;
    const char* initials;
    VaxpF32 size;
    VaxpColor color;
} VaxpAvatarConfig;

VaxpWidget* _vaxp_avatar_build(const VaxpAvatarConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VAXP_AVATAR_H */
