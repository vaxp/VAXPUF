/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_avatar.c - Avatar widget implementation
 */

#include "vaxp/widgets/vaxp_avatar.h"
#include "vaxp/core/vaxp_memory.h"
#include <string.h>
#include <ctype.h>

#define DEFAULT_SIZE 40.0f

static void avatar_init(VaxpWidget* widget) {
    VaxpAvatar* avatar = (VaxpAvatar*)widget;
    
    avatar->image_path = NULL;
    avatar->initials = NULL;
    avatar->alt_text = NULL;
    
    avatar->size = DEFAULT_SIZE;
    avatar->background_color = (VaxpColor){ 63, 81, 181, 255 };
    avatar->text_color = (VaxpColor){ 255, 255, 255, 255 };
    avatar->border_color = (VaxpColor){ 255, 255, 255, 255 };
    avatar->border_width = 0;
    
    avatar->show_indicator = VAXP_FALSE;
    avatar->indicator_color = (VaxpColor){ 76, 175, 80, 255 };
}

static void avatar_destroy(VaxpWidget* widget) {
    VaxpAvatar* avatar = (VaxpAvatar*)widget;
    
    if (avatar->image_path) {
        vaxp_free(avatar->image_path, strlen(avatar->image_path) + 1);
    }
    if (avatar->initials) {
        vaxp_free(avatar->initials, strlen(avatar->initials) + 1);
    }
    if (avatar->alt_text) {
        vaxp_free(avatar->alt_text, strlen(avatar->alt_text) + 1);
    }
    
    vaxp_widget_class.destroy(widget);
}

static void avatar_measure(VaxpWidget* widget, VaxpF32 available_width, VaxpF32 available_height,
                           VaxpF32* out_width, VaxpF32* out_height) {
    VaxpAvatar* avatar = (VaxpAvatar*)widget;
    (void)available_width; (void)available_height;
    
    *out_width = avatar->size;
    *out_height = avatar->size;
}

static void avatar_draw(VaxpWidget* widget, VaxpCanvas* canvas) {
    VaxpAvatar* avatar = (VaxpAvatar*)widget;
    
    VaxpF32 size = avatar->size;
    VaxpF32 cx = size / 2;
    VaxpF32 cy = size / 2;
    VaxpF32 r = size / 2;
    
    /* Draw background circle */
    VaxpPaint bg_paint = vaxp_paint_fill(avatar->background_color);
    vaxp_canvas_draw_circle(canvas, cx, cy, r, &bg_paint);
    
    /* Draw initials if no image */
    if (!avatar->image_path && avatar->initials) {
        VaxpPaint text_paint = vaxp_paint_fill(avatar->text_color);
        VaxpF32 font_size = size * 0.4f;
        VaxpF32 text_w = (VaxpF32)strlen(avatar->initials) * font_size * 0.6f;
        VaxpF32 tx = (size - text_w) / 2;
        VaxpF32 ty = size / 2 + font_size / 3;
        vaxp_canvas_draw_text(canvas, avatar->initials, tx, ty, NULL, &text_paint);
    }
    
    /* TODO: Draw image if available */
    
    /* Draw border */
    if (avatar->border_width > 0) {
        VaxpPaint border_paint = vaxp_paint_stroke(avatar->border_color, avatar->border_width);
        vaxp_canvas_draw_circle(canvas, cx, cy, r - avatar->border_width / 2, &border_paint);
    }
    
    /* Draw indicator */
    if (avatar->show_indicator) {
        VaxpF32 ind_r = size * 0.15f;
        VaxpF32 ind_x = size - ind_r - 2;
        VaxpF32 ind_y = size - ind_r - 2;
        
        /* White outline */
        VaxpPaint outline = vaxp_paint_fill((VaxpColor){ 255, 255, 255, 255 });
        vaxp_canvas_draw_circle(canvas, ind_x, ind_y, ind_r + 2, &outline);
        
        /* Indicator */
        VaxpPaint ind_paint = vaxp_paint_fill(avatar->indicator_color);
        vaxp_canvas_draw_circle(canvas, ind_x, ind_y, ind_r, &ind_paint);
    }
}

const VaxpWidgetClass vaxp_avatar_class = {
    .class_name = "VaxpAvatar",
    .instance_size = sizeof(VaxpAvatar),
    .parent_class = &vaxp_widget_class,
    .init = avatar_init,
    .destroy = avatar_destroy,
    .measure = avatar_measure,
    .layout = NULL,
    .draw = avatar_draw,
    .on_event = NULL,
    .on_state_changed = NULL,
};

VaxpResultPtr vaxp_avatar_create(void) {
    return vaxp_widget_create(&vaxp_avatar_class);
}

static void set_string(char** dest, const char* src) {
    if (*dest) {
        vaxp_free(*dest, strlen(*dest) + 1);
        *dest = NULL;
    }
    if (src) {
        VaxpSize len = strlen(src) + 1;
        *dest = (char*)vaxp_alloc(len);
        if (*dest) memcpy(*dest, src, len);
    }
}

void vaxp_avatar_set_image(VaxpAvatar* avatar, const char* path) {
    if (avatar) set_string(&avatar->image_path, path);
}

void vaxp_avatar_set_initials(VaxpAvatar* avatar, const char* initials) {
    if (avatar) {
        if (avatar->initials) {
            vaxp_free(avatar->initials, strlen(avatar->initials) + 1);
            avatar->initials = NULL;
        }
        
        if (initials) {
            VaxpSize len = strlen(initials);
            if (len > 2) len = 2;  /* Max 2 chars */
            
            avatar->initials = (char*)vaxp_alloc(len + 1);
            if (avatar->initials) {
                for (VaxpSize i = 0; i < len; i++) {
                    avatar->initials[i] = (char)toupper((unsigned char)initials[i]);
                }
                avatar->initials[len] = '\0';
            }
        }
        
        vaxp_widget_invalidate((VaxpWidget*)avatar);
    }
}

void vaxp_avatar_set_size(VaxpAvatar* avatar, VaxpF32 size) {
    if (avatar && size > 0) {
        avatar->size = size;
        vaxp_widget_invalidate((VaxpWidget*)avatar);
    }
}

void vaxp_avatar_set_color(VaxpAvatar* avatar, VaxpColor bg) {
    if (avatar) {
        avatar->background_color = bg;
        vaxp_widget_invalidate((VaxpWidget*)avatar);
    }
}

void vaxp_avatar_set_indicator(VaxpAvatar* avatar, VaxpBool show, VaxpColor color) {
    if (avatar) {
        avatar->show_indicator = show;
        avatar->indicator_color = color;
        vaxp_widget_invalidate((VaxpWidget*)avatar);
    }
}

VaxpWidget* _vaxp_avatar_build(const VaxpAvatarConfig* config) {
    VaxpResultPtr result = vaxp_avatar_create();
    if (!result.ok) return NULL;
    
    VaxpAvatar* avatar = (VaxpAvatar*)result.value;
    
    if (config->image) vaxp_avatar_set_image(avatar, config->image);
    if (config->initials) vaxp_avatar_set_initials(avatar, config->initials);
    if (config->size > 0) avatar->size = config->size;
    if (config->color.a > 0) avatar->background_color = config->color;
    
    return (VaxpWidget*)avatar;
}
