/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_accordion.c - Accordion implementation
 */

#include "venom/widgets/venom_accordion.h"
#include "venom/core/venom_memory.h"
#include <string.h>

#define DEFAULT_HEADER_HEIGHT 48.0f
#define INITIAL_CAPACITY 8

static void accordion_init(VenomWidget* widget) {
    VenomAccordion* acc = (VenomAccordion*)widget;
    
    acc->sections = NULL;
    acc->section_count = 0;
    acc->section_capacity = 0;
    acc->allow_multiple = VENOM_FALSE;
    
    acc->header_height = DEFAULT_HEADER_HEIGHT;
    acc->header_bg = (VenomColor){ 245, 245, 245, 255 };
    acc->header_text = (VenomColor){ 33, 33, 33, 255 };
    acc->content_bg = (VenomColor){ 255, 255, 255, 255 };
    acc->border_color = (VenomColor){ 224, 224, 224, 255 };
    acc->corner_radius = 4.0f;
    
    widget->focusable = VENOM_TRUE;
}

static void accordion_destroy(VenomWidget* widget) {
    VenomAccordion* acc = (VenomAccordion*)widget;
    
    for (VenomU32 i = 0; i < acc->section_count; i++) {
        if (acc->sections[i].title) {
            venom_free(acc->sections[i].title, strlen(acc->sections[i].title) + 1);
        }
        if (acc->sections[i].content) {
            acc->sections[i].content->parent = NULL;
            venom_unref(acc->sections[i].content);
        }
    }
    
    if (acc->sections) {
        venom_free(acc->sections, acc->section_capacity * sizeof(VenomAccordionSection));
    }
    
    venom_widget_class.destroy(widget);
}

static void accordion_measure(VenomWidget* widget, VenomF32 available_width, VenomF32 available_height,
                              VenomF32* out_width, VenomF32* out_height) {
    VenomAccordion* acc = (VenomAccordion*)widget;
    (void)available_height;
    
    *out_width = available_width;
    
    VenomF32 total_h = 0;
    for (VenomU32 i = 0; i < acc->section_count; i++) {
        total_h += acc->header_height;
        if (acc->sections[i].expanded && acc->sections[i].content) {
            VenomF32 cw, ch;
            venom_widget_measure(acc->sections[i].content, available_width, 500, &cw, &ch);
            total_h += ch;
        }
    }
    
    *out_height = total_h;
}

static void accordion_layout(VenomWidget* widget, VenomRectF bounds) {
    VenomAccordion* acc = (VenomAccordion*)widget;
    widget->bounds = bounds;
    
    VenomF32 y = 0;
    for (VenomU32 i = 0; i < acc->section_count; i++) {
        y += acc->header_height;
        
        if (acc->sections[i].expanded && acc->sections[i].content) {
            VenomF32 cw, ch;
            venom_widget_measure(acc->sections[i].content, bounds.width, 500, &cw, &ch);
            
            VenomRectF content_bounds = { 0, y, bounds.width, ch };
            venom_widget_layout(acc->sections[i].content, content_bounds);
            y += ch;
        }
    }
}

static void accordion_draw(VenomWidget* widget, VenomCanvas* canvas) {
    VenomAccordion* acc = (VenomAccordion*)widget;
    
    VenomF32 w = widget->bounds.width;
    VenomF32 y = 0;
    
    for (VenomU32 i = 0; i < acc->section_count; i++) {
        /* Draw header */
        VenomRectF header = { 0, y, w, acc->header_height };
        VenomPaint header_paint = venom_paint_fill(acc->header_bg);
        venom_canvas_draw_rect(canvas, header, &header_paint);
        
        /* Draw border */
        VenomPaint border_paint = venom_paint_stroke(acc->border_color, 1.0f);
        venom_canvas_draw_rect(canvas, header, &border_paint);
        
        /* Draw expand/collapse icon */
        const char* icon = acc->sections[i].expanded ? "▼" : "▶";
        VenomPaint icon_paint = venom_paint_fill(acc->header_text);
        venom_canvas_draw_text(canvas, icon, 12, y + acc->header_height / 2 + 5, NULL, &icon_paint);
        
        /* Draw title */
        if (acc->sections[i].title) {
            VenomPaint title_paint = venom_paint_fill(acc->header_text);
            venom_canvas_draw_text(canvas, acc->sections[i].title, 
                                   32, y + acc->header_height / 2 + 5, NULL, &title_paint);
        }
        
        y += acc->header_height;
        
        /* Draw content if expanded */
        if (acc->sections[i].expanded && acc->sections[i].content) {
            VenomWidget* content = acc->sections[i].content;
            
            /* Content background */
            VenomRectF content_bg = { 0, y, w, content->bounds.height };
            VenomPaint bg_paint = venom_paint_fill(acc->content_bg);
            venom_canvas_draw_rect(canvas, content_bg, &bg_paint);
            
            /* Border */
            venom_canvas_draw_rect(canvas, content_bg, &border_paint);
            
            /* Draw content widget */
            venom_canvas_save(canvas);
            venom_canvas_translate(canvas, 0, y);
            venom_widget_draw(content, canvas);
            venom_canvas_restore(canvas);
            
            y += content->bounds.height;
        }
    }
}

static VenomBool accordion_on_event(VenomWidget* widget, const VenomEvent* event) {
    VenomAccordion* acc = (VenomAccordion*)widget;
    
    if (event->type == VENOM_EVENT_MOUSE_BUTTON_DOWN && 
        event->mouse.button == VENOM_MOUSE_BUTTON_LEFT) {
        VenomF32 my = (VenomF32)event->mouse.y;
        VenomF32 y = 0;
        
        for (VenomU32 i = 0; i < acc->section_count; i++) {
            if (my >= y && my < y + acc->header_height) {
                venom_accordion_toggle(acc, i);
                return VENOM_TRUE;
            }
            y += acc->header_height;
            
            if (acc->sections[i].expanded && acc->sections[i].content) {
                y += acc->sections[i].content->bounds.height;
            }
        }
    }
    
    return VENOM_FALSE;
}

const VenomWidgetClass venom_accordion_class = {
    .class_name = "VenomAccordion",
    .instance_size = sizeof(VenomAccordion),
    .parent_class = &venom_widget_class,
    .init = accordion_init,
    .destroy = accordion_destroy,
    .measure = accordion_measure,
    .layout = accordion_layout,
    .draw = accordion_draw,
    .on_event = accordion_on_event,
    .on_state_changed = NULL,
};

VenomResultPtr venom_accordion_create(void) {
    return venom_widget_create(&venom_accordion_class);
}

VenomResult venom_accordion_add_section(VenomAccordion* acc, const char* title, VenomWidget* content) {
    VENOM_ENSURE_NOT_NULL(acc);
    
    if (acc->section_count >= acc->section_capacity) {
        VenomU32 new_cap = acc->section_capacity == 0 ? INITIAL_CAPACITY : acc->section_capacity * 2;
        VenomAccordionSection* new_sections = (VenomAccordionSection*)venom_alloc(
            new_cap * sizeof(VenomAccordionSection));
        if (!new_sections) return VENOM_ERR_UNIT(VENOM_ERROR_OUT_OF_MEMORY);
        
        if (acc->sections) {
            memcpy(new_sections, acc->sections, acc->section_count * sizeof(VenomAccordionSection));
            venom_free(acc->sections, acc->section_capacity * sizeof(VenomAccordionSection));
        }
        
        acc->sections = new_sections;
        acc->section_capacity = new_cap;
    }
    
    VenomAccordionSection* sec = &acc->sections[acc->section_count];
    memset(sec, 0, sizeof(VenomAccordionSection));
    
    if (title) {
        VenomSize len = strlen(title) + 1;
        sec->title = (char*)venom_alloc(len);
        if (sec->title) memcpy(sec->title, title, len);
    }
    
    sec->content = content;
    if (content) {
        venom_ref(content);
        content->parent = (VenomWidget*)acc;
    }
    sec->expanded = VENOM_FALSE;
    
    acc->section_count++;
    return VENOM_OK_UNIT();
}

void venom_accordion_expand(VenomAccordion* acc, VenomU32 index) {
    if (!acc || index >= acc->section_count) return;
    
    if (!acc->allow_multiple) {
        for (VenomU32 i = 0; i < acc->section_count; i++) {
            acc->sections[i].expanded = VENOM_FALSE;
        }
    }
    
    acc->sections[index].expanded = VENOM_TRUE;
    venom_widget_invalidate_layout((VenomWidget*)acc);
}

void venom_accordion_collapse(VenomAccordion* acc, VenomU32 index) {
    if (!acc || index >= acc->section_count) return;
    
    acc->sections[index].expanded = VENOM_FALSE;
    venom_widget_invalidate_layout((VenomWidget*)acc);
}

void venom_accordion_toggle(VenomAccordion* acc, VenomU32 index) {
    if (!acc || index >= acc->section_count) return;
    
    if (acc->sections[index].expanded) {
        venom_accordion_collapse(acc, index);
    } else {
        venom_accordion_expand(acc, index);
    }
}

void venom_accordion_expand_all(VenomAccordion* acc) {
    if (!acc) return;
    for (VenomU32 i = 0; i < acc->section_count; i++) {
        acc->sections[i].expanded = VENOM_TRUE;
    }
    venom_widget_invalidate_layout((VenomWidget*)acc);
}

void venom_accordion_collapse_all(VenomAccordion* acc) {
    if (!acc) return;
    for (VenomU32 i = 0; i < acc->section_count; i++) {
        acc->sections[i].expanded = VENOM_FALSE;
    }
    venom_widget_invalidate_layout((VenomWidget*)acc);
}
