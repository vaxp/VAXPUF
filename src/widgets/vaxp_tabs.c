/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_tabs.c - TabBar and TabView implementation
 */

#include "vaxp/widgets/vaxp_tabs.h"
#include "vaxp/core/vaxp_memory.h"
#include <string.h>

#define DEFAULT_TAB_HEIGHT 48.0f
#define DEFAULT_TAB_PADDING 16.0f
#define INITIAL_CAPACITY 8

/* ============================================================================
 * TAB BAR
 * ============================================================================ */

static void tab_bar_init(VaxpWidget* widget) {
    VaxpTabBar* bar = (VaxpTabBar*)widget;
    
    bar->tabs = NULL;
    bar->tab_count = 0;
    bar->tab_capacity = 0;
    
    bar->selected_index = 0;
    bar->style = VAXP_TAB_STYLE_UNDERLINE;
    bar->scrollable = VAXP_FALSE;
    bar->scroll_offset = 0;
    
    bar->tab_height = DEFAULT_TAB_HEIGHT;
    bar->tab_padding = DEFAULT_TAB_PADDING;
    bar->indicator_height = 3.0f;
    bar->background_color = (VaxpColor){ 255, 255, 255, 255 };
    bar->tab_color = (VaxpColor){ 97, 97, 97, 255 };
    bar->selected_color = (VaxpColor){ 63, 81, 181, 255 };
    bar->indicator_color = (VaxpColor){ 63, 81, 181, 255 };
    bar->disabled_color = (VaxpColor){ 189, 189, 189, 255 };
    
    bar->hover_index = -1;
    bar->on_change = NULL;
    bar->callback_data = NULL;
    bar->tab_view = NULL;
    
    widget->focusable = VAXP_TRUE;
}

static void tab_bar_destroy(VaxpWidget* widget) {
    VaxpTabBar* bar = (VaxpTabBar*)widget;
    
    for (VaxpU32 i = 0; i < bar->tab_count; i++) {
        if (bar->tabs[i].label) {
            vaxp_free(bar->tabs[i].label, strlen(bar->tabs[i].label) + 1);
        }
        if (bar->tabs[i].icon) {
            vaxp_free(bar->tabs[i].icon, strlen(bar->tabs[i].icon) + 1);
        }
        /* Note: content is owned by TabView */
    }
    
    if (bar->tabs) {
        vaxp_free(bar->tabs, bar->tab_capacity * sizeof(VaxpTab));
    }
    
    vaxp_widget_class.destroy(widget);
}

static VaxpF32 calculate_tab_width(const VaxpTabBar* bar, VaxpU32 index) {
    if (index >= bar->tab_count) return 0;
    const char* label = bar->tabs[index].label;
    VaxpF32 text_width = label ? (VaxpF32)strlen(label) * 9 : 0;
    return text_width + bar->tab_padding * 2;
}

static void tab_bar_measure(VaxpWidget* widget, VaxpF32 available_width, VaxpF32 available_height,
                            VaxpF32* out_width, VaxpF32* out_height) {
    VaxpTabBar* bar = (VaxpTabBar*)widget;
    (void)available_height;
    
    *out_width = available_width;
    *out_height = bar->tab_height;
}

static void tab_bar_draw(VaxpWidget* widget, VaxpCanvas* canvas) {
    VaxpTabBar* bar = (VaxpTabBar*)widget;
    
    VaxpF32 w = widget->bounds.width;
    VaxpF32 h = bar->tab_height;
    
    /* Draw background */
    VaxpRectF bg = { 0, 0, w, h };
    VaxpPaint bg_paint = vaxp_paint_fill(bar->background_color);
    vaxp_canvas_draw_rect(canvas, bg, &bg_paint);
    
    /* Draw bottom border for underline style */
    if (bar->style == VAXP_TAB_STYLE_UNDERLINE) {
        VaxpPaint border_paint = vaxp_paint_fill((VaxpColor){ 224, 224, 224, 255 });
        VaxpRectF border = { 0, h - 1, w, 1 };
        vaxp_canvas_draw_rect(canvas, border, &border_paint);
    }
    
    /* Draw tabs */
    VaxpF32 x = -bar->scroll_offset;
    
    for (VaxpU32 i = 0; i < bar->tab_count; i++) {
        VaxpF32 tab_w = calculate_tab_width(bar, i);
        VaxpRectF tab_rect = { x, 0, tab_w, h };
        
        VaxpBool is_selected = (i == bar->selected_index);
        VaxpBool is_hover = ((VaxpI32)i == bar->hover_index);
        VaxpBool is_enabled = bar->tabs[i].enabled;
        
        /* Draw tab background based on style */
        if (bar->style == VAXP_TAB_STYLE_PILL && is_selected) {
            VaxpPaint pill_paint = vaxp_paint_fill((VaxpColor){ 63, 81, 181, 40 });
            VaxpRectF pill = { x + 4, 8, tab_w - 8, h - 16 };
            vaxp_canvas_draw_rounded_rect(canvas, pill, (h - 16) / 2, &pill_paint);
        } else if (bar->style == VAXP_TAB_STYLE_BOXED && is_selected) {
            VaxpPaint box_paint = vaxp_paint_fill((VaxpColor){ 255, 255, 255, 255 });
            VaxpRectF box = { x, 0, tab_w, h - 1 };
            vaxp_canvas_draw_rect(canvas, box, &box_paint);
            
            VaxpPaint box_border = vaxp_paint_stroke((VaxpColor){ 224, 224, 224, 255 }, 1.0f);
            vaxp_canvas_draw_rect(canvas, (VaxpRectF){ x, 0, tab_w, h + 1 }, &box_border);
        }
        
        /* Hover highlight */
        if (is_hover && !is_selected && is_enabled) {
            VaxpPaint hover_paint = vaxp_paint_fill((VaxpColor){ 0, 0, 0, 15 });
            vaxp_canvas_draw_rect(canvas, tab_rect, &hover_paint);
        }
        
        /* Draw label */
        VaxpColor label_color;
        if (!is_enabled) {
            label_color = bar->disabled_color;
        } else if (is_selected) {
            label_color = bar->selected_color;
        } else {
            label_color = bar->tab_color;
        }
        
        VaxpPaint label_paint = vaxp_paint_fill(label_color);
        if (bar->tabs[i].label) {
            VaxpF32 text_w = (VaxpF32)strlen(bar->tabs[i].label) * 9;
            VaxpF32 tx = x + (tab_w - text_w) / 2;
            VaxpF32 ty = h / 2 + 5;
            vaxp_canvas_draw_text(canvas, bar->tabs[i].label, tx, ty, NULL, &label_paint);
        }
        
        /* Draw indicator for underline style */
        if (bar->style == VAXP_TAB_STYLE_UNDERLINE && is_selected) {
            VaxpPaint ind_paint = vaxp_paint_fill(bar->indicator_color);
            VaxpRectF ind = { x, h - bar->indicator_height, tab_w, bar->indicator_height };
            vaxp_canvas_draw_rect(canvas, ind, &ind_paint);
        }
        
        x += tab_w;
    }
}

static VaxpBool tab_bar_on_event(VaxpWidget* widget, const VaxpEvent* event) {
    VaxpTabBar* bar = (VaxpTabBar*)widget;
    
    switch (event->type) {
        case VAXP_EVENT_MOUSE_MOVE: {
            VaxpF32 mx = (VaxpF32)event->mouse.x + bar->scroll_offset;
            VaxpF32 x = 0;
            VaxpI32 new_hover = -1;
            
            for (VaxpU32 i = 0; i < bar->tab_count; i++) {
                VaxpF32 tw = calculate_tab_width(bar, i);
                if (mx >= x && mx < x + tw) {
                    new_hover = (VaxpI32)i;
                    break;
                }
                x += tw;
            }
            
            if (new_hover != bar->hover_index) {
                bar->hover_index = new_hover;
                widget->needs_redraw = VAXP_TRUE;
            }
            break;
        }
        
        case VAXP_EVENT_MOUSE_LEAVE:
            if (bar->hover_index != -1) {
                bar->hover_index = -1;
                widget->needs_redraw = VAXP_TRUE;
            }
            break;
            
        case VAXP_EVENT_MOUSE_BUTTON_DOWN:
            if (event->mouse.button == VAXP_MOUSE_BUTTON_LEFT && bar->hover_index >= 0) {
                if (bar->tabs[bar->hover_index].enabled && 
                    (VaxpU32)bar->hover_index != bar->selected_index) {
                    bar->selected_index = (VaxpU32)bar->hover_index;
                    widget->needs_redraw = VAXP_TRUE;
                    
                    /* Update linked TabView */
                    if (bar->tab_view) {
                        vaxp_tab_view_set_page(bar->tab_view, bar->selected_index);
                    }
                    
                    if (bar->on_change) {
                        bar->on_change(bar, bar->selected_index, bar->callback_data);
                    }
                }
                return VAXP_TRUE;
            }
            break;
            
        case VAXP_EVENT_KEY_DOWN:
            switch (event->key.key) {
                case VAXP_KEY_LEFT:
                    if (bar->selected_index > 0) {
                        bar->selected_index--;
                        widget->needs_redraw = VAXP_TRUE;
                        if (bar->tab_view) vaxp_tab_view_set_page(bar->tab_view, bar->selected_index);
                        if (bar->on_change) bar->on_change(bar, bar->selected_index, bar->callback_data);
                    }
                    return VAXP_TRUE;
                    
                case VAXP_KEY_RIGHT:
                    if (bar->selected_index < bar->tab_count - 1) {
                        bar->selected_index++;
                        widget->needs_redraw = VAXP_TRUE;
                        if (bar->tab_view) vaxp_tab_view_set_page(bar->tab_view, bar->selected_index);
                        if (bar->on_change) bar->on_change(bar, bar->selected_index, bar->callback_data);
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

const VaxpWidgetClass vaxp_tab_bar_class = {
    .class_name = "VaxpTabBar",
    .instance_size = sizeof(VaxpTabBar),
    .parent_class = &vaxp_widget_class,
    .init = tab_bar_init,
    .destroy = tab_bar_destroy,
    .measure = tab_bar_measure,
    .layout = NULL,
    .draw = tab_bar_draw,
    .on_event = tab_bar_on_event,
    .on_state_changed = NULL,
};

VaxpResultPtr vaxp_tab_bar_create(void) {
    return vaxp_widget_create(&vaxp_tab_bar_class);
}

VaxpResult vaxp_tab_bar_add_tab(VaxpTabBar* bar, const char* label, VaxpWidget* content) {
    VAXP_ENSURE_NOT_NULL(bar);
    
    if (bar->tab_count >= bar->tab_capacity) {
        VaxpU32 new_cap = bar->tab_capacity == 0 ? INITIAL_CAPACITY : bar->tab_capacity * 2;
        VaxpTab* new_tabs = (VaxpTab*)vaxp_alloc(new_cap * sizeof(VaxpTab));
        if (!new_tabs) return VAXP_ERR_UNIT(VAXP_ERROR_OUT_OF_MEMORY);
        
        if (bar->tabs) {
            memcpy(new_tabs, bar->tabs, bar->tab_count * sizeof(VaxpTab));
            vaxp_free(bar->tabs, bar->tab_capacity * sizeof(VaxpTab));
        }
        
        bar->tabs = new_tabs;
        bar->tab_capacity = new_cap;
    }
    
    VaxpTab* tab = &bar->tabs[bar->tab_count];
    memset(tab, 0, sizeof(VaxpTab));
    
    if (label) {
        VaxpSize len = strlen(label) + 1;
        tab->label = (char*)vaxp_alloc(len);
        if (tab->label) memcpy(tab->label, label, len);
    }
    
    tab->enabled = VAXP_TRUE;
    tab->content = content;
    bar->tab_count++;
    
    /* Also add to linked TabView */
    if (bar->tab_view && content) {
        vaxp_tab_view_add_page(bar->tab_view, content);
    }
    
    return VAXP_OK_UNIT();
}

void vaxp_tab_bar_set_selected(VaxpTabBar* bar, VaxpU32 index) {
    if (bar && index < bar->tab_count) {
        bar->selected_index = index;
        if (bar->tab_view) {
            vaxp_tab_view_set_page(bar->tab_view, index);
        }
        vaxp_widget_invalidate((VaxpWidget*)bar);
    }
}

VaxpU32 vaxp_tab_bar_get_selected(const VaxpTabBar* bar) {
    return bar ? bar->selected_index : 0;
}

void vaxp_tab_bar_set_style(VaxpTabBar* bar, VaxpTabStyle style) {
    if (bar) {
        bar->style = style;
        vaxp_widget_invalidate((VaxpWidget*)bar);
    }
}

void vaxp_tab_bar_set_on_change(VaxpTabBar* bar, VaxpTabCallback callback, void* data) {
    if (bar) {
        bar->on_change = callback;
        bar->callback_data = data;
    }
}

void vaxp_tab_bar_link_view(VaxpTabBar* bar, VaxpTabView* view) {
    if (bar) {
        bar->tab_view = view;
        if (view) view->tab_bar = bar;
    }
}

/* ============================================================================
 * TAB VIEW
 * ============================================================================ */

static void tab_view_init(VaxpWidget* widget) {
    VaxpTabView* view = (VaxpTabView*)widget;
    
    view->pages = NULL;
    view->page_count = 0;
    view->page_capacity = 0;
    view->current_page = 0;
    view->tab_bar = NULL;
}

static void tab_view_destroy(VaxpWidget* widget) {
    VaxpTabView* view = (VaxpTabView*)widget;
    
    for (VaxpU32 i = 0; i < view->page_count; i++) {
        if (view->pages[i]) {
            view->pages[i]->parent = NULL;
            vaxp_unref(view->pages[i]);
        }
    }
    
    if (view->pages) {
        vaxp_free(view->pages, view->page_capacity * sizeof(VaxpWidget*));
    }
    
    vaxp_widget_class.destroy(widget);
}

static void tab_view_measure(VaxpWidget* widget, VaxpF32 available_width, VaxpF32 available_height,
                             VaxpF32* out_width, VaxpF32* out_height) {
    VaxpTabView* view = (VaxpTabView*)widget;
    
    /* Measure current page */
    if (view->current_page < view->page_count && view->pages[view->current_page]) {
        vaxp_widget_measure(view->pages[view->current_page], available_width, available_height,
                             out_width, out_height);
    } else {
        *out_width = available_width;
        *out_height = available_height;
    }
}

static void tab_view_layout(VaxpWidget* widget, VaxpRectF bounds) {
    VaxpTabView* view = (VaxpTabView*)widget;
    widget->bounds = bounds;
    
    /* Layout current page */
    if (view->current_page < view->page_count && view->pages[view->current_page]) {
        VaxpRectF page_bounds = { 0, 0, bounds.width, bounds.height };
        vaxp_widget_layout(view->pages[view->current_page], page_bounds);
    }
}

static void tab_view_draw(VaxpWidget* widget, VaxpCanvas* canvas) {
    VaxpTabView* view = (VaxpTabView*)widget;
    
    /* Draw only current page */
    if (view->current_page < view->page_count && view->pages[view->current_page]) {
        VaxpWidget* page = view->pages[view->current_page];
        if (page->visible) {
            vaxp_widget_draw(page, canvas);
        }
    }
}

static VaxpBool tab_view_on_event(VaxpWidget* widget, const VaxpEvent* event) {
    VaxpTabView* view = (VaxpTabView*)widget;
    
    /* Pass to current page */
    if (view->current_page < view->page_count && view->pages[view->current_page]) {
        return vaxp_widget_dispatch_event(view->pages[view->current_page], event);
    }
    
    return VAXP_FALSE;
}

const VaxpWidgetClass vaxp_tab_view_class = {
    .class_name = "VaxpTabView",
    .instance_size = sizeof(VaxpTabView),
    .parent_class = &vaxp_widget_class,
    .init = tab_view_init,
    .destroy = tab_view_destroy,
    .measure = tab_view_measure,
    .layout = tab_view_layout,
    .draw = tab_view_draw,
    .on_event = tab_view_on_event,
    .on_state_changed = NULL,
};

VaxpResultPtr vaxp_tab_view_create(void) {
    return vaxp_widget_create(&vaxp_tab_view_class);
}

VaxpResult vaxp_tab_view_add_page(VaxpTabView* view, VaxpWidget* page) {
    VAXP_ENSURE_NOT_NULL(view);
    VAXP_ENSURE_NOT_NULL(page);
    
    if (view->page_count >= view->page_capacity) {
        VaxpU32 new_cap = view->page_capacity == 0 ? INITIAL_CAPACITY : view->page_capacity * 2;
        VaxpWidget** new_pages = (VaxpWidget**)vaxp_alloc(new_cap * sizeof(VaxpWidget*));
        if (!new_pages) return VAXP_ERR_UNIT(VAXP_ERROR_OUT_OF_MEMORY);
        
        if (view->pages) {
            memcpy(new_pages, view->pages, view->page_count * sizeof(VaxpWidget*));
            vaxp_free(view->pages, view->page_capacity * sizeof(VaxpWidget*));
        }
        
        view->pages = new_pages;
        view->page_capacity = new_cap;
    }
    
    vaxp_ref(page);
    page->parent = (VaxpWidget*)view;
    view->pages[view->page_count++] = page;
    
    return VAXP_OK_UNIT();
}

void vaxp_tab_view_set_page(VaxpTabView* view, VaxpU32 index) {
    if (view && index < view->page_count) {
        view->current_page = index;
        vaxp_widget_invalidate((VaxpWidget*)view);
    }
}

VaxpU32 vaxp_tab_view_get_page(const VaxpTabView* view) {
    return view ? view->current_page : 0;
}

void vaxp_tab_view_link_bar(VaxpTabView* view, VaxpTabBar* bar) {
    if (view) {
        view->tab_bar = bar;
        if (bar) bar->tab_view = view;
    }
}

VaxpWidget* _vaxp_tab_bar_build(const VaxpTabBarConfig* config) {
    VaxpResultPtr result = vaxp_tab_bar_create();
    if (!result.ok) return NULL;
    
    VaxpTabBar* bar = (VaxpTabBar*)result.value;
    bar->style = config->style;
    bar->selected_index = config->selected;
    bar->on_change = config->on_change;
    bar->callback_data = config->data;
    
    return (VaxpWidget*)bar;
}

VaxpWidget* _vaxp_tab_view_build(const VaxpTabViewConfig* config) {
    VaxpResultPtr result = vaxp_tab_view_create();
    if (!result.ok) return NULL;
    
    VaxpTabView* view = (VaxpTabView*)result.value;
    view->current_page = config->current;
    if (config->tab_bar) {
        vaxp_tab_view_link_bar(view, config->tab_bar);
    }
    
    return (VaxpWidget*)view;
}
