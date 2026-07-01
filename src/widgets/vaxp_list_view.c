/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_list_view.c - ListView widget implementation
 */

#include "vaxp/widgets/vaxp_list_view.h"
#include "vaxp/core/vaxp_memory.h"
#include <string.h>

#define INITIAL_CAPACITY 16
#define DEFAULT_ITEM_HEIGHT 48.0f

static void list_view_init(VaxpWidget* widget) {
    VaxpListView* list = (VaxpListView*)widget;
    
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
    list->multi_select = VAXP_FALSE;
    list->on_select = NULL;
    list->select_data = NULL;
    
    list->selection_color = (VaxpColor){ 63, 81, 181, 50 };
    list->hover_color = (VaxpColor){ 0, 0, 0, 20 };
    list->item_padding = 12.0f;
    list->show_dividers = VAXP_TRUE;
    
    list->hover_index = -1;
    
    widget->focusable = VAXP_TRUE;
}

static void list_view_destroy(VaxpWidget* widget) {
    VaxpListView* list = (VaxpListView*)widget;
    
    /* Destroy cached widgets */
    if (list->item_widgets) {
        for (VaxpU32 i = 0; i < list->widget_count; i++) {
            if (list->item_widgets[i]) {
                vaxp_unref(list->item_widgets[i]);
            }
        }
        vaxp_free(list->item_widgets, list->widget_count * sizeof(VaxpWidget*));
        list->item_widgets = NULL;
    }
    
    if (list->items) {
        vaxp_free(list->items, list->item_capacity * sizeof(void*));
        list->items = NULL;
    }
    
    vaxp_widget_class.destroy(widget);
}

static void list_view_measure(VaxpWidget* widget, VaxpF32 available_width, VaxpF32 available_height,
                              VaxpF32* out_width, VaxpF32* out_height) {
    VaxpListView* list = (VaxpListView*)widget;
    
    *out_width = widget->layout.preferred_width > 0 ? 
                 widget->layout.preferred_width : available_width;
    
    list->content_height = list->item_count * list->item_height;
    
    *out_height = widget->layout.preferred_height > 0 ? 
                  widget->layout.preferred_height : 
                  (list->content_height < available_height ? list->content_height : available_height);
}

static void list_view_layout(VaxpWidget* widget, VaxpRectF bounds) {
    widget->bounds = bounds;
}

static void list_view_draw(VaxpWidget* widget, VaxpCanvas* canvas) {
    VaxpListView* list = (VaxpListView*)widget;
    
    VaxpF32 w = widget->bounds.width;
    VaxpF32 h = widget->bounds.height;
    
    /* Clip to bounds */
    vaxp_canvas_save(canvas);
    VaxpRectF clip = { 0, 0, w, h };
    vaxp_canvas_clip_rect(canvas, clip);
    
    /* Calculate visible range */
    VaxpU32 first_visible = (VaxpU32)(list->scroll_offset / list->item_height);
    VaxpU32 visible_count = (VaxpU32)(h / list->item_height) + 2;
    VaxpU32 last_visible = first_visible + visible_count;
    if (last_visible > list->item_count) last_visible = list->item_count;
    
    /* Draw visible items */
    for (VaxpU32 i = first_visible; i < last_visible; i++) {
        VaxpF32 y = i * list->item_height - list->scroll_offset;
        VaxpRectF item_rect = { 0, y, w, list->item_height };
        
        /* Draw hover highlight */
        if ((VaxpI32)i == list->hover_index) {
            VaxpPaint hover_paint = vaxp_paint_fill(list->hover_color);
            vaxp_canvas_draw_rect(canvas, item_rect, &hover_paint);
        }
        
        /* Draw selection highlight */
        if ((VaxpI32)i == list->selected_index) {
            VaxpPaint sel_paint = vaxp_paint_fill(list->selection_color);
            vaxp_canvas_draw_rect(canvas, item_rect, &sel_paint);
        }
        
        /* Build and draw item widget */
        if (list->item_builder && i < list->item_count) {
            VaxpWidget* item_widget = list->item_builder(i, list->items[i], list->builder_data);
            if (item_widget) {
                VaxpRectF content_rect = {
                    list->item_padding, y + list->item_padding / 2,
                    w - list->item_padding * 2,
                    list->item_height - list->item_padding
                };
                vaxp_widget_layout(item_widget, content_rect);
                
                vaxp_canvas_save(canvas);
                vaxp_canvas_translate(canvas, content_rect.x, content_rect.y);
                vaxp_widget_draw(item_widget, canvas);
                vaxp_canvas_restore(canvas);
                
                /* Note: In production, we'd cache these widgets */
                vaxp_unref(item_widget);
            }
        }
        
        /* Draw divider */
        if (list->show_dividers && i < list->item_count - 1) {
            VaxpPaint div_paint = vaxp_paint_fill((VaxpColor){ 224, 224, 224, 255 });
            VaxpRectF div_rect = { list->item_padding, y + list->item_height - 1, 
                                    w - list->item_padding * 2, 1 };
            vaxp_canvas_draw_rect(canvas, div_rect, &div_paint);
        }
    }
    
    /* Draw scrollbar if needed */
    if (list->content_height > h) {
        VaxpF32 sb_width = 6.0f;
        VaxpF32 ratio = h / list->content_height;
        VaxpF32 thumb_h = h * ratio;
        if (thumb_h < 30) thumb_h = 30;
        VaxpF32 thumb_y = (list->scroll_offset / list->content_height) * h;
        
        VaxpPaint sb_paint = vaxp_paint_fill((VaxpColor){ 150, 150, 150, 150 });
        VaxpRectF sb_rect = { w - sb_width - 2, thumb_y, sb_width, thumb_h };
        vaxp_canvas_draw_rounded_rect(canvas, sb_rect, sb_width / 2, &sb_paint);
    }
    
    vaxp_canvas_restore(canvas);
}

static VaxpBool list_view_on_event(VaxpWidget* widget, const VaxpEvent* event) {
    VaxpListView* list = (VaxpListView*)widget;
    
    switch (event->type) {
        case VAXP_EVENT_MOUSE_MOVE: {
            VaxpF32 y = (VaxpF32)event->mouse.y + list->scroll_offset;
            VaxpI32 new_hover = (VaxpI32)(y / list->item_height);
            if (new_hover < 0 || (VaxpU32)new_hover >= list->item_count) {
                new_hover = -1;
            }
            if (new_hover != list->hover_index) {
                list->hover_index = new_hover;
                widget->needs_redraw = VAXP_TRUE;
            }
            return VAXP_TRUE;
        }
        
        case VAXP_EVENT_MOUSE_LEAVE:
            if (list->hover_index != -1) {
                list->hover_index = -1;
                widget->needs_redraw = VAXP_TRUE;
            }
            return VAXP_TRUE;
            
        case VAXP_EVENT_MOUSE_BUTTON_DOWN:
            if (event->mouse.button == VAXP_MOUSE_BUTTON_LEFT && list->hover_index >= 0) {
                list->selected_index = list->hover_index;
                widget->needs_redraw = VAXP_TRUE;
                
                if (list->on_select) {
                    list->on_select(list, (VaxpU32)list->selected_index, list->select_data);
                }
                return VAXP_TRUE;
            }
            break;
            
        case VAXP_EVENT_MOUSE_SCROLL: {
            list->scroll_offset -= event->scroll.y * list->item_height / 3;
            
            if (list->scroll_offset < 0) list->scroll_offset = 0;
            VaxpF32 max_scroll = list->content_height - widget->bounds.height;
            if (max_scroll < 0) max_scroll = 0;
            if (list->scroll_offset > max_scroll) list->scroll_offset = max_scroll;
            
            widget->needs_redraw = VAXP_TRUE;
            return VAXP_TRUE;
        }
        
        case VAXP_EVENT_KEY_DOWN:
            switch (event->key.key) {
                case VAXP_KEY_UP:
                    if (list->selected_index > 0) {
                        list->selected_index--;
                        widget->needs_redraw = VAXP_TRUE;
                        if (list->on_select) {
                            list->on_select(list, (VaxpU32)list->selected_index, list->select_data);
                        }
                    }
                    return VAXP_TRUE;
                    
                case VAXP_KEY_DOWN:
                    if (list->selected_index < (VaxpI32)list->item_count - 1) {
                        list->selected_index++;
                        widget->needs_redraw = VAXP_TRUE;
                        if (list->on_select) {
                            list->on_select(list, (VaxpU32)list->selected_index, list->select_data);
                        }
                    }
                    return VAXP_TRUE;
                    
                default:
                    break;
            }
            break;
            
        default:
            break;
    }
    
    return VAXP_FALSE;
}

const VaxpWidgetClass vaxp_list_view_class = {
    .class_name = "VaxpListView",
    .instance_size = sizeof(VaxpListView),
    .parent_class = &vaxp_widget_class,
    .init = list_view_init,
    .destroy = list_view_destroy,
    .measure = list_view_measure,
    .layout = list_view_layout,
    .draw = list_view_draw,
    .on_event = list_view_on_event,
    .on_state_changed = NULL,
};

VaxpResultPtr vaxp_list_view_create(void) {
    return vaxp_widget_create(&vaxp_list_view_class);
}

void vaxp_list_view_set_builder(VaxpListView* list, VaxpListItemBuilder builder, void* data) {
    if (list) {
        list->item_builder = builder;
        list->builder_data = data;
    }
}

VaxpResult vaxp_list_view_add_item(VaxpListView* list, void* item_data) {
    VAXP_ENSURE_NOT_NULL(list);
    
    if (list->item_count >= list->item_capacity) {
        VaxpU32 new_cap = list->item_capacity == 0 ? INITIAL_CAPACITY : list->item_capacity * 2;
        void** new_items = (void**)vaxp_alloc(new_cap * sizeof(void*));
        if (!new_items) return VAXP_ERR_UNIT(VAXP_ERROR_OUT_OF_MEMORY);
        
        if (list->items) {
            memcpy(new_items, list->items, list->item_count * sizeof(void*));
            vaxp_free(list->items, list->item_capacity * sizeof(void*));
        }
        
        list->items = new_items;
        list->item_capacity = new_cap;
    }
    
    list->items[list->item_count++] = item_data;
    vaxp_widget_invalidate((VaxpWidget*)list);
    return VAXP_OK_UNIT();
}

VaxpResult vaxp_list_view_remove_item(VaxpListView* list, VaxpU32 index) {
    VAXP_ENSURE_NOT_NULL(list);
    if (index >= list->item_count) return VAXP_ERR_UNIT(VAXP_ERROR_OUT_OF_BOUNDS);
    
    memmove(&list->items[index], &list->items[index + 1], 
            (list->item_count - index - 1) * sizeof(void*));
    list->item_count--;
    
    if (list->selected_index == (VaxpI32)index) {
        list->selected_index = -1;
    } else if (list->selected_index > (VaxpI32)index) {
        list->selected_index--;
    }
    
    vaxp_widget_invalidate((VaxpWidget*)list);
    return VAXP_OK_UNIT();
}

void vaxp_list_view_clear(VaxpListView* list) {
    if (list) {
        list->item_count = 0;
        list->selected_index = -1;
        list->scroll_offset = 0;
        vaxp_widget_invalidate((VaxpWidget*)list);
    }
}

VaxpU32 vaxp_list_view_count(const VaxpListView* list) {
    return list ? list->item_count : 0;
}

void vaxp_list_view_set_selected(VaxpListView* list, VaxpI32 index) {
    if (list) {
        list->selected_index = index;
        vaxp_widget_invalidate((VaxpWidget*)list);
    }
}

VaxpI32 vaxp_list_view_get_selected(const VaxpListView* list) {
    return list ? list->selected_index : -1;
}

void vaxp_list_view_set_item_height(VaxpListView* list, VaxpF32 height) {
    if (list && height > 0) {
        list->item_height = height;
        vaxp_widget_invalidate((VaxpWidget*)list);
    }
}

void vaxp_list_view_scroll_to(VaxpListView* list, VaxpU32 index) {
    if (!list || index >= list->item_count) return;
    
    list->scroll_offset = index * list->item_height;
    vaxp_widget_invalidate((VaxpWidget*)list);
}

void vaxp_list_view_set_on_select(VaxpListView* list, VaxpListSelectionCallback callback, void* data) {
    if (list) {
        list->on_select = callback;
        list->select_data = data;
    }
}

VaxpWidget* _vaxp_list_view_build(const VaxpListViewConfig* config) {
    VaxpResultPtr result = vaxp_list_view_create();
    if (!result.ok) return NULL;
    
    VaxpListView* list = (VaxpListView*)result.value;
    
    list->item_builder = config->item_builder;
    list->builder_data = config->builder_data;
    if (config->item_height > 0) list->item_height = config->item_height;
    list->on_select = config->on_select;
    list->select_data = config->select_data;
    
    return (VaxpWidget*)list;
}
