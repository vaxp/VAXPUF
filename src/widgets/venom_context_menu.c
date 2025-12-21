/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_context_menu.c - Context menu implementation
 */

#include "venom/widgets/venom_context_menu.h"
#include "venom/core/venom_memory.h"
#include <string.h>

#define DEFAULT_ITEM_HEIGHT 32.0f
#define DEFAULT_PADDING 8.0f
#define DEFAULT_MIN_WIDTH 150.0f
#define SEPARATOR_HEIGHT 9.0f

static void context_menu_init(VenomWidget* widget) {
    VenomContextMenu* menu = (VenomContextMenu*)widget;
    
    menu->items = NULL;
    menu->item_count = 0;
    
    menu->is_open = VENOM_FALSE;
    menu->popup_x = 0;
    menu->popup_y = 0;
    
    menu->hover_index = -1;
    menu->open_submenu = NULL;
    
    menu->background_color = (VenomColor){ 255, 255, 255, 245 };
    menu->hover_color = (VenomColor){ 63, 81, 181, 40 };
    menu->text_color = (VenomColor){ 33, 33, 33, 255 };
    menu->disabled_color = (VenomColor){ 150, 150, 150, 255 };
    menu->item_height = DEFAULT_ITEM_HEIGHT;
    menu->padding = DEFAULT_PADDING;
    menu->corner_radius = 4.0f;
    menu->min_width = DEFAULT_MIN_WIDTH;
}

static void menu_item_free(VenomMenuItem* item) {
    if (!item) return;
    
    if (item->label) {
        venom_free(item->label, strlen(item->label) + 1);
    }
    if (item->shortcut) {
        venom_free(item->shortcut, strlen(item->shortcut) + 1);
    }
    venom_free(item, sizeof(VenomMenuItem));
}

static void context_menu_destroy(VenomWidget* widget) {
    VenomContextMenu* menu = (VenomContextMenu*)widget;
    
    VenomMenuItem* item = menu->items;
    while (item) {
        VenomMenuItem* next = item->next;
        menu_item_free(item);
        item = next;
    }
    menu->items = NULL;
    
    venom_widget_class.destroy(widget);
}

static void context_menu_measure(VenomWidget* widget, VenomF32 available_width, VenomF32 available_height,
                                  VenomF32* out_width, VenomF32* out_height) {
    VenomContextMenu* menu = (VenomContextMenu*)widget;
    (void)available_width; (void)available_height;
    
    VenomF32 max_width = menu->min_width;
    VenomF32 total_height = menu->padding * 2;
    
    VenomMenuItem* item = menu->items;
    while (item) {
        if (item->is_separator) {
            total_height += SEPARATOR_HEIGHT;
        } else {
            total_height += menu->item_height;
            
            VenomF32 item_width = menu->padding * 2;
            if (item->label) {
                item_width += (VenomF32)strlen(item->label) * 8;
            }
            if (item->shortcut) {
                item_width += 50 + (VenomF32)strlen(item->shortcut) * 7;
            }
            if (item_width > max_width) max_width = item_width;
        }
        item = item->next;
    }
    
    *out_width = max_width;
    *out_height = total_height;
}

static void context_menu_draw(VenomWidget* widget, VenomCanvas* canvas) {
    VenomContextMenu* menu = (VenomContextMenu*)widget;
    
    if (!menu->is_open) return;
    
    VenomF32 out_w, out_h;
    context_menu_measure(widget, 0, 0, &out_w, &out_h);
    
    /* Draw shadow */
    VenomRectF shadow = { menu->popup_x + 3, menu->popup_y + 3, out_w, out_h };
    VenomPaint shadow_paint = venom_paint_fill((VenomColor){ 0, 0, 0, 40 });
    venom_canvas_draw_rounded_rect(canvas, shadow, menu->corner_radius, &shadow_paint);
    
    /* Draw background */
    VenomRectF bg = { menu->popup_x, menu->popup_y, out_w, out_h };
    VenomPaint bg_paint = venom_paint_fill(menu->background_color);
    venom_canvas_draw_rounded_rect(canvas, bg, menu->corner_radius, &bg_paint);
    
    /* Draw items */
    VenomF32 y = menu->popup_y + menu->padding;
    VenomI32 index = 0;
    VenomMenuItem* item = menu->items;
    
    while (item) {
        if (item->is_separator) {
            /* Draw separator line */
            VenomF32 sep_y = y + SEPARATOR_HEIGHT / 2;
            VenomPaint sep_paint = venom_paint_fill((VenomColor){ 200, 200, 200, 255 });
            VenomRectF sep = { menu->popup_x + menu->padding, sep_y, out_w - menu->padding * 2, 1 };
            venom_canvas_draw_rect(canvas, sep, &sep_paint);
            y += SEPARATOR_HEIGHT;
        } else {
            /* Draw hover highlight */
            if (index == menu->hover_index && item->enabled) {
                VenomRectF hover = { menu->popup_x, y, out_w, menu->item_height };
                VenomPaint hover_paint = venom_paint_fill(menu->hover_color);
                venom_canvas_draw_rect(canvas, hover, &hover_paint);
            }
            
            /* Draw label */
            VenomColor text_color = item->enabled ? menu->text_color : menu->disabled_color;
            VenomPaint text_paint = venom_paint_fill(text_color);
            VenomF32 text_y = y + menu->item_height / 2 + 5;
            venom_canvas_draw_text(canvas, item->label ? item->label : "", 
                                   menu->popup_x + menu->padding, text_y, NULL, &text_paint);
            
            /* Draw shortcut */
            if (item->shortcut) {
                VenomPaint short_paint = venom_paint_fill(menu->disabled_color);
                VenomF32 short_x = menu->popup_x + out_w - menu->padding - 
                                   (VenomF32)strlen(item->shortcut) * 7;
                venom_canvas_draw_text(canvas, item->shortcut, short_x, text_y, NULL, &short_paint);
            }
            
            /* Draw submenu arrow */
            if (item->submenu) {
                VenomPaint arrow_paint = venom_paint_fill(menu->text_color);
                venom_canvas_draw_text(canvas, "▶", 
                                       menu->popup_x + out_w - menu->padding - 10, text_y, 
                                       NULL, &arrow_paint);
            }
            
            y += menu->item_height;
            index++;
        }
        item = item->next;
    }
    
    /* Draw border */
    VenomPaint border_paint = venom_paint_stroke((VenomColor){ 200, 200, 200, 255 }, 1.0f);
    venom_canvas_draw_rounded_rect(canvas, bg, menu->corner_radius, &border_paint);
}

static VenomBool context_menu_on_event(VenomWidget* widget, const VenomEvent* event) {
    VenomContextMenu* menu = (VenomContextMenu*)widget;
    
    if (!menu->is_open) return VENOM_FALSE;
    
    VenomF32 out_w, out_h;
    context_menu_measure(widget, 0, 0, &out_w, &out_h);
    
    switch (event->type) {
        case VENOM_EVENT_MOUSE_MOVE: {
            VenomF32 mx = (VenomF32)event->mouse.x;
            VenomF32 my = (VenomF32)event->mouse.y;
            
            /* Check if in menu bounds */
            if (mx >= menu->popup_x && mx <= menu->popup_x + out_w &&
                my >= menu->popup_y && my <= menu->popup_y + out_h) {
                
                VenomF32 rel_y = my - menu->popup_y - menu->padding;
                VenomI32 new_hover = -1;
                VenomF32 y = 0;
                VenomI32 index = 0;
                VenomMenuItem* item = menu->items;
                
                while (item) {
                    VenomF32 item_h = item->is_separator ? SEPARATOR_HEIGHT : menu->item_height;
                    if (rel_y >= y && rel_y < y + item_h && !item->is_separator) {
                        new_hover = index;
                        break;
                    }
                    y += item_h;
                    if (!item->is_separator) index++;
                    item = item->next;
                }
                
                if (new_hover != menu->hover_index) {
                    menu->hover_index = new_hover;
                    widget->needs_redraw = VENOM_TRUE;
                }
                return VENOM_TRUE;
            } else {
                if (menu->hover_index != -1) {
                    menu->hover_index = -1;
                    widget->needs_redraw = VENOM_TRUE;
                }
            }
            break;
        }
        
        case VENOM_EVENT_MOUSE_BUTTON_DOWN: {
            if (event->mouse.button == VENOM_MOUSE_BUTTON_LEFT) {
                if (menu->hover_index >= 0) {
                    /* Find and click item */
                    VenomI32 index = 0;
                    VenomMenuItem* item = menu->items;
                    while (item) {
                        if (!item->is_separator) {
                            if (index == menu->hover_index && item->enabled) {
                                if (item->on_click) {
                                    item->on_click(item, item->user_data);
                                }
                                venom_context_menu_hide(menu);
                                return VENOM_TRUE;
                            }
                            index++;
                        }
                        item = item->next;
                    }
                }
                /* Click outside - close menu */
                venom_context_menu_hide(menu);
                return VENOM_TRUE;
            }
            break;
        }
        
        case VENOM_EVENT_KEY_DOWN:
            if (event->key.key == VENOM_KEY_ESCAPE) {
                venom_context_menu_hide(menu);
                return VENOM_TRUE;
            }
            break;
            
        default:
            break;
    }
    
    return menu->is_open;
}

const VenomWidgetClass venom_context_menu_class = {
    .class_name = "VenomContextMenu",
    .instance_size = sizeof(VenomContextMenu),
    .parent_class = &venom_widget_class,
    .init = context_menu_init,
    .destroy = context_menu_destroy,
    .measure = context_menu_measure,
    .layout = NULL,
    .draw = context_menu_draw,
    .on_event = context_menu_on_event,
    .on_state_changed = NULL,
};

VenomResultPtr venom_context_menu_create(void) {
    return venom_widget_create(&venom_context_menu_class);
}

VenomResult venom_context_menu_add_item(VenomContextMenu* menu, const char* label,
                                         VenomMenuItemCallback callback, void* data) {
    VENOM_ENSURE_NOT_NULL(menu);
    
    VenomMenuItem* item = (VenomMenuItem*)venom_alloc(sizeof(VenomMenuItem));
    if (!item) return VENOM_ERR_UNIT(VENOM_ERROR_OUT_OF_MEMORY);
    
    memset(item, 0, sizeof(VenomMenuItem));
    
    if (label) {
        VenomSize len = strlen(label) + 1;
        item->label = (char*)venom_alloc(len);
        if (item->label) {
            memcpy(item->label, label, len);
        }
    }
    
    item->enabled = VENOM_TRUE;
    item->is_separator = VENOM_FALSE;
    item->on_click = callback;
    item->user_data = data;
    
    /* Add to end of list */
    if (!menu->items) {
        menu->items = item;
    } else {
        VenomMenuItem* last = menu->items;
        while (last->next) last = last->next;
        last->next = item;
    }
    menu->item_count++;
    
    return VENOM_OK_UNIT();
}

VenomResult venom_context_menu_add_separator(VenomContextMenu* menu) {
    VENOM_ENSURE_NOT_NULL(menu);
    
    VenomMenuItem* item = (VenomMenuItem*)venom_alloc(sizeof(VenomMenuItem));
    if (!item) return VENOM_ERR_UNIT(VENOM_ERROR_OUT_OF_MEMORY);
    
    memset(item, 0, sizeof(VenomMenuItem));
    item->is_separator = VENOM_TRUE;
    
    if (!menu->items) {
        menu->items = item;
    } else {
        VenomMenuItem* last = menu->items;
        while (last->next) last = last->next;
        last->next = item;
    }
    
    return VENOM_OK_UNIT();
}

VenomResult venom_context_menu_add_submenu(VenomContextMenu* menu, const char* label,
                                            VenomContextMenu* submenu) {
    VENOM_ENSURE_NOT_NULL(menu);
    
    VenomMenuItem* item = (VenomMenuItem*)venom_alloc(sizeof(VenomMenuItem));
    if (!item) return VENOM_ERR_UNIT(VENOM_ERROR_OUT_OF_MEMORY);
    
    memset(item, 0, sizeof(VenomMenuItem));
    
    if (label) {
        VenomSize len = strlen(label) + 1;
        item->label = (char*)venom_alloc(len);
        if (item->label) {
            memcpy(item->label, label, len);
        }
    }
    
    item->enabled = VENOM_TRUE;
    item->submenu = submenu;
    
    if (!menu->items) {
        menu->items = item;
    } else {
        VenomMenuItem* last = menu->items;
        while (last->next) last = last->next;
        last->next = item;
    }
    menu->item_count++;
    
    return VENOM_OK_UNIT();
}

void venom_context_menu_show(VenomContextMenu* menu, VenomF32 x, VenomF32 y) {
    if (menu) {
        menu->popup_x = x;
        menu->popup_y = y;
        menu->is_open = VENOM_TRUE;
        menu->hover_index = -1;
        venom_widget_invalidate((VenomWidget*)menu);
    }
}

void venom_context_menu_hide(VenomContextMenu* menu) {
    if (menu) {
        menu->is_open = VENOM_FALSE;
        venom_widget_invalidate((VenomWidget*)menu);
    }
}

void venom_context_menu_clear(VenomContextMenu* menu) {
    if (!menu) return;
    
    VenomMenuItem* item = menu->items;
    while (item) {
        VenomMenuItem* next = item->next;
        menu_item_free(item);
        item = next;
    }
    menu->items = NULL;
    menu->item_count = 0;
}
