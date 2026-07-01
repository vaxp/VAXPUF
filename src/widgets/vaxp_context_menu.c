/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_context_menu.c - Context menu implementation
 */

#include "vaxp/widgets/vaxp_context_menu.h"
#include "vaxp/core/vaxp_memory.h"
#include <string.h>

#define DEFAULT_ITEM_HEIGHT 32.0f
#define DEFAULT_PADDING 8.0f
#define DEFAULT_MIN_WIDTH 150.0f
#define SEPARATOR_HEIGHT 9.0f

static void context_menu_init(VaxpWidget* widget) {
    VaxpContextMenu* menu = (VaxpContextMenu*)widget;
    
    menu->items = NULL;
    menu->item_count = 0;
    
    menu->is_open = VAXP_FALSE;
    menu->popup_x = 0;
    menu->popup_y = 0;
    
    menu->hover_index = -1;
    menu->open_submenu = NULL;
    
    menu->background_color = (VaxpColor){ 255, 255, 255, 245 };
    menu->hover_color = (VaxpColor){ 63, 81, 181, 40 };
    menu->text_color = (VaxpColor){ 33, 33, 33, 255 };
    menu->disabled_color = (VaxpColor){ 150, 150, 150, 255 };
    menu->item_height = DEFAULT_ITEM_HEIGHT;
    menu->padding = DEFAULT_PADDING;
    menu->corner_radius = 4.0f;
    menu->min_width = DEFAULT_MIN_WIDTH;
}

static void menu_item_free(VaxpMenuItem* item) {
    if (!item) return;
    
    if (item->label) {
        vaxp_free(item->label, strlen(item->label) + 1);
    }
    if (item->shortcut) {
        vaxp_free(item->shortcut, strlen(item->shortcut) + 1);
    }
    vaxp_free(item, sizeof(VaxpMenuItem));
}

static void context_menu_destroy(VaxpWidget* widget) {
    VaxpContextMenu* menu = (VaxpContextMenu*)widget;
    
    VaxpMenuItem* item = menu->items;
    while (item) {
        VaxpMenuItem* next = item->next;
        menu_item_free(item);
        item = next;
    }
    menu->items = NULL;
    
    vaxp_widget_class.destroy(widget);
}

static void context_menu_measure(VaxpWidget* widget, VaxpF32 available_width, VaxpF32 available_height,
                                  VaxpF32* out_width, VaxpF32* out_height) {
    VaxpContextMenu* menu = (VaxpContextMenu*)widget;
    (void)available_width; (void)available_height;
    
    VaxpF32 max_width = menu->min_width;
    VaxpF32 total_height = menu->padding * 2;
    
    VaxpMenuItem* item = menu->items;
    while (item) {
        if (item->is_separator) {
            total_height += SEPARATOR_HEIGHT;
        } else {
            total_height += menu->item_height;
            
            VaxpF32 item_width = menu->padding * 2;
            if (item->label) {
                item_width += (VaxpF32)strlen(item->label) * 8;
            }
            if (item->shortcut) {
                item_width += 50 + (VaxpF32)strlen(item->shortcut) * 7;
            }
            if (item_width > max_width) max_width = item_width;
        }
        item = item->next;
    }
    
    *out_width = max_width;
    *out_height = total_height;
}

static void context_menu_draw(VaxpWidget* widget, VaxpCanvas* canvas) {
    VaxpContextMenu* menu = (VaxpContextMenu*)widget;
    
    if (!menu->is_open) return;
    
    VaxpF32 out_w, out_h;
    context_menu_measure(widget, 0, 0, &out_w, &out_h);
    
    /* Draw shadow */
    VaxpRectF shadow = { menu->popup_x + 3, menu->popup_y + 3, out_w, out_h };
    VaxpPaint shadow_paint = vaxp_paint_fill((VaxpColor){ 0, 0, 0, 40 });
    vaxp_canvas_draw_rounded_rect(canvas, shadow, menu->corner_radius, &shadow_paint);
    
    /* Draw background */
    VaxpRectF bg = { menu->popup_x, menu->popup_y, out_w, out_h };
    VaxpPaint bg_paint = vaxp_paint_fill(menu->background_color);
    vaxp_canvas_draw_rounded_rect(canvas, bg, menu->corner_radius, &bg_paint);
    
    /* Draw items */
    VaxpF32 y = menu->popup_y + menu->padding;
    VaxpI32 index = 0;
    VaxpMenuItem* item = menu->items;
    
    while (item) {
        if (item->is_separator) {
            /* Draw separator line */
            VaxpF32 sep_y = y + SEPARATOR_HEIGHT / 2;
            VaxpPaint sep_paint = vaxp_paint_fill((VaxpColor){ 200, 200, 200, 255 });
            VaxpRectF sep = { menu->popup_x + menu->padding, sep_y, out_w - menu->padding * 2, 1 };
            vaxp_canvas_draw_rect(canvas, sep, &sep_paint);
            y += SEPARATOR_HEIGHT;
        } else {
            /* Draw hover highlight */
            if (index == menu->hover_index && item->enabled) {
                VaxpRectF hover = { menu->popup_x, y, out_w, menu->item_height };
                VaxpPaint hover_paint = vaxp_paint_fill(menu->hover_color);
                vaxp_canvas_draw_rect(canvas, hover, &hover_paint);
            }
            
            /* Draw label */
            VaxpColor text_color = item->enabled ? menu->text_color : menu->disabled_color;
            VaxpPaint text_paint = vaxp_paint_fill(text_color);
            VaxpF32 text_y = y + menu->item_height / 2 + 5;
            vaxp_canvas_draw_text(canvas, item->label ? item->label : "", 
                                   menu->popup_x + menu->padding, text_y, NULL, &text_paint);
            
            /* Draw shortcut */
            if (item->shortcut) {
                VaxpPaint short_paint = vaxp_paint_fill(menu->disabled_color);
                VaxpF32 short_x = menu->popup_x + out_w - menu->padding - 
                                   (VaxpF32)strlen(item->shortcut) * 7;
                vaxp_canvas_draw_text(canvas, item->shortcut, short_x, text_y, NULL, &short_paint);
            }
            
            /* Draw submenu arrow */
            if (item->submenu) {
                VaxpPaint arrow_paint = vaxp_paint_fill(menu->text_color);
                vaxp_canvas_draw_text(canvas, "▶", 
                                       menu->popup_x + out_w - menu->padding - 10, text_y, 
                                       NULL, &arrow_paint);
            }
            
            y += menu->item_height;
            index++;
        }
        item = item->next;
    }
    
    /* Draw border - GL backend currently doesn't support stroke rects so this covers the text */
    /* VaxpPaint border_paint = vaxp_paint_stroke((VaxpColor){ 200, 200, 200, 255 }, 1.0f);
    vaxp_canvas_draw_rounded_rect(canvas, bg, menu->corner_radius, &border_paint); */
}

static VaxpBool context_menu_on_event(VaxpWidget* widget, const VaxpEvent* event) {
    VaxpContextMenu* menu = (VaxpContextMenu*)widget;
    
    if (!menu->is_open) return VAXP_FALSE;
    
    VaxpF32 out_w, out_h;
    context_menu_measure(widget, 0, 0, &out_w, &out_h);
    
    switch (event->type) {
        case VAXP_EVENT_MOUSE_MOVE: {
            VaxpF32 mx = (VaxpF32)event->mouse.x;
            VaxpF32 my = (VaxpF32)event->mouse.y;
            
            /* Check if in menu bounds */
            if (mx >= menu->popup_x && mx <= menu->popup_x + out_w &&
                my >= menu->popup_y && my <= menu->popup_y + out_h) {
                
                VaxpF32 rel_y = my - menu->popup_y - menu->padding;
                VaxpI32 new_hover = -1;
                VaxpF32 y = 0;
                VaxpI32 index = 0;
                VaxpMenuItem* item = menu->items;
                
                while (item) {
                    VaxpF32 item_h = item->is_separator ? SEPARATOR_HEIGHT : menu->item_height;
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
                    widget->needs_redraw = VAXP_TRUE;
                }
                return VAXP_TRUE;
            } else {
                if (menu->hover_index != -1) {
                    menu->hover_index = -1;
                    widget->needs_redraw = VAXP_TRUE;
                }
            }
            break;
        }
        
        case VAXP_EVENT_MOUSE_BUTTON_DOWN: {
            if (event->mouse.button == VAXP_MOUSE_BUTTON_LEFT) {
                if (menu->hover_index >= 0) {
                    /* Find and click item */
                    VaxpI32 index = 0;
                    VaxpMenuItem* item = menu->items;
                    while (item) {
                        if (!item->is_separator) {
                            if (index == menu->hover_index && item->enabled) {
                                if (item->on_click) {
                                    item->on_click(item, item->user_data);
                                }
                                vaxp_context_menu_hide(menu);
                                return VAXP_TRUE;
                            }
                            index++;
                        }
                        item = item->next;
                    }
                }
                /* Click outside - close menu */
                vaxp_context_menu_hide(menu);
                return VAXP_TRUE;
            }
            break;
        }
        
        case VAXP_EVENT_KEY_DOWN:
            if (event->key.key == VAXP_KEY_ESCAPE) {
                vaxp_context_menu_hide(menu);
                return VAXP_TRUE;
            }
            break;
            
        default:
            break;
    }
    
    return menu->is_open;
}

const VaxpWidgetClass vaxp_context_menu_class = {
    .class_name = "VaxpContextMenu",
    .instance_size = sizeof(VaxpContextMenu),
    .parent_class = &vaxp_widget_class,
    .init = context_menu_init,
    .destroy = context_menu_destroy,
    .measure = context_menu_measure,
    .layout = NULL,
    .draw = context_menu_draw,
    .on_event = context_menu_on_event,
    .on_state_changed = NULL,
};

VaxpResultPtr vaxp_context_menu_create(void) {
    return vaxp_widget_create(&vaxp_context_menu_class);
}

VaxpResult vaxp_context_menu_add_item(VaxpContextMenu* menu, const char* label,
                                         VaxpMenuItemCallback callback, void* data) {
    VAXP_ENSURE_NOT_NULL(menu);
    
    VaxpMenuItem* item = (VaxpMenuItem*)vaxp_alloc(sizeof(VaxpMenuItem));
    if (!item) return VAXP_ERR_UNIT(VAXP_ERROR_OUT_OF_MEMORY);
    
    memset(item, 0, sizeof(VaxpMenuItem));
    
    if (label) {
        VaxpSize len = strlen(label) + 1;
        item->label = (char*)vaxp_alloc(len);
        if (item->label) {
            memcpy(item->label, label, len);
        }
    }
    
    item->enabled = VAXP_TRUE;
    item->is_separator = VAXP_FALSE;
    item->on_click = callback;
    item->user_data = data;
    
    /* Add to end of list */
    if (!menu->items) {
        menu->items = item;
    } else {
        VaxpMenuItem* last = menu->items;
        while (last->next) last = last->next;
        last->next = item;
    }
    menu->item_count++;
    
    return VAXP_OK_UNIT();
}

VaxpResult vaxp_context_menu_add_separator(VaxpContextMenu* menu) {
    VAXP_ENSURE_NOT_NULL(menu);
    
    VaxpMenuItem* item = (VaxpMenuItem*)vaxp_alloc(sizeof(VaxpMenuItem));
    if (!item) return VAXP_ERR_UNIT(VAXP_ERROR_OUT_OF_MEMORY);
    
    memset(item, 0, sizeof(VaxpMenuItem));
    item->is_separator = VAXP_TRUE;
    
    if (!menu->items) {
        menu->items = item;
    } else {
        VaxpMenuItem* last = menu->items;
        while (last->next) last = last->next;
        last->next = item;
    }
    
    return VAXP_OK_UNIT();
}

VaxpResult vaxp_context_menu_add_submenu(VaxpContextMenu* menu, const char* label,
                                            VaxpContextMenu* submenu) {
    VAXP_ENSURE_NOT_NULL(menu);
    
    VaxpMenuItem* item = (VaxpMenuItem*)vaxp_alloc(sizeof(VaxpMenuItem));
    if (!item) return VAXP_ERR_UNIT(VAXP_ERROR_OUT_OF_MEMORY);
    
    memset(item, 0, sizeof(VaxpMenuItem));
    
    if (label) {
        VaxpSize len = strlen(label) + 1;
        item->label = (char*)vaxp_alloc(len);
        if (item->label) {
            memcpy(item->label, label, len);
        }
    }
    
    item->enabled = VAXP_TRUE;
    item->submenu = submenu;
    
    if (!menu->items) {
        menu->items = item;
    } else {
        VaxpMenuItem* last = menu->items;
        while (last->next) last = last->next;
        last->next = item;
    }
    menu->item_count++;
    
    return VAXP_OK_UNIT();
}

void vaxp_context_menu_show(VaxpContextMenu* menu, VaxpF32 x, VaxpF32 y) {
    if (menu) {
        menu->popup_x = x;
        menu->popup_y = y;
        menu->is_open = VAXP_TRUE;
        menu->hover_index = -1;
        vaxp_widget_invalidate((VaxpWidget*)menu);
    }
}

void vaxp_context_menu_hide(VaxpContextMenu* menu) {
    if (menu) {
        menu->is_open = VAXP_FALSE;
        vaxp_widget_invalidate((VaxpWidget*)menu);
    }
}

void vaxp_context_menu_clear(VaxpContextMenu* menu) {
    if (!menu) return;
    
    VaxpMenuItem* item = menu->items;
    while (item) {
        VaxpMenuItem* next = item->next;
        menu_item_free(item);
        item = next;
    }
    menu->items = NULL;
    menu->item_count = 0;
}
