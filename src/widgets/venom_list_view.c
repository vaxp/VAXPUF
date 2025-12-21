/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_list_view.c - ListView widget implementation
 */

#include "venom/widgets/venom_list_view.h"
#include "venom/core/venom_memory.h"
#include <string.h>

#define INITIAL_CAPACITY 16
#define DEFAULT_ITEM_HEIGHT 48.0f

static void list_view_init(VenomWidget* widget) {
    VenomListView* list = (VenomListView*)widget;
    
    list->items = NULL;
    list->item_count = 0;
    list->item_capacity = 0;
    
    list->item_builder = NULL;
    list->builder_data = NULL;
    
    list->item_widgets = NULL;
    list->widget_count = 0;
    
    list->scroll_offset = 0;
    list->item_height = DEFAULT_ITEM_HEIGHT;
    list->content_height = 0;
    
    list->selected_index = -1;
    list->multi_select = VENOM_FALSE;
    list->on_select = NULL;
    list->select_data = NULL;
    
    list->selection_color = (VenomColor){ 63, 81, 181, 50 };
    list->hover_color = (VenomColor){ 0, 0, 0, 20 };
    list->item_padding = 12.0f;
    list->show_dividers = VENOM_TRUE;
    
    list->hover_index = -1;
    
    widget->focusable = VENOM_TRUE;
}

static void list_view_destroy(VenomWidget* widget) {
    VenomListView* list = (VenomListView*)widget;
    
    /* Destroy cached widgets */
    if (list->item_widgets) {
        for (VenomU32 i = 0; i < list->widget_count; i++) {
            if (list->item_widgets[i]) {
                venom_unref(list->item_widgets[i]);
            }
        }
        venom_free(list->item_widgets, list->widget_count * sizeof(VenomWidget*));
        list->item_widgets = NULL;
    }
    
    if (list->items) {
        venom_free(list->items, list->item_capacity * sizeof(void*));
        list->items = NULL;
    }
    
    venom_widget_class.destroy(widget);
}

static void list_view_measure(VenomWidget* widget, VenomF32 available_width, VenomF32 available_height,
                              VenomF32* out_width, VenomF32* out_height) {
    VenomListView* list = (VenomListView*)widget;
    
    *out_width = widget->layout.preferred_width > 0 ? 
                 widget->layout.preferred_width : available_width;
    
    list->content_height = list->item_count * list->item_height;
    
    *out_height = widget->layout.preferred_height > 0 ? 
                  widget->layout.preferred_height : 
                  (list->content_height < available_height ? list->content_height : available_height);
}

static void list_view_layout(VenomWidget* widget, VenomRectF bounds) {
    widget->bounds = bounds;
}

static void list_view_draw(VenomWidget* widget, VenomCanvas* canvas) {
    VenomListView* list = (VenomListView*)widget;
    
    VenomF32 w = widget->bounds.width;
    VenomF32 h = widget->bounds.height;
    
    /* Clip to bounds */
    venom_canvas_save(canvas);
    VenomRectF clip = { 0, 0, w, h };
    venom_canvas_clip_rect(canvas, clip);
    
    /* Calculate visible range */
    VenomU32 first_visible = (VenomU32)(list->scroll_offset / list->item_height);
    VenomU32 visible_count = (VenomU32)(h / list->item_height) + 2;
    VenomU32 last_visible = first_visible + visible_count;
    if (last_visible > list->item_count) last_visible = list->item_count;
    
    /* Draw visible items */
    for (VenomU32 i = first_visible; i < last_visible; i++) {
        VenomF32 y = i * list->item_height - list->scroll_offset;
        VenomRectF item_rect = { 0, y, w, list->item_height };
        
        /* Draw hover highlight */
        if ((VenomI32)i == list->hover_index) {
            VenomPaint hover_paint = venom_paint_fill(list->hover_color);
            venom_canvas_draw_rect(canvas, item_rect, &hover_paint);
        }
        
        /* Draw selection highlight */
        if ((VenomI32)i == list->selected_index) {
            VenomPaint sel_paint = venom_paint_fill(list->selection_color);
            venom_canvas_draw_rect(canvas, item_rect, &sel_paint);
        }
        
        /* Build and draw item widget */
        if (list->item_builder && i < list->item_count) {
            VenomWidget* item_widget = list->item_builder(i, list->items[i], list->builder_data);
            if (item_widget) {
                VenomRectF content_rect = {
                    list->item_padding, y + list->item_padding / 2,
                    w - list->item_padding * 2,
                    list->item_height - list->item_padding
                };
                venom_widget_layout(item_widget, content_rect);
                
                venom_canvas_save(canvas);
                venom_canvas_translate(canvas, content_rect.x, content_rect.y);
                venom_widget_draw(item_widget, canvas);
                venom_canvas_restore(canvas);
                
                /* Note: In production, we'd cache these widgets */
                venom_unref(item_widget);
            }
        }
        
        /* Draw divider */
        if (list->show_dividers && i < list->item_count - 1) {
            VenomPaint div_paint = venom_paint_fill((VenomColor){ 224, 224, 224, 255 });
            VenomRectF div_rect = { list->item_padding, y + list->item_height - 1, 
                                    w - list->item_padding * 2, 1 };
            venom_canvas_draw_rect(canvas, div_rect, &div_paint);
        }
    }
    
    /* Draw scrollbar if needed */
    if (list->content_height > h) {
        VenomF32 sb_width = 6.0f;
        VenomF32 ratio = h / list->content_height;
        VenomF32 thumb_h = h * ratio;
        if (thumb_h < 30) thumb_h = 30;
        VenomF32 thumb_y = (list->scroll_offset / list->content_height) * h;
        
        VenomPaint sb_paint = venom_paint_fill((VenomColor){ 150, 150, 150, 150 });
        VenomRectF sb_rect = { w - sb_width - 2, thumb_y, sb_width, thumb_h };
        venom_canvas_draw_rounded_rect(canvas, sb_rect, sb_width / 2, &sb_paint);
    }
    
    venom_canvas_restore(canvas);
}

static VenomBool list_view_on_event(VenomWidget* widget, const VenomEvent* event) {
    VenomListView* list = (VenomListView*)widget;
    
    switch (event->type) {
        case VENOM_EVENT_MOUSE_MOVE: {
            VenomF32 y = (VenomF32)event->mouse.y + list->scroll_offset;
            VenomI32 new_hover = (VenomI32)(y / list->item_height);
            if (new_hover < 0 || (VenomU32)new_hover >= list->item_count) {
                new_hover = -1;
            }
            if (new_hover != list->hover_index) {
                list->hover_index = new_hover;
                widget->needs_redraw = VENOM_TRUE;
            }
            return VENOM_TRUE;
        }
        
        case VENOM_EVENT_MOUSE_LEAVE:
            if (list->hover_index != -1) {
                list->hover_index = -1;
                widget->needs_redraw = VENOM_TRUE;
            }
            return VENOM_TRUE;
            
        case VENOM_EVENT_MOUSE_BUTTON_DOWN:
            if (event->mouse.button == VENOM_MOUSE_BUTTON_LEFT && list->hover_index >= 0) {
                list->selected_index = list->hover_index;
                widget->needs_redraw = VENOM_TRUE;
                
                if (list->on_select) {
                    list->on_select(list, (VenomU32)list->selected_index, list->select_data);
                }
                return VENOM_TRUE;
            }
            break;
            
        case VENOM_EVENT_MOUSE_SCROLL: {
            list->scroll_offset -= event->scroll.y * list->item_height / 3;
            
            if (list->scroll_offset < 0) list->scroll_offset = 0;
            VenomF32 max_scroll = list->content_height - widget->bounds.height;
            if (max_scroll < 0) max_scroll = 0;
            if (list->scroll_offset > max_scroll) list->scroll_offset = max_scroll;
            
            widget->needs_redraw = VENOM_TRUE;
            return VENOM_TRUE;
        }
        
        case VENOM_EVENT_KEY_DOWN:
            switch (event->key.key) {
                case VENOM_KEY_UP:
                    if (list->selected_index > 0) {
                        list->selected_index--;
                        widget->needs_redraw = VENOM_TRUE;
                        if (list->on_select) {
                            list->on_select(list, (VenomU32)list->selected_index, list->select_data);
                        }
                    }
                    return VENOM_TRUE;
                    
                case VENOM_KEY_DOWN:
                    if (list->selected_index < (VenomI32)list->item_count - 1) {
                        list->selected_index++;
                        widget->needs_redraw = VENOM_TRUE;
                        if (list->on_select) {
                            list->on_select(list, (VenomU32)list->selected_index, list->select_data);
                        }
                    }
                    return VENOM_TRUE;
                    
                default:
                    break;
            }
            break;
            
        default:
            break;
    }
    
    return VENOM_FALSE;
}

const VenomWidgetClass venom_list_view_class = {
    .class_name = "VenomListView",
    .instance_size = sizeof(VenomListView),
    .parent_class = &venom_widget_class,
    .init = list_view_init,
    .destroy = list_view_destroy,
    .measure = list_view_measure,
    .layout = list_view_layout,
    .draw = list_view_draw,
    .on_event = list_view_on_event,
    .on_state_changed = NULL,
};

VenomResultPtr venom_list_view_create(void) {
    return venom_widget_create(&venom_list_view_class);
}

void venom_list_view_set_builder(VenomListView* list, VenomListItemBuilder builder, void* data) {
    if (list) {
        list->item_builder = builder;
        list->builder_data = data;
    }
}

VenomResult venom_list_view_add_item(VenomListView* list, void* item_data) {
    VENOM_ENSURE_NOT_NULL(list);
    
    if (list->item_count >= list->item_capacity) {
        VenomU32 new_cap = list->item_capacity == 0 ? INITIAL_CAPACITY : list->item_capacity * 2;
        void** new_items = (void**)venom_alloc(new_cap * sizeof(void*));
        if (!new_items) return VENOM_ERR_UNIT(VENOM_ERROR_OUT_OF_MEMORY);
        
        if (list->items) {
            memcpy(new_items, list->items, list->item_count * sizeof(void*));
            venom_free(list->items, list->item_capacity * sizeof(void*));
        }
        
        list->items = new_items;
        list->item_capacity = new_cap;
    }
    
    list->items[list->item_count++] = item_data;
    venom_widget_invalidate((VenomWidget*)list);
    return VENOM_OK_UNIT();
}

VenomResult venom_list_view_remove_item(VenomListView* list, VenomU32 index) {
    VENOM_ENSURE_NOT_NULL(list);
    if (index >= list->item_count) return VENOM_ERR_UNIT(VENOM_ERROR_OUT_OF_BOUNDS);
    
    memmove(&list->items[index], &list->items[index + 1], 
            (list->item_count - index - 1) * sizeof(void*));
    list->item_count--;
    
    if (list->selected_index == (VenomI32)index) {
        list->selected_index = -1;
    } else if (list->selected_index > (VenomI32)index) {
        list->selected_index--;
    }
    
    venom_widget_invalidate((VenomWidget*)list);
    return VENOM_OK_UNIT();
}

void venom_list_view_clear(VenomListView* list) {
    if (list) {
        list->item_count = 0;
        list->selected_index = -1;
        list->scroll_offset = 0;
        venom_widget_invalidate((VenomWidget*)list);
    }
}

VenomU32 venom_list_view_count(const VenomListView* list) {
    return list ? list->item_count : 0;
}

void venom_list_view_set_selected(VenomListView* list, VenomI32 index) {
    if (list) {
        list->selected_index = index;
        venom_widget_invalidate((VenomWidget*)list);
    }
}

VenomI32 venom_list_view_get_selected(const VenomListView* list) {
    return list ? list->selected_index : -1;
}

void venom_list_view_set_item_height(VenomListView* list, VenomF32 height) {
    if (list && height > 0) {
        list->item_height = height;
        venom_widget_invalidate((VenomWidget*)list);
    }
}

void venom_list_view_scroll_to(VenomListView* list, VenomU32 index) {
    if (!list || index >= list->item_count) return;
    
    list->scroll_offset = index * list->item_height;
    venom_widget_invalidate((VenomWidget*)list);
}

void venom_list_view_set_on_select(VenomListView* list, VenomListSelectionCallback callback, void* data) {
    if (list) {
        list->on_select = callback;
        list->select_data = data;
    }
}

VenomWidget* _venom_list_view_build(const VenomListViewConfig* config) {
    VenomResultPtr result = venom_list_view_create();
    if (!result.ok) return NULL;
    
    VenomListView* list = (VenomListView*)result.value;
    
    list->item_builder = config->item_builder;
    list->builder_data = config->builder_data;
    if (config->item_height > 0) list->item_height = config->item_height;
    list->on_select = config->on_select;
    list->select_data = config->select_data;
    
    return (VenomWidget*)list;
}
