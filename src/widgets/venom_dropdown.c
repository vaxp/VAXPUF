/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_dropdown.c - Dropdown selection implementation
 */

#include "venom/widgets/venom_dropdown.h"
#include "venom/core/venom_memory.h"
#include <string.h>

#define DEFAULT_HEIGHT 40.0f
#define DEFAULT_ITEM_HEIGHT 36.0f
#define DEFAULT_MAX_DROPDOWN_HEIGHT 200.0f
#define INITIAL_CAPACITY 8

static void dropdown_init(VenomWidget* widget) {
    VenomDropdown* dd = (VenomDropdown*)widget;
    
    dd->items = NULL;
    dd->item_count = 0;
    dd->item_capacity = 0;
    
    dd->selected_index = -1;
    dd->placeholder = NULL;
    
    dd->is_open = VENOM_FALSE;
    dd->hover_index = -1;
    dd->max_dropdown_height = DEFAULT_MAX_DROPDOWN_HEIGHT;
    dd->scroll_offset = 0;
    
    dd->height = DEFAULT_HEIGHT;
    dd->item_height = DEFAULT_ITEM_HEIGHT;
    dd->padding = 12.0f;
    dd->corner_radius = 4.0f;
    dd->background_color = (VenomColor){ 255, 255, 255, 255 };
    dd->border_color = (VenomColor){ 189, 189, 189, 255 };
    dd->text_color = (VenomColor){ 33, 33, 33, 255 };
    dd->placeholder_color = (VenomColor){ 158, 158, 158, 255 };
    dd->dropdown_bg = (VenomColor){ 255, 255, 255, 255 };
    dd->hover_color = (VenomColor){ 63, 81, 181, 40 };
    dd->selected_bg = (VenomColor){ 63, 81, 181, 80 };
    
    dd->on_change = NULL;
    dd->callback_data = NULL;
    
    widget->focusable = VENOM_TRUE;
}

static void dropdown_destroy(VenomWidget* widget) {
    VenomDropdown* dd = (VenomDropdown*)widget;
    
    for (VenomU32 i = 0; i < dd->item_count; i++) {
        if (dd->items[i].label) {
            venom_free(dd->items[i].label, strlen(dd->items[i].label) + 1);
        }
        if (dd->items[i].value) {
            venom_free(dd->items[i].value, strlen(dd->items[i].value) + 1);
        }
    }
    
    if (dd->items) {
        venom_free(dd->items, dd->item_capacity * sizeof(VenomDropdownItem));
        dd->items = NULL;
    }
    
    if (dd->placeholder) {
        venom_free(dd->placeholder, strlen(dd->placeholder) + 1);
        dd->placeholder = NULL;
    }
    
    venom_widget_class.destroy(widget);
}

static void dropdown_measure(VenomWidget* widget, VenomF32 available_width, VenomF32 available_height,
                             VenomF32* out_width, VenomF32* out_height) {
    VenomDropdown* dd = (VenomDropdown*)widget;
    (void)available_height;
    
    *out_width = widget->layout.preferred_width > 0 ? 
                 widget->layout.preferred_width : available_width;
    *out_height = dd->height;
}

static void dropdown_draw(VenomWidget* widget, VenomCanvas* canvas) {
    VenomDropdown* dd = (VenomDropdown*)widget;
    
    VenomF32 w = widget->bounds.width;
    VenomF32 h = dd->height;
    
    /* Draw main button background */
    VenomRectF bg = { 0, 0, w, h };
    VenomPaint bg_paint = venom_paint_fill(dd->background_color);
    venom_canvas_draw_rounded_rect(canvas, bg, dd->corner_radius, &bg_paint);
    
    /* Draw border */
    VenomColor border_color = dd->border_color;
    if (widget->state & VENOM_WIDGET_STATE_FOCUSED || dd->is_open) {
        border_color = (VenomColor){ 63, 81, 181, 255 };
    }
    VenomPaint border_paint = venom_paint_stroke(border_color, 1.0f);
    venom_canvas_draw_rounded_rect(canvas, bg, dd->corner_radius, &border_paint);
    
    /* Draw selected text or placeholder */
    VenomF32 text_y = h / 2 + 5;
    if (dd->selected_index >= 0 && (VenomU32)dd->selected_index < dd->item_count) {
        VenomPaint text_paint = venom_paint_fill(dd->text_color);
        venom_canvas_draw_text(canvas, dd->items[dd->selected_index].label, 
                               dd->padding, text_y, NULL, &text_paint);
    } else if (dd->placeholder) {
        VenomPaint ph_paint = venom_paint_fill(dd->placeholder_color);
        venom_canvas_draw_text(canvas, dd->placeholder, dd->padding, text_y, NULL, &ph_paint);
    }
    
    /* Draw dropdown arrow */
    VenomPaint arrow_paint = venom_paint_fill(dd->text_color);
    const char* arrow = dd->is_open ? "▲" : "▼";
    venom_canvas_draw_text(canvas, arrow, w - dd->padding - 12, text_y, NULL, &arrow_paint);
    
    /* Draw dropdown list if open */
    if (dd->is_open && dd->item_count > 0) {
        VenomF32 list_h = dd->item_count * dd->item_height;
        if (list_h > dd->max_dropdown_height) {
            list_h = dd->max_dropdown_height;
        }
        
        VenomF32 list_y = h + 2;
        
        /* Draw dropdown shadow */
        VenomRectF shadow = { 3, list_y + 3, w, list_h };
        VenomPaint shadow_paint = venom_paint_fill((VenomColor){ 0, 0, 0, 40 });
        venom_canvas_draw_rounded_rect(canvas, shadow, dd->corner_radius, &shadow_paint);
        
        /* Draw dropdown background */
        VenomRectF list_bg = { 0, list_y, w, list_h };
        VenomPaint list_paint = venom_paint_fill(dd->dropdown_bg);
        venom_canvas_draw_rounded_rect(canvas, list_bg, dd->corner_radius, &list_paint);
        
        /* Clip to list area */
        venom_canvas_save(canvas);
        venom_canvas_clip_rect(canvas, list_bg);
        
        /* Draw items */
        for (VenomU32 i = 0; i < dd->item_count; i++) {
            VenomF32 item_y = list_y + i * dd->item_height - dd->scroll_offset;
            
            if (item_y + dd->item_height < list_y || item_y > list_y + list_h) {
                continue;  /* Outside visible area */
            }
            
            VenomRectF item_rect = { 0, item_y, w, dd->item_height };
            
            /* Draw hover/selected highlight */
            if ((VenomI32)i == dd->hover_index) {
                VenomPaint hover_paint = venom_paint_fill(dd->hover_color);
                venom_canvas_draw_rect(canvas, item_rect, &hover_paint);
            } else if ((VenomI32)i == dd->selected_index) {
                VenomPaint sel_paint = venom_paint_fill(dd->selected_bg);
                venom_canvas_draw_rect(canvas, item_rect, &sel_paint);
            }
            
            /* Draw item text */
            VenomColor item_color = dd->items[i].enabled ? dd->text_color : dd->placeholder_color;
            VenomPaint item_paint = venom_paint_fill(item_color);
            venom_canvas_draw_text(canvas, dd->items[i].label ? dd->items[i].label : "",
                                   dd->padding, item_y + dd->item_height / 2 + 5, NULL, &item_paint);
        }
        
        venom_canvas_restore(canvas);
        
        /* Draw border */
        VenomPaint list_border = venom_paint_stroke(dd->border_color, 1.0f);
        venom_canvas_draw_rounded_rect(canvas, list_bg, dd->corner_radius, &list_border);
    }
}

static VenomBool dropdown_on_event(VenomWidget* widget, const VenomEvent* event) {
    VenomDropdown* dd = (VenomDropdown*)widget;
    
    switch (event->type) {
        case VENOM_EVENT_MOUSE_BUTTON_DOWN:
            if (event->mouse.button == VENOM_MOUSE_BUTTON_LEFT) {
                VenomF32 my = (VenomF32)event->mouse.y;
                
                if (dd->is_open) {
                    /* Check if clicking on an item */
                    VenomF32 list_y = dd->height + 2;
                    VenomF32 list_h = dd->item_count * dd->item_height;
                    if (list_h > dd->max_dropdown_height) list_h = dd->max_dropdown_height;
                    
                    if (my >= list_y && my < list_y + list_h) {
                        VenomI32 clicked = (VenomI32)((my - list_y + dd->scroll_offset) / dd->item_height);
                        if (clicked >= 0 && (VenomU32)clicked < dd->item_count) {
                            if (dd->items[clicked].enabled) {
                                dd->selected_index = clicked;
                                dd->is_open = VENOM_FALSE;
                                widget->needs_redraw = VENOM_TRUE;
                                
                                if (dd->on_change) {
                                    dd->on_change(dd, clicked, 
                                                  dd->items[clicked].value ? dd->items[clicked].value : dd->items[clicked].label,
                                                  dd->callback_data);
                                }
                            }
                        }
                    } else {
                        dd->is_open = VENOM_FALSE;
                        widget->needs_redraw = VENOM_TRUE;
                    }
                } else {
                    /* Toggle dropdown */
                    dd->is_open = VENOM_TRUE;
                    dd->hover_index = dd->selected_index;
                    widget->needs_redraw = VENOM_TRUE;
                }
                return VENOM_TRUE;
            }
            break;
            
        case VENOM_EVENT_MOUSE_MOVE:
            if (dd->is_open) {
                VenomF32 my = (VenomF32)event->mouse.y;
                VenomF32 list_y = dd->height + 2;
                
                VenomI32 new_hover = (VenomI32)((my - list_y + dd->scroll_offset) / dd->item_height);
                if (new_hover < 0 || (VenomU32)new_hover >= dd->item_count) {
                    new_hover = -1;
                }
                
                if (new_hover != dd->hover_index) {
                    dd->hover_index = new_hover;
                    widget->needs_redraw = VENOM_TRUE;
                }
            }
            break;
            
        case VENOM_EVENT_KEY_DOWN:
            switch (event->key.key) {
                case VENOM_KEY_RETURN:
                case VENOM_KEY_SPACE:
                    if (!dd->is_open) {
                        dd->is_open = VENOM_TRUE;
                        dd->hover_index = dd->selected_index >= 0 ? dd->selected_index : 0;
                    } else if (dd->hover_index >= 0) {
                        dd->selected_index = dd->hover_index;
                        dd->is_open = VENOM_FALSE;
                        if (dd->on_change && (VenomU32)dd->selected_index < dd->item_count) {
                            dd->on_change(dd, dd->selected_index,
                                          dd->items[dd->selected_index].value,
                                          dd->callback_data);
                        }
                    }
                    widget->needs_redraw = VENOM_TRUE;
                    return VENOM_TRUE;
                    
                case VENOM_KEY_ESCAPE:
                    if (dd->is_open) {
                        dd->is_open = VENOM_FALSE;
                        widget->needs_redraw = VENOM_TRUE;
                        return VENOM_TRUE;
                    }
                    break;
                    
                case VENOM_KEY_UP:
                    if (dd->is_open && dd->hover_index > 0) {
                        dd->hover_index--;
                        widget->needs_redraw = VENOM_TRUE;
                    }
                    return VENOM_TRUE;
                    
                case VENOM_KEY_DOWN:
                    if (dd->is_open && dd->hover_index < (VenomI32)dd->item_count - 1) {
                        dd->hover_index++;
                        widget->needs_redraw = VENOM_TRUE;
                    } else if (!dd->is_open) {
                        dd->is_open = VENOM_TRUE;
                        dd->hover_index = 0;
                        widget->needs_redraw = VENOM_TRUE;
                    }
                    return VENOM_TRUE;
                    
                default:
                    break;
            }
            break;
            
        case VENOM_EVENT_MOUSE_SCROLL:
            if (dd->is_open) {
                dd->scroll_offset -= event->scroll.y * dd->item_height;
                if (dd->scroll_offset < 0) dd->scroll_offset = 0;
                
                VenomF32 max_scroll = dd->item_count * dd->item_height - dd->max_dropdown_height;
                if (max_scroll < 0) max_scroll = 0;
                if (dd->scroll_offset > max_scroll) dd->scroll_offset = max_scroll;
                
                widget->needs_redraw = VENOM_TRUE;
                return VENOM_TRUE;
            }
            break;
            
        default:
            break;
    }
    
    return VENOM_FALSE;
}

const VenomWidgetClass venom_dropdown_class = {
    .class_name = "VenomDropdown",
    .instance_size = sizeof(VenomDropdown),
    .parent_class = &venom_widget_class,
    .init = dropdown_init,
    .destroy = dropdown_destroy,
    .measure = dropdown_measure,
    .layout = NULL,
    .draw = dropdown_draw,
    .on_event = dropdown_on_event,
    .on_state_changed = NULL,
};

VenomResultPtr venom_dropdown_create(void) {
    return venom_widget_create(&venom_dropdown_class);
}

VenomResult venom_dropdown_add_item(VenomDropdown* dd, const char* label, const char* value) {
    VENOM_ENSURE_NOT_NULL(dd);
    
    if (dd->item_count >= dd->item_capacity) {
        VenomU32 new_cap = dd->item_capacity == 0 ? INITIAL_CAPACITY : dd->item_capacity * 2;
        VenomDropdownItem* new_items = (VenomDropdownItem*)venom_alloc(new_cap * sizeof(VenomDropdownItem));
        if (!new_items) return VENOM_ERR_UNIT(VENOM_ERROR_OUT_OF_MEMORY);
        
        if (dd->items) {
            memcpy(new_items, dd->items, dd->item_count * sizeof(VenomDropdownItem));
            venom_free(dd->items, dd->item_capacity * sizeof(VenomDropdownItem));
        }
        
        dd->items = new_items;
        dd->item_capacity = new_cap;
    }
    
    VenomDropdownItem* item = &dd->items[dd->item_count];
    memset(item, 0, sizeof(VenomDropdownItem));
    
    if (label) {
        VenomSize len = strlen(label) + 1;
        item->label = (char*)venom_alloc(len);
        if (item->label) memcpy(item->label, label, len);
    }
    
    if (value) {
        VenomSize len = strlen(value) + 1;
        item->value = (char*)venom_alloc(len);
        if (item->value) memcpy(item->value, value, len);
    }
    
    item->enabled = VENOM_TRUE;
    dd->item_count++;
    
    return VENOM_OK_UNIT();
}

void venom_dropdown_clear(VenomDropdown* dd) {
    if (!dd) return;
    
    for (VenomU32 i = 0; i < dd->item_count; i++) {
        if (dd->items[i].label) {
            venom_free(dd->items[i].label, strlen(dd->items[i].label) + 1);
        }
        if (dd->items[i].value) {
            venom_free(dd->items[i].value, strlen(dd->items[i].value) + 1);
        }
    }
    
    dd->item_count = 0;
    dd->selected_index = -1;
}

void venom_dropdown_set_selected(VenomDropdown* dd, VenomI32 index) {
    if (dd) {
        dd->selected_index = index;
        venom_widget_invalidate((VenomWidget*)dd);
    }
}

VenomI32 venom_dropdown_get_selected(const VenomDropdown* dd) {
    return dd ? dd->selected_index : -1;
}

const char* venom_dropdown_get_selected_value(const VenomDropdown* dd) {
    if (!dd || dd->selected_index < 0 || (VenomU32)dd->selected_index >= dd->item_count) {
        return NULL;
    }
    return dd->items[dd->selected_index].value ? 
           dd->items[dd->selected_index].value : 
           dd->items[dd->selected_index].label;
}

const char* venom_dropdown_get_selected_label(const VenomDropdown* dd) {
    if (!dd || dd->selected_index < 0 || (VenomU32)dd->selected_index >= dd->item_count) {
        return NULL;
    }
    return dd->items[dd->selected_index].label;
}

void venom_dropdown_set_placeholder(VenomDropdown* dd, const char* placeholder) {
    if (!dd) return;
    
    if (dd->placeholder) {
        venom_free(dd->placeholder, strlen(dd->placeholder) + 1);
        dd->placeholder = NULL;
    }
    
    if (placeholder) {
        VenomSize len = strlen(placeholder) + 1;
        dd->placeholder = (char*)venom_alloc(len);
        if (dd->placeholder) memcpy(dd->placeholder, placeholder, len);
    }
}

void venom_dropdown_set_on_change(VenomDropdown* dd, VenomDropdownCallback callback, void* data) {
    if (dd) {
        dd->on_change = callback;
        dd->callback_data = data;
    }
}

VenomWidget* _venom_dropdown_build(const VenomDropdownConfig* config) {
    VenomResultPtr result = venom_dropdown_create();
    if (!result.ok) return NULL;
    
    VenomDropdown* dd = (VenomDropdown*)result.value;
    
    if (config->placeholder) venom_dropdown_set_placeholder(dd, config->placeholder);
    dd->selected_index = config->selected;
    dd->on_change = config->on_change;
    dd->callback_data = config->data;
    
    return (VenomWidget*)dd;
}
