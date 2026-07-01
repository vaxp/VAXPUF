/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_accordion.c - Accordion implementation
 */

#include "vaxp/widgets/vaxp_accordion.h"
#include "vaxp/core/vaxp_memory.h"
#include <string.h>

#define DEFAULT_HEADER_HEIGHT 48.0f
#define INITIAL_CAPACITY 8

static void accordion_init(VaxpWidget* widget) {
    VaxpAccordion* acc = (VaxpAccordion*)widget;
    
    acc->sections = NULL;
    acc->section_count = 0;
    acc->section_capacity = 0;
    acc->allow_multiple = VAXP_FALSE;
    
    acc->header_height = DEFAULT_HEADER_HEIGHT;
    acc->header_bg = (VaxpColor){ 245, 245, 245, 255 };
    acc->header_text = (VaxpColor){ 33, 33, 33, 255 };
    acc->content_bg = (VaxpColor){ 255, 255, 255, 255 };
    acc->border_color = (VaxpColor){ 224, 224, 224, 255 };
    acc->corner_radius = 4.0f;
    
    widget->focusable = VAXP_TRUE;
}

static void accordion_destroy(VaxpWidget* widget) {
    VaxpAccordion* acc = (VaxpAccordion*)widget;
    
    for (VaxpU32 i = 0; i < acc->section_count; i++) {
        if (acc->sections[i].title) {
            vaxp_free(acc->sections[i].title, strlen(acc->sections[i].title) + 1);
        }
        if (acc->sections[i].content) {
            acc->sections[i].content->parent = NULL;
            vaxp_unref(acc->sections[i].content);
        }
    }
    
    if (acc->sections) {
        vaxp_free(acc->sections, acc->section_capacity * sizeof(VaxpAccordionSection));
    }
    
    vaxp_widget_class.destroy(widget);
}

static void accordion_measure(VaxpWidget* widget, VaxpF32 available_width, VaxpF32 available_height,
                              VaxpF32* out_width, VaxpF32* out_height) {
    VaxpAccordion* acc = (VaxpAccordion*)widget;
    (void)available_height;
    
    *out_width = available_width;
    
    VaxpF32 total_h = 0;
    for (VaxpU32 i = 0; i < acc->section_count; i++) {
        total_h += acc->header_height;
        if (acc->sections[i].expanded && acc->sections[i].content) {
            VaxpF32 cw, ch;
            vaxp_widget_measure(acc->sections[i].content, available_width, 500, &cw, &ch);
            total_h += ch;
        }
    }
    
    *out_height = total_h;
}

static void accordion_layout(VaxpWidget* widget, VaxpRectF bounds) {
    VaxpAccordion* acc = (VaxpAccordion*)widget;
    widget->bounds = bounds;
    
    VaxpF32 y = 0;
    for (VaxpU32 i = 0; i < acc->section_count; i++) {
        y += acc->header_height;
        
        if (acc->sections[i].expanded && acc->sections[i].content) {
            VaxpF32 cw, ch;
            vaxp_widget_measure(acc->sections[i].content, bounds.width, 500, &cw, &ch);
            
            VaxpRectF content_bounds = { 0, y, bounds.width, ch };
            vaxp_widget_layout(acc->sections[i].content, content_bounds);
            y += ch;
        }
    }
}

static void accordion_draw(VaxpWidget* widget, VaxpCanvas* canvas) {
    VaxpAccordion* acc = (VaxpAccordion*)widget;
    
    VaxpF32 w = widget->bounds.width;
    VaxpF32 y = 0;
    
    for (VaxpU32 i = 0; i < acc->section_count; i++) {
        /* Draw header */
        VaxpRectF header = { 0, y, w, acc->header_height };
        VaxpPaint header_paint = vaxp_paint_fill(acc->header_bg);
        vaxp_canvas_draw_rect(canvas, header, &header_paint);
        
        /* Draw border */
        VaxpPaint border_paint = vaxp_paint_stroke(acc->border_color, 1.0f);
        vaxp_canvas_draw_rect(canvas, header, &border_paint);
        
        /* Draw expand/collapse icon */
        const char* icon = acc->sections[i].expanded ? "▼" : "▶";
        VaxpPaint icon_paint = vaxp_paint_fill(acc->header_text);
        vaxp_canvas_draw_text(canvas, icon, 12, y + acc->header_height / 2 + 5, NULL, &icon_paint);
        
        /* Draw title */
        if (acc->sections[i].title) {
            VaxpPaint title_paint = vaxp_paint_fill(acc->header_text);
            vaxp_canvas_draw_text(canvas, acc->sections[i].title, 
                                   32, y + acc->header_height / 2 + 5, NULL, &title_paint);
        }
        
        y += acc->header_height;
        
        /* Draw content if expanded */
        if (acc->sections[i].expanded && acc->sections[i].content) {
            VaxpWidget* content = acc->sections[i].content;
            
            /* Content background */
            VaxpRectF content_bg = { 0, y, w, content->bounds.height };
            VaxpPaint bg_paint = vaxp_paint_fill(acc->content_bg);
            vaxp_canvas_draw_rect(canvas, content_bg, &bg_paint);
            
            /* Border */
            vaxp_canvas_draw_rect(canvas, content_bg, &border_paint);
            
            /* Draw content widget */
            vaxp_canvas_save(canvas);
            vaxp_canvas_translate(canvas, 0, y);
            vaxp_widget_draw(content, canvas);
            vaxp_canvas_restore(canvas);
            
            y += content->bounds.height;
        }
    }
}

static VaxpBool accordion_on_event(VaxpWidget* widget, const VaxpEvent* event) {
    VaxpAccordion* acc = (VaxpAccordion*)widget;
    
    if (event->type == VAXP_EVENT_MOUSE_BUTTON_DOWN && 
        event->mouse.button == VAXP_MOUSE_BUTTON_LEFT) {
        VaxpF32 my = (VaxpF32)event->mouse.y;
        VaxpF32 y = 0;
        
        for (VaxpU32 i = 0; i < acc->section_count; i++) {
            if (my >= y && my < y + acc->header_height) {
                vaxp_accordion_toggle(acc, i);
                return VAXP_TRUE;
            }
            y += acc->header_height;
            
            if (acc->sections[i].expanded && acc->sections[i].content) {
                y += acc->sections[i].content->bounds.height;
            }
        }
    }
    
    return VAXP_FALSE;
}

const VaxpWidgetClass vaxp_accordion_class = {
    .class_name = "VaxpAccordion",
    .instance_size = sizeof(VaxpAccordion),
    .parent_class = &vaxp_widget_class,
    .init = accordion_init,
    .destroy = accordion_destroy,
    .measure = accordion_measure,
    .layout = accordion_layout,
    .draw = accordion_draw,
    .on_event = accordion_on_event,
    .on_state_changed = NULL,
};

VaxpResultPtr vaxp_accordion_create(void) {
    return vaxp_widget_create(&vaxp_accordion_class);
}

VaxpResult vaxp_accordion_add_section(VaxpAccordion* acc, const char* title, VaxpWidget* content) {
    VAXP_ENSURE_NOT_NULL(acc);
    
    if (acc->section_count >= acc->section_capacity) {
        VaxpU32 new_cap = acc->section_capacity == 0 ? INITIAL_CAPACITY : acc->section_capacity * 2;
        VaxpAccordionSection* new_sections = (VaxpAccordionSection*)vaxp_alloc(
            new_cap * sizeof(VaxpAccordionSection));
        if (!new_sections) return VAXP_ERR_UNIT(VAXP_ERROR_OUT_OF_MEMORY);
        
        if (acc->sections) {
            memcpy(new_sections, acc->sections, acc->section_count * sizeof(VaxpAccordionSection));
            vaxp_free(acc->sections, acc->section_capacity * sizeof(VaxpAccordionSection));
        }
        
        acc->sections = new_sections;
        acc->section_capacity = new_cap;
    }
    
    VaxpAccordionSection* sec = &acc->sections[acc->section_count];
    memset(sec, 0, sizeof(VaxpAccordionSection));
    
    if (title) {
        VaxpSize len = strlen(title) + 1;
        sec->title = (char*)vaxp_alloc(len);
        if (sec->title) memcpy(sec->title, title, len);
    }
    
    sec->content = content;
    if (content) {
        vaxp_ref(content);
        content->parent = (VaxpWidget*)acc;
    }
    sec->expanded = VAXP_FALSE;
    
    acc->section_count++;
    return VAXP_OK_UNIT();
}

void vaxp_accordion_expand(VaxpAccordion* acc, VaxpU32 index) {
    if (!acc || index >= acc->section_count) return;
    
    if (!acc->allow_multiple) {
        for (VaxpU32 i = 0; i < acc->section_count; i++) {
            acc->sections[i].expanded = VAXP_FALSE;
        }
    }
    
    acc->sections[index].expanded = VAXP_TRUE;
    vaxp_widget_invalidate_layout((VaxpWidget*)acc);
}

void vaxp_accordion_collapse(VaxpAccordion* acc, VaxpU32 index) {
    if (!acc || index >= acc->section_count) return;
    
    acc->sections[index].expanded = VAXP_FALSE;
    vaxp_widget_invalidate_layout((VaxpWidget*)acc);
}

void vaxp_accordion_toggle(VaxpAccordion* acc, VaxpU32 index) {
    if (!acc || index >= acc->section_count) return;
    
    if (acc->sections[index].expanded) {
        vaxp_accordion_collapse(acc, index);
    } else {
        vaxp_accordion_expand(acc, index);
    }
}

void vaxp_accordion_expand_all(VaxpAccordion* acc) {
    if (!acc) return;
    for (VaxpU32 i = 0; i < acc->section_count; i++) {
        acc->sections[i].expanded = VAXP_TRUE;
    }
    vaxp_widget_invalidate_layout((VaxpWidget*)acc);
}

void vaxp_accordion_collapse_all(VaxpAccordion* acc) {
    if (!acc) return;
    for (VaxpU32 i = 0; i < acc->section_count; i++) {
        acc->sections[i].expanded = VAXP_FALSE;
    }
    vaxp_widget_invalidate_layout((VaxpWidget*)acc);
}
