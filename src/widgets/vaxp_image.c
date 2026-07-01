/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_image.c - Image widget implementation using Cairo + libpng
 */

#include "vaxp/widgets/vaxp_image.h"
#include "vaxp/core/vaxp_memory.h"
#include <string.h>
#include <strings.h>  /* For strcasecmp */
#include <stdio.h>
#include <cairo/cairo.h>
#include <png.h>

/* ============================================================================
 * IMAGE DATA IMPLEMENTATION
 * ============================================================================ */

static void image_data_destructor(void* self) {
    VaxpImageData* img = (VaxpImageData*)self;
    
    if (img->cairo_surface) {
        cairo_surface_destroy((cairo_surface_t*)img->cairo_surface);
        img->cairo_surface = NULL;
    }
    
    if (img->pixels) {
        vaxp_free(img->pixels, img->stride * img->height);
        img->pixels = NULL;
    }
}

VaxpResultPtr vaxp_image_load_file(const char* path) {
    if (!path) return VAXP_ERR_PTR(VAXP_ERROR_INVALID_ARGUMENT);
    
    /* Check file extension for format */
    const char* ext = strrchr(path, '.');
    if (!ext) return VAXP_ERR_PTR(VAXP_ERROR_INVALID_ARGUMENT);
    
    /* Load PNG using libpng */
    if (strcasecmp(ext, ".png") == 0) {
        FILE* fp = fopen(path, "rb");
        if (!fp) return VAXP_ERR_PTR(VAXP_ERROR_PATH_NOT_FOUND);
        
        /* Read PNG header */
        png_byte header[8];
        if (fread(header, 1, 8, fp) != 8) {
            fclose(fp);
            return VAXP_ERR_PTR(VAXP_ERROR_IMAGE_LOAD);
        }
        
        if (png_sig_cmp(header, 0, 8)) {
            fclose(fp);
            return VAXP_ERR_PTR(VAXP_ERROR_IMAGE_LOAD);
        }
        
        png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        if (!png) {
            fclose(fp);
            return VAXP_ERR_PTR(VAXP_ERROR_OUT_OF_MEMORY);
        }
        
        png_infop info = png_create_info_struct(png);
        if (!info) {
            png_destroy_read_struct(&png, NULL, NULL);
            fclose(fp);
            return VAXP_ERR_PTR(VAXP_ERROR_OUT_OF_MEMORY);
        }
        
        if (setjmp(png_jmpbuf(png))) {
            png_destroy_read_struct(&png, &info, NULL);
            fclose(fp);
            return VAXP_ERR_PTR(VAXP_ERROR_IMAGE_LOAD);
        }
        
        png_init_io(png, fp);
        png_set_sig_bytes(png, 8);
        png_read_info(png, info);
        
        VaxpU32 width = png_get_image_width(png, info);
        VaxpU32 height = png_get_image_height(png, info);
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
        VaxpU32 stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, width);
        VaxpU8* pixels = (VaxpU8*)vaxp_alloc(stride * height);
        if (!pixels) {
            png_destroy_read_struct(&png, &info, NULL);
            fclose(fp);
            return VAXP_ERR_PTR(VAXP_ERROR_OUT_OF_MEMORY);
        }
        
        /* Read row by row */
        png_bytep* row_ptrs = (png_bytep*)vaxp_alloc(height * sizeof(png_bytep));
        if (!row_ptrs) {
            vaxp_free(pixels, stride * height);
            png_destroy_read_struct(&png, &info, NULL);
            fclose(fp);
            return VAXP_ERR_PTR(VAXP_ERROR_OUT_OF_MEMORY);
        }
        
        for (VaxpU32 y = 0; y < height; y++) {
            row_ptrs[y] = pixels + y * stride;
        }
        
        png_read_image(png, row_ptrs);
        png_read_end(png, NULL);
        
        vaxp_free(row_ptrs, height * sizeof(png_bytep));
        png_destroy_read_struct(&png, &info, NULL);
        fclose(fp);
        
        /* Premultiply alpha for Cairo */
        for (VaxpU32 y = 0; y < height; y++) {
            VaxpU8* row = pixels + y * stride;
            for (VaxpU32 x = 0; x < width; x++) {
                VaxpU8* px = row + x * 4;
                VaxpU8 a = px[3];
                if (a != 255) {
                    px[0] = (VaxpU8)((px[0] * a) / 255);
                    px[1] = (VaxpU8)((px[1] * a) / 255);
                    px[2] = (VaxpU8)((px[2] * a) / 255);
                }
            }
        }
        
        /* Create image data */
        VaxpImageData* img = VAXP_REF_NEW(VaxpImageData, image_data_destructor);
        if (!img) {
            vaxp_free(pixels, stride * height);
            return VAXP_ERR_PTR(VAXP_ERROR_OUT_OF_MEMORY);
        }
        
        img->pixels = pixels;
        img->width = width;
        img->height = height;
        img->stride = stride;
        
        /* Create Cairo surface from pixel data */
        img->cairo_surface = cairo_image_surface_create_for_data(
            pixels, CAIRO_FORMAT_ARGB32, width, height, stride);
        
        if (cairo_surface_status((cairo_surface_t*)img->cairo_surface) != CAIRO_STATUS_SUCCESS) {
            vaxp_unref(img);
            return VAXP_ERR_PTR(VAXP_ERROR_SURFACE_CREATE);
        }
        
        return VAXP_OK_PTR(img);
    }
    
    /* Unsupported format */
    return VAXP_ERR_PTR(VAXP_ERROR_NOT_SUPPORTED);
}

VaxpResultPtr vaxp_image_create(VaxpU32 width, VaxpU32 height) {
    VaxpU32 stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, width);
    VaxpU8* pixels = (VaxpU8*)vaxp_alloc(stride * height);
    if (!pixels) return VAXP_ERR_PTR(VAXP_ERROR_OUT_OF_MEMORY);
    
    memset(pixels, 0, stride * height);
    
    VaxpImageData* img = VAXP_REF_NEW(VaxpImageData, image_data_destructor);
    if (!img) {
        vaxp_free(pixels, stride * height);
        return VAXP_ERR_PTR(VAXP_ERROR_OUT_OF_MEMORY);
    }
    
    img->pixels = pixels;
    img->width = width;
    img->height = height;
    img->stride = stride;
    img->cairo_surface = cairo_image_surface_create_for_data(
        pixels, CAIRO_FORMAT_ARGB32, width, height, stride);
    
    return VAXP_OK_PTR(img);
}

void vaxp_image_get_size(const VaxpImageData* image, VaxpU32* width, VaxpU32* height) {
    if (!image) return;
    if (width) *width = image->width;
    if (height) *height = image->height;
}

/* ============================================================================
 * IMAGE WIDGET IMPLEMENTATION
 * ============================================================================ */

static void image_widget_init(VaxpWidget* widget) {
    VaxpImageWidget* img = (VaxpImageWidget*)widget;
    
    img->image = NULL;
    img->fit = VAXP_IMAGE_FIT_CONTAIN;
    img->corner_radius = 0;
    img->background = (VaxpColor){ 0, 0, 0, 0 };  /* Transparent */
    img->opacity = 1.0f;
}

static void image_widget_destroy(VaxpWidget* widget) {
    VaxpImageWidget* img = (VaxpImageWidget*)widget;
    
    if (img->image) {
        vaxp_unref(img->image);
        img->image = NULL;
    }
    
    vaxp_widget_class.destroy(widget);
}

static void image_widget_measure(VaxpWidget* widget, VaxpF32 available_width, VaxpF32 available_height,
                                  VaxpF32* out_width, VaxpF32* out_height) {
    VaxpImageWidget* img = (VaxpImageWidget*)widget;
    
    if (img->image) {
        *out_width = (VaxpF32)img->image->width;
        *out_height = (VaxpF32)img->image->height;
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

static void image_widget_draw(VaxpWidget* widget, VaxpCanvas* canvas) {
    VaxpImageWidget* img = (VaxpImageWidget*)widget;
    
    if (!img->image || !img->image->cairo_surface) return;
    
    /* Get underlying Cairo context (this is a Cairo-specific implementation detail) */
    /* For a generic approach, we'd add draw_image to canvas interface */
    
    VaxpF32 bounds_w = widget->bounds.width;
    VaxpF32 bounds_h = widget->bounds.height;
    VaxpF32 img_w = (VaxpF32)img->image->width;
    VaxpF32 img_h = (VaxpF32)img->image->height;
    
    /* Calculate scale and position based on fit mode */
    VaxpF32 scale_x = 1.0f, scale_y = 1.0f;
    VaxpF32 offset_x = 0, offset_y = 0;
    
    switch (img->fit) {
        case VAXP_IMAGE_FIT_CONTAIN: {
            VaxpF32 ratio = (bounds_w / img_w < bounds_h / img_h) ? 
                             (bounds_w / img_w) : (bounds_h / img_h);
            scale_x = scale_y = ratio;
            offset_x = (bounds_w - img_w * ratio) / 2;
            offset_y = (bounds_h - img_h * ratio) / 2;
            break;
        }
        case VAXP_IMAGE_FIT_COVER: {
            VaxpF32 ratio = (bounds_w / img_w > bounds_h / img_h) ? 
                             (bounds_w / img_w) : (bounds_h / img_h);
            scale_x = scale_y = ratio;
            offset_x = (bounds_w - img_w * ratio) / 2;
            offset_y = (bounds_h - img_h * ratio) / 2;
            break;
        }
        case VAXP_IMAGE_FIT_FILL:
            scale_x = bounds_w / img_w;
            scale_y = bounds_h / img_h;
            break;
        case VAXP_IMAGE_FIT_NONE:
            offset_x = (bounds_w - img_w) / 2;
            offset_y = (bounds_h - img_h) / 2;
            break;
        case VAXP_IMAGE_FIT_SCALE_DOWN: {
            if (img_w > bounds_w || img_h > bounds_h) {
                VaxpF32 ratio = (bounds_w / img_w < bounds_h / img_h) ? 
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
        vaxp_canvas_save(canvas);
        VaxpRectF clip = { 0, 0, bounds_w, bounds_h };
        vaxp_canvas_clip_rounded_rect(canvas, clip, img->corner_radius);
    }
    
    /* Draw image using canvas interface */
    vaxp_canvas_save(canvas);
    vaxp_canvas_translate(canvas, offset_x, offset_y);
    vaxp_canvas_scale(canvas, scale_x, scale_y);
    
    /* Use the canvas draw_image if available */
    vaxp_canvas_draw_image(canvas, (VaxpImage*)img->image, 0, 0);
    
    vaxp_canvas_restore(canvas);
    
    if (img->corner_radius > 0) {
        vaxp_canvas_restore(canvas);
    }
}

/* ============================================================================
 * IMAGE WIDGET CLASS
 * ============================================================================ */

const VaxpWidgetClass vaxp_image_widget_class = {
    .class_name = "VaxpImageWidget",
    .instance_size = sizeof(VaxpImageWidget),
    .parent_class = &vaxp_widget_class,
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

VaxpResultPtr vaxp_image_widget_create(void) {
    return vaxp_widget_create(&vaxp_image_widget_class);
}

VaxpResult vaxp_image_widget_set_image(VaxpImageWidget* widget, VaxpImageData* image) {
    VAXP_ENSURE_NOT_NULL(widget);
    
    if (widget->image) {
        vaxp_unref(widget->image);
    }
    
    widget->image = image;
    if (image) {
        vaxp_ref(image);
    }
    
    vaxp_widget_invalidate((VaxpWidget*)widget);
    return VAXP_OK_UNIT();
}

VaxpResult vaxp_image_widget_load(VaxpImageWidget* widget, const char* path) {
    VAXP_ENSURE_NOT_NULL(widget);
    VAXP_ENSURE_NOT_NULL(path);
    
    VaxpResultPtr result = vaxp_image_load_file(path);
    if (!result.ok) return VAXP_ERR_UNIT(result.error);
    
    if (widget->image) {
        vaxp_unref(widget->image);
    }
    widget->image = (VaxpImageData*)result.value;
    
    vaxp_widget_invalidate((VaxpWidget*)widget);
    return VAXP_OK_UNIT();
}

void vaxp_image_widget_set_fit(VaxpImageWidget* widget, VaxpImageFit fit) {
    if (widget) {
        widget->fit = fit;
        vaxp_widget_invalidate((VaxpWidget*)widget);
    }
}

void vaxp_image_widget_set_corner_radius(VaxpImageWidget* widget, VaxpF32 radius) {
    if (widget) {
        widget->corner_radius = radius;
        vaxp_widget_invalidate((VaxpWidget*)widget);
    }
}

void vaxp_image_widget_set_opacity(VaxpImageWidget* widget, VaxpF32 opacity) {
    if (widget) {
        widget->opacity = opacity > 1.0f ? 1.0f : (opacity < 0 ? 0 : opacity);
        vaxp_widget_invalidate((VaxpWidget*)widget);
    }
}

/* ============================================================================
 * CONVENIENCE BUILDER
 * ============================================================================ */

VaxpWidget* _vaxp_image_widget_build(const VaxpImageWidgetConfig* config) {
    VaxpResultPtr result = vaxp_image_widget_create();
    if (!result.ok) return NULL;
    
    VaxpImageWidget* img = (VaxpImageWidget*)result.value;
    VaxpWidget* widget = (VaxpWidget*)img;
    
    if (config->src) {
        vaxp_image_widget_load(img, config->src);
    } else if (config->data) {
        vaxp_image_widget_set_image(img, config->data);
    }
    
    if (config->fit != 0) img->fit = config->fit;
    if (config->corner_radius > 0) img->corner_radius = config->corner_radius;
    if (config->opacity > 0) img->opacity = config->opacity;
    if (config->width > 0) widget->layout.preferred_width = config->width;
    if (config->height > 0) widget->layout.preferred_height = config->height;
    
    return widget;
}
