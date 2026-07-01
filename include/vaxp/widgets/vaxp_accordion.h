/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_accordion.h - Accordion widget (collapsible sections)
 */

#ifndef VAXP_ACCORDION_H
#define VAXP_ACCORDION_H

#include "vaxp/widgets/vaxp_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VaxpAccordionSection {
    char* title;
    VaxpWidget* content;
    VaxpBool expanded;
} VaxpAccordionSection;

typedef struct VaxpAccordion {
    VaxpWidget base;
    
    VaxpAccordionSection* sections;
    VaxpU32 section_count;
    VaxpU32 section_capacity;
    
    VaxpBool allow_multiple;     /* Allow multiple open */
    
    /* Styling */
    VaxpF32 header_height;
    VaxpColor header_bg;
    VaxpColor header_text;
    VaxpColor content_bg;
    VaxpColor border_color;
    VaxpF32 corner_radius;
    
} VaxpAccordion;

VaxpResultPtr vaxp_accordion_create(void);
VaxpResult vaxp_accordion_add_section(VaxpAccordion* acc, const char* title, VaxpWidget* content);
void vaxp_accordion_expand(VaxpAccordion* acc, VaxpU32 index);
void vaxp_accordion_collapse(VaxpAccordion* acc, VaxpU32 index);
void vaxp_accordion_toggle(VaxpAccordion* acc, VaxpU32 index);
void vaxp_accordion_expand_all(VaxpAccordion* acc);
void vaxp_accordion_collapse_all(VaxpAccordion* acc);

extern const VaxpWidgetClass vaxp_accordion_class;

#ifdef __cplusplus
}
#endif

#endif /* VAXP_ACCORDION_H */
