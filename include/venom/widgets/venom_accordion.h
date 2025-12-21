/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_accordion.h - Accordion widget (collapsible sections)
 */

#ifndef VENOM_ACCORDION_H
#define VENOM_ACCORDION_H

#include "venom/widgets/venom_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VenomAccordionSection {
    char* title;
    VenomWidget* content;
    VenomBool expanded;
} VenomAccordionSection;

typedef struct VenomAccordion {
    VenomWidget base;
    
    VenomAccordionSection* sections;
    VenomU32 section_count;
    VenomU32 section_capacity;
    
    VenomBool allow_multiple;     /* Allow multiple open */
    
    /* Styling */
    VenomF32 header_height;
    VenomColor header_bg;
    VenomColor header_text;
    VenomColor content_bg;
    VenomColor border_color;
    VenomF32 corner_radius;
    
} VenomAccordion;

VenomResultPtr venom_accordion_create(void);
VenomResult venom_accordion_add_section(VenomAccordion* acc, const char* title, VenomWidget* content);
void venom_accordion_expand(VenomAccordion* acc, VenomU32 index);
void venom_accordion_collapse(VenomAccordion* acc, VenomU32 index);
void venom_accordion_toggle(VenomAccordion* acc, VenomU32 index);
void venom_accordion_expand_all(VenomAccordion* acc);
void venom_accordion_collapse_all(VenomAccordion* acc);

extern const VenomWidgetClass venom_accordion_class;

#ifdef __cplusplus
}
#endif

#endif /* VENOM_ACCORDION_H */
