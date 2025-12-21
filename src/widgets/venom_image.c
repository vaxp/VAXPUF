/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_image.c - Image widget implementation using Cairo + libpng
 */

#include "venom/widgets/venom_image.h"
#include "venom/core/venom_memory.h"
#include <string.h>
#include <strings.h>  /* For strcasecmp */
#include <stdio.h>
#include <cairo/cairo.h>
#include <png.h>

/* ============================================================================
 * IMAGE DATA IMPLEMENTATION
 * ============================================================================ */

static void image_data_destructor(void* self) {
    VenomImageData* img = (VenomImageData*)self;
    
    if (img->cairo_surface) {
        cairo_surface_destroy((cairo_surface_t*)img->cairo_surface);
        img->cairo_surface = NULL;
    }
    
    if (img->pixels) {
        venom_free(img->pixels, img->stride * img->height);
        img->pixels = NULL;
    }
}

VenomResultPtr venom_image_load_file(const char* path) {
    if (!path) return VENOM_ERR_PTR(VENOM_ERROR_INVALID_ARGUMENT);
    
    /* Check file extension for format */
    const char* ext = strrchr(path, '.');
    if (!ext) return VENOM_ERR_PTR(VENOM_ERROR_INVALID_ARGUMENT);
    
    /* Load PNG using libpng */
    if (strcasecmp(ext, ".png") == 0) {
        FILE* fp = fopen(path, "rb");
        if (!fp) return VENOM_ERR_PTR(VENOM_ERROR_PATH_NOT_FOUND);
        
        /* Read PNG header */
        png_byte header[8];
        if (fread(header, 1, 8, fp) != 8) {
            fclose(fp);
            return VENOM_ERR_PTR(VENOM_ERROR_IMAGE_LOAD);
        }
        
        if (png_sig_cmp(header, 0, 8)) {
            fclose(fp);
            return VENOM_ERR_PTR(VENOM_ERROR_IMAGE_LOAD);
        }
        
        png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        if (!png) {
            fclose(fp);
            return VENOM_ERR_PTR(VENOM_ERROR_OUT_OF_MEMORY);
        }
        
        png_infop info = png_create_info_struct(png);
        if (!info) {
            png_destroy_read_struct(&png, NULL, NULL);
            fclose(fp);
            return VENOM_ERR_PTR(VENOM_ERROR_OUT_OF_MEMORY);
        }
        
        if (setjmp(png_jmpbuf(png))) {
            png_destroy_read_struct(&png, &info, NULL);
            fclose(fp);
            return VENOM_ERR_PTR(VENOM_ERROR_IMAGE_LOAD);
        }
        
        png_init_io(png, fp);
        png_set_sig_bytes(png, 8);
        png_read_info(png, info);
        
        VenomU32 width = png_get_image_width(png, info);
        VenomU32 height = png_get_image_height(png, info);
        png_byte color_type = png_get_color_type(png, info);
        png_byte bit_depth = png_get_bit_depth(png, info);
        
        /* Convert to RGBA */
        if (bit_depth == 16) png_set_strip_16(png);
        if (color_type == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(png);
        if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) 
            png_set_expand_gray_1_2_4_to_8(png);
        if (png_get_valid(png, info, PNG_INFO_tRNS)) png_set_tRNS_to_alpha(png);
        if (color_type == PNG_COLOR_TYPE_RGB || 
            color_type == PNG_COLOR_TYPE_GRAY ||
            color_type == PNG_COLOR_TYPE_PALETTE)
            png_set_filler(png, 0xFF, PNG_FILLER_AFTER);
        if (color_type == PNG_COLOR_TYPE_GRAY || 
            color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
            png_set_gray_to_rgb(png);
        
        /* Cairo uses BGRA (ARGB32), swap bytes */
        png_set_bgr(png);
        
        png_read_update_info(png, info);
        
        /* Allocate pixel buffer with Cairo stride */
        VenomU32 stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, width);
        VenomU8* pixels = (VenomU8*)venom_alloc(stride * height);
        if (!pixels) {
            png_destroy_read_struct(&png, &info, NULL);
            fclose(fp);
            return VENOM_ERR_PTR(VENOM_ERROR_OUT_OF_MEMORY);
        }
        
        /* Read row by row */
        png_bytep* row_ptrs = (png_bytep*)venom_alloc(height * sizeof(png_bytep));
        if (!row_ptrs) {
            venom_free(pixels, stride * height);
            png_destroy_read_struct(&png, &info, NULL);
            fclose(fp);
            return VENOM_ERR_PTR(VENOM_ERROR_OUT_OF_MEMORY);
        }
        
        for (VenomU32 y = 0; y < height; y++) {
            row_ptrs[y] = pixels + y * stride;
        }
        
        png_read_image(png, row_ptrs);
        png_read_end(png, NULL);
        
        venom_free(row_ptrs, height * sizeof(png_bytep));
        png_destroy_read_struct(&png, &info, NULL);
        fclose(fp);
        
        /* Premultiply alpha for Cairo */
        for (VenomU32 y = 0; y < height; y++) {
            VenomU8* row = pixels + y * stride;
            for (VenomU32 x = 0; x < width; x++) {
                VenomU8* px = row + x * 4;
                VenomU8 a = px[3];
                if (a != 255) {
                    px[0] = (VenomU8)((px[0] * a) / 255);
                    px[1] = (VenomU8)((px[1] * a) / 255);
                    px[2] = (VenomU8)((px[2] * a) / 255);
                }
            }
        }
        
        /* Create image data */
        VenomImageData* img = VENOM_REF_NEW(VenomImageData, image_data_destructor);
        if (!img) {
            venom_free(pixels, stride * height);
            return VENOM_ERR_PTR(VENOM_ERROR_OUT_OF_MEMORY);
        }
        
        img->pixels = pixels;
        img->width = width;
        img->height = height;
        img->stride = stride;
        
        /* Create Cairo surface from pixel data */
        img->cairo_surface = cairo_image_surface_create_for_data(
            pixels, CAIRO_FORMAT_ARGB32, width, height, stride);
        
        if (cairo_surface_status((cairo_surface_t*)img->cairo_surface) != CAIRO_STATUS_SUCCESS) {
            venom_unref(img);
            return VENOM_ERR_PTR(VENOM_ERROR_SURFACE_CREATE);
        }
        
        return VENOM_OK_PTR(img);
    }
    
    /* Unsupported format */
    return VENOM_ERR_PTR(VENOM_ERROR_NOT_SUPPORTED);
}

VenomResultPtr venom_image_create(VenomU32 width, VenomU32 height) {
    VenomU32 stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, width);
    VenomU8* pixels = (VenomU8*)venom_alloc(stride * height);
    if (!pixels) return VENOM_ERR_PTR(VENOM_ERROR_OUT_OF_MEMORY);
    
    memset(pixels, 0, stride * height);
    
    VenomImageData* img = VENOM_REF_NEW(VenomImageData, image_data_destructor);
    if (!img) {
        venom_free(pixels, stride * height);
        return VENOM_ERR_PTR(VENOM_ERROR_OUT_OF_MEMORY);
    }
    
    img->pixels = pixels;
    img->width = width;
    img->height = height;
    img->stride = stride;
    img->cairo_surface = cairo_image_surface_create_for_data(
        pixels, CAIRO_FORMAT_ARGB32, width, height, stride);
    
    return VENOM_OK_PTR(img);
}

void venom_image_get_size(const VenomImageData* image, VenomU32* width, VenomU32* height) {
    if (!image) return;
    if (width) *width = image->width;
    if (height) *height = image->height;
}

/* ============================================================================
 * IMAGE WIDGET IMPLEMENTATION
 * ============================================================================ */

static void image_widget_init(VenomWidget* widget) {
    VenomImageWidget* img = (VenomImageWidget*)widget;
    
    img->image = NULL;
    img->fit = VENOM_IMAGE_FIT_CONTAIN;
    img->corner_radius = 0;
    img->background = (VenomColor){ 0, 0, 0, 0 };  /* Transparent */
    img->opacity = 1.0f;
}

static void image_widget_destroy(VenomWidget* widget) {
    VenomImageWidget* img = (VenomImageWidget*)widget;
    
    if (img->image) {
        venom_unref(img->image);
        img->image = NULL;
    }
    
    venom_widget_class.destroy(widget);
}

static void image_widget_measure(VenomWidget* widget, VenomF32 available_width, VenomF32 available_height,
                                  VenomF32* out_width, VenomF32* out_height) {
    VenomImageWidget* img = (VenomImageWidget*)widget;
    
    if (img->image) {
        *out_width = (VenomF32)img->image->width;
        *out_height = (VenomF32)img->image->height;
    } else {
        *out_width = 0;
        *out_height = 0;
    }
    
    /* Apply preferred/min/max constraints */
    if (widget->layout.preferred_width > 0) *out_width = widget->layout.preferred_width;
    if (widget->layout.preferred_height > 0) *out_height = widget->layout.preferred_height;
    if (*out_width > available_width) *out_width = available_width;
    if (*out_height > available_height) *out_height = available_height;
}

static void image_widget_draw(VenomWidget* widget, VenomCanvas* canvas) {
    VenomImageWidget* img = (VenomImageWidget*)widget;
    
    if (!img->image || !img->image->cairo_surface) return;
    
    /* Get underlying Cairo context (this is a Cairo-specific implementation detail) */
    /* For a generic approach, we'd add draw_image to canvas interface */
    
    VenomF32 bounds_w = widget->bounds.width;
    VenomF32 bounds_h = widget->bounds.height;
    VenomF32 img_w = (VenomF32)img->image->width;
    VenomF32 img_h = (VenomF32)img->image->height;
    
    /* Calculate scale and position based on fit mode */
    VenomF32 scale_x = 1.0f, scale_y = 1.0f;
    VenomF32 offset_x = 0, offset_y = 0;
    
    switch (img->fit) {
        case VENOM_IMAGE_FIT_CONTAIN: {
            VenomF32 ratio = (bounds_w / img_w < bounds_h / img_h) ? 
                             (bounds_w / img_w) : (bounds_h / img_h);
            scale_x = scale_y = ratio;
            offset_x = (bounds_w - img_w * ratio) / 2;
            offset_y = (bounds_h - img_h * ratio) / 2;
            break;
        }
        case VENOM_IMAGE_FIT_COVER: {
            VenomF32 ratio = (bounds_w / img_w > bounds_h / img_h) ? 
                             (bounds_w / img_w) : (bounds_h / img_h);
            scale_x = scale_y = ratio;
            offset_x = (bounds_w - img_w * ratio) / 2;
            offset_y = (bounds_h - img_h * ratio) / 2;
            break;
        }
        case VENOM_IMAGE_FIT_FILL:
            scale_x = bounds_w / img_w;
            scale_y = bounds_h / img_h;
            break;
        case VENOM_IMAGE_FIT_NONE:
            offset_x = (bounds_w - img_w) / 2;
            offset_y = (bounds_h - img_h) / 2;
            break;
        case VENOM_IMAGE_FIT_SCALE_DOWN: {
            if (img_w > bounds_w || img_h > bounds_h) {
                VenomF32 ratio = (bounds_w / img_w < bounds_h / img_h) ? 
                                 (bounds_w / img_w) : (bounds_h / img_h);
                scale_x = scale_y = ratio;
                offset_x = (bounds_w - img_w * ratio) / 2;
                offset_y = (bounds_h - img_h * ratio) / 2;
            } else {
                offset_x = (bounds_w - img_w) / 2;
                offset_y = (bounds_h - img_h) / 2;
            }
            break;
        }
    }
    
    /* Clip to rounded rect if needed */
    if (img->corner_radius > 0) {
        venom_canvas_save(canvas);
        VenomRectF clip = { 0, 0, bounds_w, bounds_h };
        venom_canvas_clip_rounded_rect(canvas, clip, img->corner_radius);
    }
    
    /* Draw image using canvas interface */
    venom_canvas_save(canvas);
    venom_canvas_translate(canvas, offset_x, offset_y);
    venom_canvas_scale(canvas, scale_x, scale_y);
    
    /* Use the canvas draw_image if available */
    venom_canvas_draw_image(canvas, (VenomImage*)img->image, 0, 0);
    
    venom_canvas_restore(canvas);
    
    if (img->corner_radius > 0) {
        venom_canvas_restore(canvas);
    }
}

/* ============================================================================
 * IMAGE WIDGET CLASS
 * ============================================================================ */

const VenomWidgetClass venom_image_widget_class = {
    .class_name = "VenomImageWidget",
    .instance_size = sizeof(VenomImageWidget),
    .parent_class = &venom_widget_class,
    .init = image_widget_init,
    .destroy = image_widget_destroy,
    .measure = image_widget_measure,
    .layout = NULL,
    .draw = image_widget_draw,
    .on_event = NULL,
    .on_state_changed = NULL,
};

/* ============================================================================
 * IMAGE WIDGET API
 * ============================================================================ */

VenomResultPtr venom_image_widget_create(void) {
    return venom_widget_create(&venom_image_widget_class);
}

VenomResult venom_image_widget_set_image(VenomImageWidget* widget, VenomImageData* image) {
    VENOM_ENSURE_NOT_NULL(widget);
    
    if (widget->image) {
        venom_unref(widget->image);
    }
    
    widget->image = image;
    if (image) {
        venom_ref(image);
    }
    
    venom_widget_invalidate((VenomWidget*)widget);
    return VENOM_OK_UNIT();
}

VenomResult venom_image_widget_load(VenomImageWidget* widget, const char* path) {
    VENOM_ENSURE_NOT_NULL(widget);
    VENOM_ENSURE_NOT_NULL(path);
    
    VenomResultPtr result = venom_image_load_file(path);
    if (!result.ok) return VENOM_ERR_UNIT(result.error);
    
    if (widget->image) {
        venom_unref(widget->image);
    }
    widget->image = (VenomImageData*)result.value;
    
    venom_widget_invalidate((VenomWidget*)widget);
    return VENOM_OK_UNIT();
}

void venom_image_widget_set_fit(VenomImageWidget* widget, VenomImageFit fit) {
    if (widget) {
        widget->fit = fit;
        venom_widget_invalidate((VenomWidget*)widget);
    }
}

void venom_image_widget_set_corner_radius(VenomImageWidget* widget, VenomF32 radius) {
    if (widget) {
        widget->corner_radius = radius;
        venom_widget_invalidate((VenomWidget*)widget);
    }
}

void venom_image_widget_set_opacity(VenomImageWidget* widget, VenomF32 opacity) {
    if (widget) {
        widget->opacity = opacity > 1.0f ? 1.0f : (opacity < 0 ? 0 : opacity);
        venom_widget_invalidate((VenomWidget*)widget);
    }
}

/* ============================================================================
 * CONVENIENCE BUILDER
 * ============================================================================ */

VenomWidget* _venom_image_widget_build(const VenomImageWidgetConfig* config) {
    VenomResultPtr result = venom_image_widget_create();
    if (!result.ok) return NULL;
    
    VenomImageWidget* img = (VenomImageWidget*)result.value;
    VenomWidget* widget = (VenomWidget*)img;
    
    if (config->src) {
        venom_image_widget_load(img, config->src);
    } else if (config->data) {
        venom_image_widget_set_image(img, config->data);
    }
    
    if (config->fit != 0) img->fit = config->fit;
    if (config->corner_radius > 0) img->corner_radius = config->corner_radius;
    if (config->opacity > 0) img->opacity = config->opacity;
    if (config->width > 0) widget->layout.preferred_width = config->width;
    if (config->height > 0) widget->layout.preferred_height = config->height;
    
    return widget;
}
