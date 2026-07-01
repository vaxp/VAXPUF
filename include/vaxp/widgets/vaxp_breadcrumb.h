/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_breadcrumb.h - Breadcrumb navigation
 */

#ifndef VAXP_BREADCRUMB_H
#define VAXP_BREADCRUMB_H

#include "vaxp/widgets/vaxp_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VaxpBreadcrumb VaxpBreadcrumb;
typedef void (*VaxpBreadcrumbCallback)(VaxpBreadcrumb* bc, VaxpU32 index, const char* path, void* data);

typedef struct VaxpBreadcrumbItem {
    char* label;
    char* path;
} VaxpBreadcrumbItem;

struct VaxpBreadcrumb {
    VaxpWidget base;
    
    VaxpBreadcrumbItem* items;
    VaxpU32 item_count;
    VaxpU32 item_capacity;
    
    char* separator;
    VaxpI32 hover_index;
    
    VaxpBreadcrumbCallback on_navigate;
    void* callback_data;
    
    VaxpColor text_color;
    VaxpColor hover_color;
    VaxpColor separator_color;
    VaxpF32 item_padding;
};

VaxpResultPtr vaxp_breadcrumb_create(void);
VaxpResult vaxp_breadcrumb_add_item(VaxpBreadcrumb* bc, const char* label, const char* path);
void vaxp_breadcrumb_set_path(VaxpBreadcrumb* bc, const char* path, char delimiter);
void vaxp_breadcrumb_pop(VaxpBreadcrumb* bc);
void vaxp_breadcrumb_clear(VaxpBreadcrumb* bc);
void vaxp_breadcrumb_set_separator(VaxpBreadcrumb* bc, const char* sep);
void vaxp_breadcrumb_set_on_navigate(VaxpBreadcrumb* bc, VaxpBreadcrumbCallback callback, void* data);

extern const VaxpWidgetClass vaxp_breadcrumb_class;

#ifdef __cplusplus
}
#endif

#endif /* VAXP_BREADCRUMB_H */
