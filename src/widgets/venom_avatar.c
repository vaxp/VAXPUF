/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_avatar.c - Avatar widget implementation
 */

#include "venom/widgets/venom_avatar.h"
#include "venom/core/venom_memory.h"
#include <string.h>
#include <ctype.h>

#define DEFAULT_SIZE 40.0f

static void avatar_init(VenomWidget* widget) {
    VenomAvatar* avatar = (VenomAvatar*)widget;
    
    avatar->image_path = NULL;
    avatar->initials = NULL;
    avatar->alt_text = NULL;
    
    avatar->size = DEFAULT_SIZE;
    avatar->background_color = (VenomColor){ 63, 81, 181, 255 };
    avatar->text_color = (VenomColor){ 255, 255, 255, 255 };
    avatar->border_color = (VenomColor){ 255, 255, 255, 255 };
    avatar->border_width = 0;
    
    avatar->show_indicator = VENOM_FALSE;
    avatar->indicator_color = (VenomColor){ 76, 175, 80, 255 };
}

static void avatar_destroy(VenomWidget* widget) {
    VenomAvatar* avatar = (VenomAvatar*)widget;
    
    if (avatar->image_path) {
        venom_free(avatar->image_path, strlen(avatar->image_path) + 1);
    }
    if (avatar->initials) {
        venom_free(avatar->initials, strlen(avatar->initials) + 1);
    }
    if (avatar->alt_text) {
        venom_free(avatar->alt_text, strlen(avatar->alt_text) + 1);
    }
    
    venom_widget_class.destroy(widget);
}

static void avatar_measure(VenomWidget* widget, VenomF32 available_width, VenomF32 available_height,
                           VenomF32* out_width, VenomF32* out_height) {
    VenomAvatar* avatar = (VenomAvatar*)widget;
    (void)available_width; (void)available_height;
    
    *out_width = avatar->size;
    *out_height = avatar->size;
}

static void avatar_draw(VenomWidget* widget, VenomCanvas* canvas) {
    VenomAvatar* avatar = (VenomAvatar*)widget;
    
    VenomF32 size = avatar->size;
    VenomF32 cx = size / 2;
    VenomF32 cy = size / 2;
    VenomF32 r = size / 2;
    
    /* Draw background circle */
    VenomPaint bg_paint = venom_paint_fill(avatar->background_color);
    venom_canvas_draw_circle(canvas, cx, cy, r, &bg_paint);
    
    /* Draw initials if no image */
    if (!avatar->image_path && avatar->initials) {
        VenomPaint text_paint = venom_paint_fill(avatar->text_color);
        VenomF32 font_size = size * 0.4f;
        VenomF32 text_w = (VenomF32)strlen(avatar->initials) * font_size * 0.6f;
        VenomF32 tx = (size - text_w) / 2;
        VenomF32 ty = size / 2 + font_size / 3;
        venom_canvas_draw_text(canvas, avatar->initials, tx, ty, NULL, &text_paint);
    }
    
    /* TODO: Draw image if available */
    
    /* Draw border */
    if (avatar->border_width > 0) {
        VenomPaint border_paint = venom_paint_stroke(avatar->border_color, avatar->border_width);
        venom_canvas_draw_circle(canvas, cx, cy, r - avatar->border_width / 2, &border_paint);
    }
    
    /* Draw indicator */
    if (avatar->show_indicator) {
        VenomF32 ind_r = size * 0.15f;
        VenomF32 ind_x = size - ind_r - 2;
        VenomF32 ind_y = size - ind_r - 2;
        
        /* White outline */
        VenomPaint outline = venom_paint_fill((VenomColor){ 255, 255, 255, 255 });
        venom_canvas_draw_circle(canvas, ind_x, ind_y, ind_r + 2, &outline);
        
        /* Indicator */
        VenomPaint ind_paint = venom_paint_fill(avatar->indicator_color);
        venom_canvas_draw_circle(canvas, ind_x, ind_y, ind_r, &ind_paint);
    }
}

const VenomWidgetClass venom_avatar_class = {
    .class_name = "VenomAvatar",
    .instance_size = sizeof(VenomAvatar),
    .parent_class = &venom_widget_class,
    .init = avatar_init,
    .destroy = avatar_destroy,
    .measure = avatar_measure,
    .layout = NULL,
    .draw = avatar_draw,
    .on_event = NULL,
    .on_state_changed = NULL,
};

VenomResultPtr venom_avatar_create(void) {
    return venom_widget_create(&venom_avatar_class);
}

static void set_string(char** dest, const char* src) {
    if (*dest) {
        venom_free(*dest, strlen(*dest) + 1);
        *dest = NULL;
    }
    if (src) {
        VenomSize len = strlen(src) + 1;
        *dest = (char*)venom_alloc(len);
        if (*dest) memcpy(*dest, src, len);
    }
}

void venom_avatar_set_image(VenomAvatar* avatar, const char* path) {
    if (avatar) set_string(&avatar->image_path, path);
}

void venom_avatar_set_initials(VenomAvatar* avatar, const char* initials) {
    if (avatar) {
        if (avatar->initials) {
            venom_free(avatar->initials, strlen(avatar->initials) + 1);
            avatar->initials = NULL;
        }
        
        if (initials) {
            VenomSize len = strlen(initials);
            if (len > 2) len = 2;  /* Max 2 chars */
            
            avatar->initials = (char*)venom_alloc(len + 1);
            if (avatar->initials) {
                for (VenomSize i = 0; i < len; i++) {
                    avatar->initials[i] = (char)toupper((unsigned char)initials[i]);
                }
                avatar->initials[len] = '\0';
            }
        }
        
        venom_widget_invalidate((VenomWidget*)avatar);
    }
}

void venom_avatar_set_size(VenomAvatar* avatar, VenomF32 size) {
    if (avatar && size > 0) {
        avatar->size = size;
        venom_widget_invalidate((VenomWidget*)avatar);
    }
}

void venom_avatar_set_color(VenomAvatar* avatar, VenomColor bg) {
    if (avatar) {
        avatar->background_color = bg;
        venom_widget_invalidate((VenomWidget*)avatar);
    }
}

void venom_avatar_set_indicator(VenomAvatar* avatar, VenomBool show, VenomColor color) {
    if (avatar) {
        avatar->show_indicator = show;
        avatar->indicator_color = color;
        venom_widget_invalidate((VenomWidget*)avatar);
    }
}

VenomWidget* _venom_avatar_build(const VenomAvatarConfig* config) {
    VenomResultPtr result = venom_avatar_create();
    if (!result.ok) return NULL;
    
    VenomAvatar* avatar = (VenomAvatar*)result.value;
    
    if (config->image) venom_avatar_set_image(avatar, config->image);
    if (config->initials) venom_avatar_set_initials(avatar, config->initials);
    if (config->size > 0) avatar->size = config->size;
    if (config->color.a > 0) avatar->background_color = config->color;
    
    return (VenomWidget*)avatar;
}
