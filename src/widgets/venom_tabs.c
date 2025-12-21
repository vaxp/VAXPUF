/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_tabs.c - TabBar and TabView implementation
 */

#include "venom/widgets/venom_tabs.h"
#include "venom/core/venom_memory.h"
#include <string.h>

#define DEFAULT_TAB_HEIGHT 48.0f
#define DEFAULT_TAB_PADDING 16.0f
#define INITIAL_CAPACITY 8

/* ============================================================================
 * TAB BAR
 * ============================================================================ */

static void tab_bar_init(VenomWidget* widget) {
    VenomTabBar* bar = (VenomTabBar*)widget;
    
    bar->tabs = NULL;
    bar->tab_count = 0;
    bar->tab_capacity = 0;
    
    bar->selected_index = 0;
    bar->style = VENOM_TAB_STYLE_UNDERLINE;
    bar->scrollable = VENOM_FALSE;
    bar->scroll_offset = 0;
    
    bar->tab_height = DEFAULT_TAB_HEIGHT;
    bar->tab_padding = DEFAULT_TAB_PADDING;
    bar->indicator_height = 3.0f;
    bar->background_color = (VenomColor){ 255, 255, 255, 255 };
    bar->tab_color = (VenomColor){ 97, 97, 97, 255 };
    bar->selected_color = (VenomColor){ 63, 81, 181, 255 };
    bar->indicator_color = (VenomColor){ 63, 81, 181, 255 };
    bar->disabled_color = (VenomColor){ 189, 189, 189, 255 };
    
    bar->hover_index = -1;
    bar->on_change = NULL;
    bar->callback_data = NULL;
    bar->tab_view = NULL;
    
    widget->focusable = VENOM_TRUE;
}

static void tab_bar_destroy(VenomWidget* widget) {
    VenomTabBar* bar = (VenomTabBar*)widget;
    
    for (VenomU32 i = 0; i < bar->tab_count; i++) {
        if (bar->tabs[i].label) {
            venom_free(bar->tabs[i].label, strlen(bar->tabs[i].label) + 1);
        }
        if (bar->tabs[i].icon) {
            venom_free(bar->tabs[i].icon, strlen(bar->tabs[i].icon) + 1);
        }
        /* Note: content is owned by TabView */
    }
    
    if (bar->tabs) {
        venom_free(bar->tabs, bar->tab_capacity * sizeof(VenomTab));
    }
    
    venom_widget_class.destroy(widget);
}

static VenomF32 calculate_tab_width(const VenomTabBar* bar, VenomU32 index) {
    if (index >= bar->tab_count) return 0;
    const char* label = bar->tabs[index].label;
    VenomF32 text_width = label ? (VenomF32)strlen(label) * 9 : 0;
    return text_width + bar->tab_padding * 2;
}

static void tab_bar_measure(VenomWidget* widget, VenomF32 available_width, VenomF32 available_height,
                            VenomF32* out_width, VenomF32* out_height) {
    VenomTabBar* bar = (VenomTabBar*)widget;
    (void)available_height;
    
    *out_width = available_width;
    *out_height = bar->tab_height;
}

static void tab_bar_draw(VenomWidget* widget, VenomCanvas* canvas) {
    VenomTabBar* bar = (VenomTabBar*)widget;
    
    VenomF32 w = widget->bounds.width;
    VenomF32 h = bar->tab_height;
    
    /* Draw background */
    VenomRectF bg = { 0, 0, w, h };
    VenomPaint bg_paint = venom_paint_fill(bar->background_color);
    venom_canvas_draw_rect(canvas, bg, &bg_paint);
    
    /* Draw bottom border for underline style */
    if (bar->style == VENOM_TAB_STYLE_UNDERLINE) {
        VenomPaint border_paint = venom_paint_fill((VenomColor){ 224, 224, 224, 255 });
        VenomRectF border = { 0, h - 1, w, 1 };
        venom_canvas_draw_rect(canvas, border, &border_paint);
    }
    
    /* Draw tabs */
    VenomF32 x = -bar->scroll_offset;
    
    for (VenomU32 i = 0; i < bar->tab_count; i++) {
        VenomF32 tab_w = calculate_tab_width(bar, i);
        VenomRectF tab_rect = { x, 0, tab_w, h };
        
        VenomBool is_selected = (i == bar->selected_index);
        VenomBool is_hover = ((VenomI32)i == bar->hover_index);
        VenomBool is_enabled = bar->tabs[i].enabled;
        
        /* Draw tab background based on style */
        if (bar->style == VENOM_TAB_STYLE_PILL && is_selected) {
            VenomPaint pill_paint = venom_paint_fill((VenomColor){ 63, 81, 181, 40 });
            VenomRectF pill = { x + 4, 8, tab_w - 8, h - 16 };
            venom_canvas_draw_rounded_rect(canvas, pill, (h - 16) / 2, &pill_paint);
        } else if (bar->style == VENOM_TAB_STYLE_BOXED && is_selected) {
            VenomPaint box_paint = venom_paint_fill((VenomColor){ 255, 255, 255, 255 });
            VenomRectF box = { x, 0, tab_w, h - 1 };
            venom_canvas_draw_rect(canvas, box, &box_paint);
            
            VenomPaint box_border = venom_paint_stroke((VenomColor){ 224, 224, 224, 255 }, 1.0f);
            venom_canvas_draw_rect(canvas, (VenomRectF){ x, 0, tab_w, h + 1 }, &box_border);
        }
        
        /* Hover highlight */
        if (is_hover && !is_selected && is_enabled) {
            VenomPaint hover_paint = venom_paint_fill((VenomColor){ 0, 0, 0, 15 });
            venom_canvas_draw_rect(canvas, tab_rect, &hover_paint);
        }
        
        /* Draw label */
        VenomColor label_color;
        if (!is_enabled) {
            label_color = bar->disabled_color;
        } else if (is_selected) {
            label_color = bar->selected_color;
        } else {
            label_color = bar->tab_color;
        }
        
        VenomPaint label_paint = venom_paint_fill(label_color);
        if (bar->tabs[i].label) {
            VenomF32 text_w = (VenomF32)strlen(bar->tabs[i].label) * 9;
            VenomF32 tx = x + (tab_w - text_w) / 2;
            VenomF32 ty = h / 2 + 5;
            venom_canvas_draw_text(canvas, bar->tabs[i].label, tx, ty, NULL, &label_paint);
        }
        
        /* Draw indicator for underline style */
        if (bar->style == VENOM_TAB_STYLE_UNDERLINE && is_selected) {
            VenomPaint ind_paint = venom_paint_fill(bar->indicator_color);
            VenomRectF ind = { x, h - bar->indicator_height, tab_w, bar->indicator_height };
            venom_canvas_draw_rect(canvas, ind, &ind_paint);
        }
        
        x += tab_w;
    }
}

static VenomBool tab_bar_on_event(VenomWidget* widget, const VenomEvent* event) {
    VenomTabBar* bar = (VenomTabBar*)widget;
    
    switch (event->type) {
        case VENOM_EVENT_MOUSE_MOVE: {
            VenomF32 mx = (VenomF32)event->mouse.x + bar->scroll_offset;
            VenomF32 x = 0;
            VenomI32 new_hover = -1;
            
            for (VenomU32 i = 0; i < bar->tab_count; i++) {
                VenomF32 tw = calculate_tab_width(bar, i);
                if (mx >= x && mx < x + tw) {
                    new_hover = (VenomI32)i;
                    break;
                }
                x += tw;
            }
            
            if (new_hover != bar->hover_index) {
                bar->hover_index = new_hover;
                widget->needs_redraw = VENOM_TRUE;
            }
            break;
        }
        
        case VENOM_EVENT_MOUSE_LEAVE:
            if (bar->hover_index != -1) {
                bar->hover_index = -1;
                widget->needs_redraw = VENOM_TRUE;
            }
            break;
            
        case VENOM_EVENT_MOUSE_BUTTON_DOWN:
            if (event->mouse.button == VENOM_MOUSE_BUTTON_LEFT && bar->hover_index >= 0) {
                if (bar->tabs[bar->hover_index].enabled && 
                    (VenomU32)bar->hover_index != bar->selected_index) {
                    bar->selected_index = (VenomU32)bar->hover_index;
                    widget->needs_redraw = VENOM_TRUE;
                    
                    /* Update linked TabView */
                    if (bar->tab_view) {
                        venom_tab_view_set_page(bar->tab_view, bar->selected_index);
                    }
                    
                    if (bar->on_change) {
                        bar->on_change(bar, bar->selected_index, bar->callback_data);
                    }
                }
                return VENOM_TRUE;
            }
            break;
            
        case VENOM_EVENT_KEY_DOWN:
            switch (event->key.key) {
                case VENOM_KEY_LEFT:
                    if (bar->selected_index > 0) {
                        bar->selected_index--;
                        widget->needs_redraw = VENOM_TRUE;
                        if (bar->tab_view) venom_tab_view_set_page(bar->tab_view, bar->selected_index);
                        if (bar->on_change) bar->on_change(bar, bar->selected_index, bar->callback_data);
                    }
                    return VENOM_TRUE;
                    
                case VENOM_KEY_RIGHT:
                    if (bar->selected_index < bar->tab_count - 1) {
                        bar->selected_index++;
                        widget->needs_redraw = VENOM_TRUE;
                        if (bar->tab_view) venom_tab_view_set_page(bar->tab_view, bar->selected_index);
                        if (bar->on_change) bar->on_change(bar, bar->selected_index, bar->callback_data);
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

const VenomWidgetClass venom_tab_bar_class = {
    .class_name = "VenomTabBar",
    .instance_size = sizeof(VenomTabBar),
    .parent_class = &venom_widget_class,
    .init = tab_bar_init,
    .destroy = tab_bar_destroy,
    .measure = tab_bar_measure,
    .layout = NULL,
    .draw = tab_bar_draw,
    .on_event = tab_bar_on_event,
    .on_state_changed = NULL,
};

VenomResultPtr venom_tab_bar_create(void) {
    return venom_widget_create(&venom_tab_bar_class);
}

VenomResult venom_tab_bar_add_tab(VenomTabBar* bar, const char* label, VenomWidget* content) {
    VENOM_ENSURE_NOT_NULL(bar);
    
    if (bar->tab_count >= bar->tab_capacity) {
        VenomU32 new_cap = bar->tab_capacity == 0 ? INITIAL_CAPACITY : bar->tab_capacity * 2;
        VenomTab* new_tabs = (VenomTab*)venom_alloc(new_cap * sizeof(VenomTab));
        if (!new_tabs) return VENOM_ERR_UNIT(VENOM_ERROR_OUT_OF_MEMORY);
        
        if (bar->tabs) {
            memcpy(new_tabs, bar->tabs, bar->tab_count * sizeof(VenomTab));
            venom_free(bar->tabs, bar->tab_capacity * sizeof(VenomTab));
        }
        
        bar->tabs = new_tabs;
        bar->tab_capacity = new_cap;
    }
    
    VenomTab* tab = &bar->tabs[bar->tab_count];
    memset(tab, 0, sizeof(VenomTab));
    
    if (label) {
        VenomSize len = strlen(label) + 1;
        tab->label = (char*)venom_alloc(len);
        if (tab->label) memcpy(tab->label, label, len);
    }
    
    tab->enabled = VENOM_TRUE;
    tab->content = content;
    bar->tab_count++;
    
    /* Also add to linked TabView */
    if (bar->tab_view && content) {
        venom_tab_view_add_page(bar->tab_view, content);
    }
    
    return VENOM_OK_UNIT();
}

void venom_tab_bar_set_selected(VenomTabBar* bar, VenomU32 index) {
    if (bar && index < bar->tab_count) {
        bar->selected_index = index;
        if (bar->tab_view) {
            venom_tab_view_set_page(bar->tab_view, index);
        }
        venom_widget_invalidate((VenomWidget*)bar);
    }
}

VenomU32 venom_tab_bar_get_selected(const VenomTabBar* bar) {
    return bar ? bar->selected_index : 0;
}

void venom_tab_bar_set_style(VenomTabBar* bar, VenomTabStyle style) {
    if (bar) {
        bar->style = style;
        venom_widget_invalidate((VenomWidget*)bar);
    }
}

void venom_tab_bar_set_on_change(VenomTabBar* bar, VenomTabCallback callback, void* data) {
    if (bar) {
        bar->on_change = callback;
        bar->callback_data = data;
    }
}

void venom_tab_bar_link_view(VenomTabBar* bar, VenomTabView* view) {
    if (bar) {
        bar->tab_view = view;
        if (view) view->tab_bar = bar;
    }
}

/* ============================================================================
 * TAB VIEW
 * ============================================================================ */

static void tab_view_init(VenomWidget* widget) {
    VenomTabView* view = (VenomTabView*)widget;
    
    view->pages = NULL;
    view->page_count = 0;
    view->page_capacity = 0;
    view->current_page = 0;
    view->tab_bar = NULL;
}

static void tab_view_destroy(VenomWidget* widget) {
    VenomTabView* view = (VenomTabView*)widget;
    
    for (VenomU32 i = 0; i < view->page_count; i++) {
        if (view->pages[i]) {
            view->pages[i]->parent = NULL;
            venom_unref(view->pages[i]);
        }
    }
    
    if (view->pages) {
        venom_free(view->pages, view->page_capacity * sizeof(VenomWidget*));
    }
    
    venom_widget_class.destroy(widget);
}

static void tab_view_measure(VenomWidget* widget, VenomF32 available_width, VenomF32 available_height,
                             VenomF32* out_width, VenomF32* out_height) {
    VenomTabView* view = (VenomTabView*)widget;
    
    /* Measure current page */
    if (view->current_page < view->page_count && view->pages[view->current_page]) {
        venom_widget_measure(view->pages[view->current_page], available_width, available_height,
                             out_width, out_height);
    } else {
        *out_width = available_width;
        *out_height = available_height;
    }
}

static void tab_view_layout(VenomWidget* widget, VenomRectF bounds) {
    VenomTabView* view = (VenomTabView*)widget;
    widget->bounds = bounds;
    
    /* Layout current page */
    if (view->current_page < view->page_count && view->pages[view->current_page]) {
        VenomRectF page_bounds = { 0, 0, bounds.width, bounds.height };
        venom_widget_layout(view->pages[view->current_page], page_bounds);
    }
}

static void tab_view_draw(VenomWidget* widget, VenomCanvas* canvas) {
    VenomTabView* view = (VenomTabView*)widget;
    
    /* Draw only current page */
    if (view->current_page < view->page_count && view->pages[view->current_page]) {
        VenomWidget* page = view->pages[view->current_page];
        if (page->visible) {
            venom_widget_draw(page, canvas);
        }
    }
}

static VenomBool tab_view_on_event(VenomWidget* widget, const VenomEvent* event) {
    VenomTabView* view = (VenomTabView*)widget;
    
    /* Pass to current page */
    if (view->current_page < view->page_count && view->pages[view->current_page]) {
        return venom_widget_dispatch_event(view->pages[view->current_page], event);
    }
    
    return VENOM_FALSE;
}

const VenomWidgetClass venom_tab_view_class = {
    .class_name = "VenomTabView",
    .instance_size = sizeof(VenomTabView),
    .parent_class = &venom_widget_class,
    .init = tab_view_init,
    .destroy = tab_view_destroy,
    .measure = tab_view_measure,
    .layout = tab_view_layout,
    .draw = tab_view_draw,
    .on_event = tab_view_on_event,
    .on_state_changed = NULL,
};

VenomResultPtr venom_tab_view_create(void) {
    return venom_widget_create(&venom_tab_view_class);
}

VenomResult venom_tab_view_add_page(VenomTabView* view, VenomWidget* page) {
    VENOM_ENSURE_NOT_NULL(view);
    VENOM_ENSURE_NOT_NULL(page);
    
    if (view->page_count >= view->page_capacity) {
        VenomU32 new_cap = view->page_capacity == 0 ? INITIAL_CAPACITY : view->page_capacity * 2;
        VenomWidget** new_pages = (VenomWidget**)venom_alloc(new_cap * sizeof(VenomWidget*));
        if (!new_pages) return VENOM_ERR_UNIT(VENOM_ERROR_OUT_OF_MEMORY);
        
        if (view->pages) {
            memcpy(new_pages, view->pages, view->page_count * sizeof(VenomWidget*));
            venom_free(view->pages, view->page_capacity * sizeof(VenomWidget*));
        }
        
        view->pages = new_pages;
        view->page_capacity = new_cap;
    }
    
    venom_ref(page);
    page->parent = (VenomWidget*)view;
    view->pages[view->page_count++] = page;
    
    return VENOM_OK_UNIT();
}

void venom_tab_view_set_page(VenomTabView* view, VenomU32 index) {
    if (view && index < view->page_count) {
        view->current_page = index;
        venom_widget_invalidate((VenomWidget*)view);
    }
}

VenomU32 venom_tab_view_get_page(const VenomTabView* view) {
    return view ? view->current_page : 0;
}

void venom_tab_view_link_bar(VenomTabView* view, VenomTabBar* bar) {
    if (view) {
        view->tab_bar = bar;
        if (bar) bar->tab_view = view;
    }
}

VenomWidget* _venom_tab_bar_build(const VenomTabBarConfig* config) {
    VenomResultPtr result = venom_tab_bar_create();
    if (!result.ok) return NULL;
    
    VenomTabBar* bar = (VenomTabBar*)result.value;
    bar->style = config->style;
    bar->selected_index = config->selected;
    bar->on_change = config->on_change;
    bar->callback_data = config->data;
    
    return (VenomWidget*)bar;
}

VenomWidget* _venom_tab_view_build(const VenomTabViewConfig* config) {
    VenomResultPtr result = venom_tab_view_create();
    if (!result.ok) return NULL;
    
    VenomTabView* view = (VenomTabView*)result.value;
    view->current_page = config->current;
    if (config->tab_bar) {
        venom_tab_view_link_bar(view, config->tab_bar);
    }
    
    return (VenomWidget*)view;
}
