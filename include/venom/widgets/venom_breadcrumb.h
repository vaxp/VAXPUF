/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_breadcrumb.h - Breadcrumb navigation
 */

#ifndef VENOM_BREADCRUMB_H
#define VENOM_BREADCRUMB_H

#include "venom/widgets/venom_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VenomBreadcrumb VenomBreadcrumb;
typedef void (*VenomBreadcrumbCallback)(VenomBreadcrumb* bc, VenomU32 index, const char* path, void* data);

typedef struct VenomBreadcrumbItem {
    char* label;
    char* path;
} VenomBreadcrumbItem;

struct VenomBreadcrumb {
    VenomWidget base;
    
    VenomBreadcrumbItem* items;
    VenomU32 item_count;
    VenomU32 item_capacity;
    
    char* separator;
    VenomI32 hover_index;
    
    VenomBreadcrumbCallback on_navigate;
    void* callback_data;
    
    VenomColor text_color;
    VenomColor hover_color;
    VenomColor separator_color;
    VenomF32 item_padding;
};

VenomResultPtr venom_breadcrumb_create(void);
VenomResult venom_breadcrumb_add_item(VenomBreadcrumb* bc, const char* label, const char* path);
void venom_breadcrumb_set_path(VenomBreadcrumb* bc, const char* path, char delimiter);
void venom_breadcrumb_pop(VenomBreadcrumb* bc);
void venom_breadcrumb_clear(VenomBreadcrumb* bc);
void venom_breadcrumb_set_separator(VenomBreadcrumb* bc, const char* sep);
void venom_breadcrumb_set_on_navigate(VenomBreadcrumb* bc, VenomBreadcrumbCallback callback, void* data);

extern const VenomWidgetClass venom_breadcrumb_class;

#ifdef __cplusplus
}
#endif

#endif /* VENOM_BREADCRUMB_H */
