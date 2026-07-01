/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_dropdown.c - Dropdown selection implementation
 */

#include "vaxp/widgets/vaxp_dropdown.h"
#include "vaxp/core/vaxp_memory.h"
#include <string.h>

#define DEFAULT_HEIGHT 40.0f
#define DEFAULT_ITEM_HEIGHT 36.0f
#define DEFAULT_MAX_DROPDOWN_HEIGHT 200.0f
#define INITIAL_CAPACITY 8

static void dropdown_init(VaxpWidget* widget) {
    VaxpDropdown* dd = (VaxpDropdown*)widget;
    
    dd->items = NULL;
    dd->item_count = 0;
    dd->item_capacity = 0;
    
    dd->selected_index = -1;
    dd->placeholder = NULL;
    
    dd->is_open = VAXP_FALSE;
    dd->hover_index = -1;
    dd->max_dropdown_height = DEFAULT_MAX_DROPDOWN_HEIGHT;
    dd->scroll_offset = 0;
    
    dd->height = DEFAULT_HEIGHT;
    dd->item_height = DEFAULT_ITEM_HEIGHT;
    dd->padding = 12.0f;
    dd->corner_radius = 4.0f;
    dd->background_color = (VaxpColor){ 255, 255, 255, 255 };
    dd->border_color = (VaxpColor){ 189, 189, 189, 255 };
    dd->text_color = (VaxpColor){ 33, 33, 33, 255 };
    dd->placeholder_color = (VaxpColor){ 158, 158, 158, 255 };
    dd->dropdown_bg = (VaxpColor){ 255, 255, 255, 255 };
    dd->hover_color = (VaxpColor){ 63, 81, 181, 40 };
    dd->selected_bg = (VaxpColor){ 63, 81, 181, 80 };
    
    dd->on_change = NULL;
    dd->callback_data = NULL;
    
    widget->focusable = VAXP_TRUE;
}

static void dropdown_destroy(VaxpWidget* widget) {
    VaxpDropdown* dd = (VaxpDropdown*)widget;
    
    for (VaxpU32 i = 0; i < dd->item_count; i++) {
        if (dd->items[i].label) {
            vaxp_free(dd->items[i].label, strlen(dd->items[i].label) + 1);
        }
        if (dd->items[i].value) {
            vaxp_free(dd->items[i].value, strlen(dd->items[i].value) + 1);
        }
    }
    
    if (dd->items) {
        vaxp_free(dd->items, dd->item_capacity * sizeof(VaxpDropdownItem));
        dd->items = NULL;
    }
    
    if (dd->placeholder) {
        vaxp_free(dd->placeholder, strlen(dd->placeholder) + 1);
        dd->placeholder = NULL;
    }
    
    vaxp_widget_class.destroy(widget);
}

static void dropdown_measure(VaxpWidget* widget, VaxpF32 available_width, VaxpF32 available_height,
                             VaxpF32* out_width, VaxpF32* out_height) {
    VaxpDropdown* dd = (VaxpDropdown*)widget;
    (void)available_height;
    
    *out_width = widget->layout.preferred_width > 0 ? 
                 widget->layout.preferred_width : available_width;
    *out_height = dd->height;
}

static void dropdown_draw(VaxpWidget* widget, VaxpCanvas* canvas) {
    VaxpDropdown* dd = (VaxpDropdown*)widget;
    
    VaxpF32 w = widget->bounds.width;
    VaxpF32 h = dd->height;
    
    /* Draw main button background */
    VaxpRectF bg = { 0, 0, w, h };
    VaxpPaint bg_paint = vaxp_paint_fill(dd->background_color);
    vaxp_canvas_draw_rounded_rect(canvas, bg, dd->corner_radius, &bg_paint);
    
    /* Draw border */
    VaxpColor border_color = dd->border_color;
    if (widget->state & VAXP_WIDGET_STATE_FOCUSED || dd->is_open) {
        border_color = (VaxpColor){ 63, 81, 181, 255 };
    }
    VaxpPaint border_paint = vaxp_paint_stroke(border_color, 1.0f);
    vaxp_canvas_draw_rounded_rect(canvas, bg, dd->corner_radius, &border_paint);
    
    /* Draw selected text or placeholder */
    VaxpF32 text_y = h / 2 + 5;
    if (dd->selected_index >= 0 && (VaxpU32)dd->selected_index < dd->item_count) {
        VaxpPaint text_paint = vaxp_paint_fill(dd->text_color);
        vaxp_canvas_draw_text(canvas, dd->items[dd->selected_index].label, 
                               dd->padding, text_y, NULL, &text_paint);
    } else if (dd->placeholder) {
        VaxpPaint ph_paint = vaxp_paint_fill(dd->placeholder_color);
        vaxp_canvas_draw_text(canvas, dd->placeholder, dd->padding, text_y, NULL, &ph_paint);
    }
    
    /* Draw dropdown arrow */
    VaxpPaint arrow_paint = vaxp_paint_fill(dd->text_color);
    const char* arrow = dd->is_open ? "▲" : "▼";
    vaxp_canvas_draw_text(canvas, arrow, w - dd->padding - 12, text_y, NULL, &arrow_paint);
    
    /* Draw dropdown list if open */
    if (dd->is_open && dd->item_count > 0) {
        VaxpF32 list_h = dd->item_count * dd->item_height;
        if (list_h > dd->max_dropdown_height) {
            list_h = dd->max_dropdown_height;
        }
        
        VaxpF32 list_y = h + 2;
        
        /* Draw dropdown shadow */
        VaxpRectF shadow = { 3, list_y + 3, w, list_h };
        VaxpPaint shadow_paint = vaxp_paint_fill((VaxpColor){ 0, 0, 0, 40 });
        vaxp_canvas_draw_rounded_rect(canvas, shadow, dd->corner_radius, &shadow_paint);
        
        /* Draw dropdown background */
        VaxpRectF list_bg = { 0, list_y, w, list_h };
        VaxpPaint list_paint = vaxp_paint_fill(dd->dropdown_bg);
        vaxp_canvas_draw_rounded_rect(canvas, list_bg, dd->corner_radius, &list_paint);
        
        /* Clip to list area */
        vaxp_canvas_save(canvas);
        vaxp_canvas_clip_rect(canvas, list_bg);
        
        /* Draw items */
        for (VaxpU32 i = 0; i < dd->item_count; i++) {
            VaxpF32 item_y = list_y + i * dd->item_height - dd->scroll_offset;
            
            if (item_y + dd->item_height < list_y || item_y > list_y + list_h) {
                continue;  /* Outside visible area */
            }
            
            VaxpRectF item_rect = { 0, item_y, w, dd->item_height };
            
            /* Draw hover/selected highlight */
            if ((VaxpI32)i == dd->hover_index) {
                VaxpPaint hover_paint = vaxp_paint_fill(dd->hover_color);
                vaxp_canvas_draw_rect(canvas, item_rect, &hover_paint);
            } else if ((VaxpI32)i == dd->selected_index) {
                VaxpPaint sel_paint = vaxp_paint_fill(dd->selected_bg);
                vaxp_canvas_draw_rect(canvas, item_rect, &sel_paint);
            }
            
            /* Draw item text */
            VaxpColor item_color = dd->items[i].enabled ? dd->text_color : dd->placeholder_color;
            VaxpPaint item_paint = vaxp_paint_fill(item_color);
            vaxp_canvas_draw_text(canvas, dd->items[i].label ? dd->items[i].label : "",
                                   dd->padding, item_y + dd->item_height / 2 + 5, NULL, &item_paint);
        }
        
        vaxp_canvas_restore(canvas);
        
        /* Draw border */
        VaxpPaint list_border = vaxp_paint_stroke(dd->border_color, 1.0f);
        vaxp_canvas_draw_rounded_rect(canvas, list_bg, dd->corner_radius, &list_border);
    }
}

static VaxpBool dropdown_on_event(VaxpWidget* widget, const VaxpEvent* event) {
    VaxpDropdown* dd = (VaxpDropdown*)widget;
    
    switch (event->type) {
        case VAXP_EVENT_MOUSE_BUTTON_DOWN:
            if (event->mouse.button == VAXP_MOUSE_BUTTON_LEFT) {
                VaxpF32 my = (VaxpF32)event->mouse.y;
                
                if (dd->is_open) {
                    /* Check if clicking on an item */
                    VaxpF32 list_y = dd->height + 2;
                    VaxpF32 list_h = dd->item_count * dd->item_height;
                    if (list_h > dd->max_dropdown_height) list_h = dd->max_dropdown_height;
                    
                    if (my >= list_y && my < list_y + list_h) {
                        VaxpI32 clicked = (VaxpI32)((my - list_y + dd->scroll_offset) / dd->item_height);
                        if (clicked >= 0 && (VaxpU32)clicked < dd->item_count) {
                            if (dd->items[clicked].enabled) {
                                dd->selected_index = clicked;
                                dd->is_open = VAXP_FALSE;
                                widget->needs_redraw = VAXP_TRUE;
                                
                                if (dd->on_change) {
                                    dd->on_change(dd, clicked, 
                                                  dd->items[clicked].value ? dd->items[clicked].value : dd->items[clicked].label,
                                                  dd->callback_data);
                                }
                            }
                        }
                    } else {
                        dd->is_open = VAXP_FALSE;
                        widget->needs_redraw = VAXP_TRUE;
                    }
                } else {
                    /* Toggle dropdown */
                    dd->is_open = VAXP_TRUE;
                    dd->hover_index = dd->selected_index;
                    widget->needs_redraw = VAXP_TRUE;
                }
                return VAXP_TRUE;
            }
            break;
            
        case VAXP_EVENT_MOUSE_MOVE:
            if (dd->is_open) {
                VaxpF32 my = (VaxpF32)event->mouse.y;
                VaxpF32 list_y = dd->height + 2;
                
                VaxpI32 new_hover = (VaxpI32)((my - list_y + dd->scroll_offset) / dd->item_height);
                if (new_hover < 0 || (VaxpU32)new_hover >= dd->item_count) {
                    new_hover = -1;
                }
                
                if (new_hover != dd->hover_index) {
                    dd->hover_index = new_hover;
                    widget->needs_redraw = VAXP_TRUE;
                }
            }
            break;
            
        case VAXP_EVENT_KEY_DOWN:
            switch (event->key.key) {
                case VAXP_KEY_RETURN:
                case VAXP_KEY_SPACE:
                    if (!dd->is_open) {
                        dd->is_open = VAXP_TRUE;
                        dd->hover_index = dd->selected_index >= 0 ? dd->selected_index : 0;
                    } else if (dd->hover_index >= 0) {
                        dd->selected_index = dd->hover_index;
                        dd->is_open = VAXP_FALSE;
                        if (dd->on_change && (VaxpU32)dd->selected_index < dd->item_count) {
                            dd->on_change(dd, dd->selected_index,
                                          dd->items[dd->selected_index].value,
                                          dd->callback_data);
                        }
                    }
                    widget->needs_redraw = VAXP_TRUE;
                    return VAXP_TRUE;
                    
                case VAXP_KEY_ESCAPE:
                    if (dd->is_open) {
                        dd->is_open = VAXP_FALSE;
                        widget->needs_redraw = VAXP_TRUE;
                        return VAXP_TRUE;
                    }
                    break;
                    
                case VAXP_KEY_UP:
                    if (dd->is_open && dd->hover_index > 0) {
                        dd->hover_index--;
                        widget->needs_redraw = VAXP_TRUE;
                    }
                    return VAXP_TRUE;
                    
                case VAXP_KEY_DOWN:
                    if (dd->is_open && dd->hover_index < (VaxpI32)dd->item_count - 1) {
                        dd->hover_index++;
                        widget->needs_redraw = VAXP_TRUE;
                    } else if (!dd->is_open) {
                        dd->is_open = VAXP_TRUE;
                        dd->hover_index = 0;
                        widget->needs_redraw = VAXP_TRUE;
                    }
                    return VAXP_TRUE;
                    
                default:
                    break;
            }
            break;
            
        case VAXP_EVENT_MOUSE_SCROLL:
            if (dd->is_open) {
                dd->scroll_offset -= event->scroll.y * dd->item_height;
                if (dd->scroll_offset < 0) dd->scroll_offset = 0;
                
                VaxpF32 max_scroll = dd->item_count * dd->item_height - dd->max_dropdown_height;
                if (max_scroll < 0) max_scroll = 0;
                if (dd->scroll_offset > max_scroll) dd->scroll_offset = max_scroll;
                
                widget->needs_redraw = VAXP_TRUE;
                return VAXP_TRUE;
            }
            break;
            
        default:
            break;
    }
    
    return VAXP_FALSE;
}

const VaxpWidgetClass vaxp_dropdown_class = {
    .class_name = "VaxpDropdown",
    .instance_size = sizeof(VaxpDropdown),
    .parent_class = &vaxp_widget_class,
    .init = dropdown_init,
    .destroy = dropdown_destroy,
    .measure = dropdown_measure,
    .layout = NULL,
    .draw = dropdown_draw,
    .on_event = dropdown_on_event,
    .on_state_changed = NULL,
};

VaxpResultPtr vaxp_dropdown_create(void) {
    return vaxp_widget_create(&vaxp_dropdown_class);
}

VaxpResult vaxp_dropdown_add_item(VaxpDropdown* dd, const char* label, const char* value) {
    VAXP_ENSURE_NOT_NULL(dd);
    
    if (dd->item_count >= dd->item_capacity) {
        VaxpU32 new_cap = dd->item_capacity == 0 ? INITIAL_CAPACITY : dd->item_capacity * 2;
        VaxpDropdownItem* new_items = (VaxpDropdownItem*)vaxp_alloc(new_cap * sizeof(VaxpDropdownItem));
        if (!new_items) return VAXP_ERR_UNIT(VAXP_ERROR_OUT_OF_MEMORY);
        
        if (dd->items) {
            memcpy(new_items, dd->items, dd->item_count * sizeof(VaxpDropdownItem));
            vaxp_free(dd->items, dd->item_capacity * sizeof(VaxpDropdownItem));
        }
        
        dd->items = new_items;
        dd->item_capacity = new_cap;
    }
    
    VaxpDropdownItem* item = &dd->items[dd->item_count];
    memset(item, 0, sizeof(VaxpDropdownItem));
    
    if (label) {
        VaxpSize len = strlen(label) + 1;
        item->label = (char*)vaxp_alloc(len);
        if (item->label) memcpy(item->label, label, len);
    }
    
    if (value) {
        VaxpSize len = strlen(value) + 1;
        item->value = (char*)vaxp_alloc(len);
        if (item->value) memcpy(item->value, value, len);
    }
    
    item->enabled = VAXP_TRUE;
    dd->item_count++;
    
    return VAXP_OK_UNIT();
}

void vaxp_dropdown_clear(VaxpDropdown* dd) {
    if (!dd) return;
    
    for (VaxpU32 i = 0; i < dd->item_count; i++) {
        if (dd->items[i].label) {
            vaxp_free(dd->items[i].label, strlen(dd->items[i].label) + 1);
        }
        if (dd->items[i].value) {
            vaxp_free(dd->items[i].value, strlen(dd->items[i].value) + 1);
        }
    }
    
    dd->item_count = 0;
    dd->selected_index = -1;
}

void vaxp_dropdown_set_selected(VaxpDropdown* dd, VaxpI32 index) {
    if (dd) {
        dd->selected_index = index;
        vaxp_widget_invalidate((VaxpWidget*)dd);
    }
}

VaxpI32 vaxp_dropdown_get_selected(const VaxpDropdown* dd) {
    return dd ? dd->selected_index : -1;
}

const char* vaxp_dropdown_get_selected_value(const VaxpDropdown* dd) {
    if (!dd || dd->selected_index < 0 || (VaxpU32)dd->selected_index >= dd->item_count) {
        return NULL;
    }
    return dd->items[dd->selected_index].value ? 
           dd->items[dd->selected_index].value : 
           dd->items[dd->selected_index].label;
}

const char* vaxp_dropdown_get_selected_label(const VaxpDropdown* dd) {
    if (!dd || dd->selected_index < 0 || (VaxpU32)dd->selected_index >= dd->item_count) {
        return NULL;
    }
    return dd->items[dd->selected_index].label;
}

void vaxp_dropdown_set_placeholder(VaxpDropdown* dd, const char* placeholder) {
    if (!dd) return;
    
    if (dd->placeholder) {
        vaxp_free(dd->placeholder, strlen(dd->placeholder) + 1);
        dd->placeholder = NULL;
    }
    
    if (placeholder) {
        VaxpSize len = strlen(placeholder) + 1;
        dd->placeholder = (char*)vaxp_alloc(len);
        if (dd->placeholder) memcpy(dd->placeholder, placeholder, len);
    }
}

void vaxp_dropdown_set_on_change(VaxpDropdown* dd, VaxpDropdownCallback callback, void* data) {
    if (dd) {
        dd->on_change = callback;
        dd->callback_data = data;
    }
}

VaxpWidget* _vaxp_dropdown_build(const VaxpDropdownConfig* config) {
    VaxpResultPtr result = vaxp_dropdown_create();
    if (!result.ok) return NULL;
    
    VaxpDropdown* dd = (VaxpDropdown*)result.value;
    
    if (config->placeholder) vaxp_dropdown_set_placeholder(dd, config->placeholder);
    dd->selected_index = config->selected;
    dd->on_change = config->on_change;
    dd->callback_data = config->data;
    
    return (VaxpWidget*)dd;
}
