/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_titlebar.c - Custom Window Title Bar implementation
 */

#include "vaxp/widgets/vaxp_titlebar.h"
#include "vaxp/core/vaxp_memory.h"
#include "vaxp/graphics/vaxp_canvas.h"
#include <string.h>

/* ============================================================================
 * FORWARD DECLARATIONS
 * ============================================================================ */

static void titlebar_init(VaxpWidget* widget);
static void titlebar_destroy(VaxpWidget* widget);
static void titlebar_measure(VaxpWidget* widget, VaxpF32 available_width, VaxpF32 available_height,
                              VaxpF32* out_width, VaxpF32* out_height);
static void titlebar_layout(VaxpWidget* widget, VaxpRectF bounds);
static void titlebar_draw(VaxpWidget* widget, VaxpCanvas* canvas);
static VaxpBool titlebar_on_event(VaxpWidget* widget, const VaxpEvent* event);

/* ============================================================================
 * WIDGET CLASS
 * ============================================================================ */

const VaxpWidgetClass vaxp_titlebar_class = {
    .class_name = "VaxpTitleBar",
    .instance_size = sizeof(VaxpTitleBar),
    .parent_class = &vaxp_widget_class,
    .init = titlebar_init,
    .destroy = titlebar_destroy,
    .measure = titlebar_measure,
    .layout = titlebar_layout,
    .draw = titlebar_draw,
    .on_event = titlebar_on_event,
    .on_state_changed = NULL,
};

/* ============================================================================
 * HELPER FUNCTIONS
 * ============================================================================ */

static char* str_dup(const char* s) {
    if (!s) return NULL;
    size_t len = strlen(s);
    char* copy = vaxp_alloc(len + 1);
    if (copy) memcpy(copy, s, len + 1);
    return copy;
}

static void str_free(char* s) {
    if (s) vaxp_free(s, strlen(s) + 1);
}

static VaxpI32 count_visible_buttons(VaxpTitleBarButton buttons) {
    VaxpI32 count = 0;
    if (buttons & VAXP_TITLEBAR_BUTTON_MINIMIZE) count++;
    if (buttons & VAXP_TITLEBAR_BUTTON_MAXIMIZE) count++;
    if (buttons & VAXP_TITLEBAR_BUTTON_CLOSE) count++;
    return count;
}

/* ============================================================================
 * LIFECYCLE
 * ============================================================================ */

static void titlebar_init(VaxpWidget* widget) {
    VaxpTitleBar* bar = (VaxpTitleBar*)widget;
    
    bar->title = NULL;
    bar->icon = NULL;
    bar->left_widgets = NULL;
    bar->left_widget_count = 0;
    bar->right_widgets = NULL;
    bar->right_widget_count = 0;
    
    bar->visible_buttons = VAXP_TITLEBAR_BUTTON_ALL;
    bar->hover_button = -1;
    bar->pressed_button = -1;
    bar->is_maximized = VAXP_FALSE;
    
    bar->on_minimize = NULL;
    bar->on_maximize = NULL;
    bar->on_restore = NULL;
    bar->on_close = NULL;
    bar->on_drag = NULL;
    bar->user_data = NULL;
    
    bar->is_dragging = VAXP_FALSE;
    bar->drag_start_x = 0;
    bar->drag_start_y = 0;
    
    bar->background = vaxp_color_rgb(245, 245, 245);
    bar->title_color = vaxp_color_rgb(30, 30, 30);
    bar->button_hover = vaxp_color_rgba(0, 0, 0, 20);
    bar->button_pressed = vaxp_color_rgba(0, 0, 0, 40);
    bar->close_hover = vaxp_color_rgb(232, 17, 35);
    bar->close_pressed = vaxp_color_rgb(200, 15, 30);
    bar->icon_color = vaxp_color_rgb(50, 50, 50);
    bar->height = 32.0f;
    bar->button_width = 46.0f;
    bar->padding_h = 12.0f;
    bar->icon_padding = 8.0f;
    bar->title_font_size = 13.0f;
    bar->center_title = VAXP_FALSE;
    
    widget->layout.min_height = bar->height;
}

static void titlebar_destroy(VaxpWidget* widget) {
    VaxpTitleBar* bar = (VaxpTitleBar*)widget;
    str_free(bar->title);
    
    if (bar->left_widgets) {
        vaxp_free(bar->left_widgets, bar->left_widget_count * sizeof(VaxpWidget*));
    }
    if (bar->right_widgets) {
        vaxp_free(bar->right_widgets, bar->right_widget_count * sizeof(VaxpWidget*));
    }
    
    vaxp_widget_class.destroy(widget);
}

VaxpResultPtr vaxp_titlebar_create(const char* title) {
    VaxpResultPtr result = vaxp_widget_create(&vaxp_titlebar_class);
    if (!result.ok) return result;
    
    VaxpTitleBar* bar = (VaxpTitleBar*)result.value;
    bar->title = str_dup(title);
    
    return result;
}

/* ============================================================================
 * CONFIGURATION
 * ============================================================================ */

void vaxp_titlebar_set_title(VaxpTitleBar* bar, const char* title) {
    if (!bar) return;
    str_free(bar->title);
    bar->title = str_dup(title);
    vaxp_widget_invalidate(&bar->base);
}

const char* vaxp_titlebar_get_title(const VaxpTitleBar* bar) {
    return bar ? bar->title : NULL;
}

void vaxp_titlebar_set_icon(VaxpTitleBar* bar, VaxpWidget* icon) {
    if (!bar) return;
    bar->icon = icon;
    vaxp_widget_invalidate(&bar->base);
}

void vaxp_titlebar_set_buttons(VaxpTitleBar* bar, VaxpTitleBarButton buttons) {
    if (!bar) return;
    bar->visible_buttons = buttons;
    vaxp_widget_invalidate(&bar->base);
}

void vaxp_titlebar_set_maximized(VaxpTitleBar* bar, VaxpBool maximized) {
    if (!bar) return;
    bar->is_maximized = maximized;
    vaxp_widget_invalidate(&bar->base);
}

VaxpBool vaxp_titlebar_is_maximized(const VaxpTitleBar* bar) {
    return bar ? bar->is_maximized : VAXP_FALSE;
}

/* ============================================================================
 * CUSTOM WIDGETS
 * ============================================================================ */

VaxpResult vaxp_titlebar_add_left_widget(VaxpTitleBar* bar, VaxpWidget* widget) {
    if (!bar || !widget) {
        return VAXP_ERR_UNIT(VAXP_ERROR_INVALID_ARGUMENT);
    }
    
    VaxpU32 new_count = bar->left_widget_count + 1;
    VaxpWidget** new_widgets = vaxp_alloc(new_count * sizeof(VaxpWidget*));
    if (!new_widgets) {
        return VAXP_ERR_UNIT(VAXP_ERROR_OUT_OF_MEMORY);
    }
    
    if (bar->left_widgets) {
        memcpy(new_widgets, bar->left_widgets, bar->left_widget_count * sizeof(VaxpWidget*));
        vaxp_free(bar->left_widgets, bar->left_widget_count * sizeof(VaxpWidget*));
    }
    
    new_widgets[bar->left_widget_count] = widget;
    bar->left_widgets = new_widgets;
    bar->left_widget_count = new_count;
    
    vaxp_widget_invalidate(&bar->base);
    return VAXP_OK_UNIT();
}

VaxpResult vaxp_titlebar_add_right_widget(VaxpTitleBar* bar, VaxpWidget* widget) {
    if (!bar || !widget) {
        return VAXP_ERR_UNIT(VAXP_ERROR_INVALID_ARGUMENT);
    }
    
    VaxpU32 new_count = bar->right_widget_count + 1;
    VaxpWidget** new_widgets = vaxp_alloc(new_count * sizeof(VaxpWidget*));
    if (!new_widgets) {
        return VAXP_ERR_UNIT(VAXP_ERROR_OUT_OF_MEMORY);
    }
    
    if (bar->right_widgets) {
        memcpy(new_widgets, bar->right_widgets, bar->right_widget_count * sizeof(VaxpWidget*));
        vaxp_free(bar->right_widgets, bar->right_widget_count * sizeof(VaxpWidget*));
    }
    
    new_widgets[bar->right_widget_count] = widget;
    bar->right_widgets = new_widgets;
    bar->right_widget_count = new_count;
    
    vaxp_widget_invalidate(&bar->base);
    return VAXP_OK_UNIT();
}

void vaxp_titlebar_clear_widgets(VaxpTitleBar* bar) {
    if (!bar) return;
    
    if (bar->left_widgets) {
        vaxp_free(bar->left_widgets, bar->left_widget_count * sizeof(VaxpWidget*));
        bar->left_widgets = NULL;
        bar->left_widget_count = 0;
    }
    
    if (bar->right_widgets) {
        vaxp_free(bar->right_widgets, bar->right_widget_count * sizeof(VaxpWidget*));
        bar->right_widgets = NULL;
        bar->right_widget_count = 0;
    }
    
    vaxp_widget_invalidate(&bar->base);
}

/* ============================================================================
 * CALLBACKS
 * ============================================================================ */

void vaxp_titlebar_on_minimize(VaxpTitleBar* bar, VaxpTitleBarCallback callback, void* data) {
    if (!bar) return;
    bar->on_minimize = callback;
    bar->user_data = data;
}

void vaxp_titlebar_on_maximize(VaxpTitleBar* bar, VaxpTitleBarCallback callback, void* data) {
    if (!bar) return;
    bar->on_maximize = callback;
    bar->user_data = data;
}

void vaxp_titlebar_on_restore(VaxpTitleBar* bar, VaxpTitleBarCallback callback, void* data) {
    if (!bar) return;
    bar->on_restore = callback;
    bar->user_data = data;
}

void vaxp_titlebar_on_close(VaxpTitleBar* bar, VaxpTitleBarCallback callback, void* data) {
    if (!bar) return;
    bar->on_close = callback;
    bar->user_data = data;
}

void vaxp_titlebar_on_drag(VaxpTitleBar* bar, VaxpTitleBarDragCallback callback, void* data) {
    if (!bar) return;
    bar->on_drag = callback;
    bar->user_data = data;
}

/* ============================================================================
 * APPEARANCE
 * ============================================================================ */

void vaxp_titlebar_set_background(VaxpTitleBar* bar, VaxpColor color) {
    if (!bar) return;
    bar->background = color;
    vaxp_widget_invalidate(&bar->base);
}

void vaxp_titlebar_set_title_color(VaxpTitleBar* bar, VaxpColor color) {
    if (!bar) return;
    bar->title_color = color;
    vaxp_widget_invalidate(&bar->base);
}

void vaxp_titlebar_set_height(VaxpTitleBar* bar, VaxpF32 height) {
    if (!bar) return;
    bar->height = height;
    bar->base.layout.min_height = height;
    vaxp_widget_invalidate(&bar->base);
}

void vaxp_titlebar_set_center_title(VaxpTitleBar* bar, VaxpBool centered) {
    if (!bar) return;
    bar->center_title = centered;
    vaxp_widget_invalidate(&bar->base);
}

void vaxp_titlebar_set_button_width(VaxpTitleBar* bar, VaxpF32 width) {
    if (!bar) return;
    bar->button_width = width;
    vaxp_widget_invalidate(&bar->base);
}

/* ============================================================================
 * THEMES
 * ============================================================================ */

void vaxp_titlebar_apply_light_theme(VaxpTitleBar* bar) {
    if (!bar) return;
    bar->background = vaxp_color_rgb(245, 245, 245);
    bar->title_color = vaxp_color_rgb(30, 30, 30);
    bar->button_hover = vaxp_color_rgba(0, 0, 0, 20);
    bar->button_pressed = vaxp_color_rgba(0, 0, 0, 40);
    bar->close_hover = vaxp_color_rgb(232, 17, 35);
    bar->close_pressed = vaxp_color_rgb(200, 15, 30);
    bar->icon_color = vaxp_color_rgb(50, 50, 50);
    vaxp_widget_invalidate(&bar->base);
}

void vaxp_titlebar_apply_dark_theme(VaxpTitleBar* bar) {
    if (!bar) return;
    bar->background = vaxp_color_rgb(32, 32, 32);
    bar->title_color = vaxp_color_rgb(255, 255, 255);
    bar->button_hover = vaxp_color_rgba(255, 255, 255, 25);
    bar->button_pressed = vaxp_color_rgba(255, 255, 255, 40);
    bar->close_hover = vaxp_color_rgb(232, 17, 35);
    bar->close_pressed = vaxp_color_rgb(200, 15, 30);
    bar->icon_color = vaxp_color_rgb(255, 255, 255);
    vaxp_widget_invalidate(&bar->base);
}

/* ============================================================================
 * WIDGET CALLBACKS
 * ============================================================================ */

static void titlebar_measure(VaxpWidget* widget, VaxpF32 available_width, 
                              VaxpF32 available_height, VaxpF32* out_width, VaxpF32* out_height) {
    VaxpTitleBar* bar = (VaxpTitleBar*)widget;
    (void)available_height;
    
    *out_width = available_width > 0 ? available_width : 400.0f;
    *out_height = bar->height;
}

static void titlebar_layout(VaxpWidget* widget, VaxpRectF bounds) {
    widget->bounds = bounds;
}

static void draw_minimize_icon(VaxpCanvas* canvas, VaxpF32 cx, VaxpF32 cy, VaxpColor color) {
    VaxpPaint paint = vaxp_paint_stroke(color, 1.0f);
    vaxp_canvas_draw_line(canvas, cx - 5, cy, cx + 5, cy, &paint);
}

static void draw_maximize_icon(VaxpCanvas* canvas, VaxpF32 cx, VaxpF32 cy,
                                VaxpColor color, VaxpBool is_maximized) {
    VaxpPaint paint = vaxp_paint_stroke(color, 1.0f);
    
    if (is_maximized) {
        VaxpRectF rect1 = { cx - 3, cy - 5, 8, 8 };
        VaxpRectF rect2 = { cx - 5, cy - 3, 8, 8 };
        vaxp_canvas_draw_rect(canvas, rect1, &paint);
        vaxp_canvas_draw_rect(canvas, rect2, &paint);
    } else {
        VaxpRectF rect = { cx - 5, cy - 5, 10, 10 };
        vaxp_canvas_draw_rect(canvas, rect, &paint);
    }
}

static void draw_close_icon(VaxpCanvas* canvas, VaxpF32 cx, VaxpF32 cy, VaxpColor color) {
    VaxpPaint paint = vaxp_paint_stroke(color, 1.2f);
    vaxp_canvas_draw_line(canvas, cx - 5, cy - 5, cx + 5, cy + 5, &paint);
    vaxp_canvas_draw_line(canvas, cx + 5, cy - 5, cx - 5, cy + 5, &paint);
}

static void titlebar_draw(VaxpWidget* widget, VaxpCanvas* canvas) {
    VaxpTitleBar* bar = (VaxpTitleBar*)widget;
    VaxpRectF bounds = widget->bounds;
    
    VaxpPaint bg_paint = vaxp_paint_fill(bar->background);
    vaxp_canvas_draw_rect(canvas, bounds, &bg_paint);
    
    VaxpPaint border_paint = vaxp_paint_stroke(vaxp_color_rgba(0, 0, 0, 25), 1.0f);
    vaxp_canvas_draw_line(canvas, bounds.x, bounds.y + bounds.height - 0.5f,
                            bounds.x + bounds.width, bounds.y + bounds.height - 0.5f,
                            &border_paint);
    
    VaxpI32 button_count = count_visible_buttons(bar->visible_buttons);
    VaxpF32 buttons_width = button_count * bar->button_width;
    
    VaxpF32 title_x;
    if (bar->center_title) {
        VaxpF32 title_width = bar->title ? (VaxpF32)strlen(bar->title) * bar->title_font_size * 0.6f : 0;
        title_x = bounds.x + (bounds.width - title_width) / 2;
    } else {
        title_x = bounds.x + bar->padding_h;
        if (bar->icon) {
            title_x += 20 + bar->icon_padding;
        }
    }
    
    if (bar->title) {
        VaxpPaint title_paint = vaxp_paint_fill(bar->title_color);
        VaxpF32 title_y = bounds.y + bounds.height / 2 + bar->title_font_size * 0.35f;
        vaxp_canvas_draw_text(canvas, bar->title, title_x, title_y, NULL, &title_paint);
    }
    
    VaxpF32 btn_x = bounds.x + bounds.width - buttons_width;
    VaxpF32 btn_y = bounds.y;
    VaxpI32 btn_index = 0;
    
    if (bar->visible_buttons & VAXP_TITLEBAR_BUTTON_MINIMIZE) {
        VaxpRectF btn_rect = { btn_x, btn_y, bar->button_width, bar->height };
        
        if (bar->pressed_button == btn_index) {
            VaxpPaint pressed = vaxp_paint_fill(bar->button_pressed);
            vaxp_canvas_draw_rect(canvas, btn_rect, &pressed);
        } else if (bar->hover_button == btn_index) {
            VaxpPaint hover = vaxp_paint_fill(bar->button_hover);
            vaxp_canvas_draw_rect(canvas, btn_rect, &hover);
        }
        
        draw_minimize_icon(canvas, btn_x + bar->button_width / 2, 
                           btn_y + bar->height / 2, bar->icon_color);
        
        btn_x += bar->button_width;
        btn_index++;
    }
    
    if (bar->visible_buttons & VAXP_TITLEBAR_BUTTON_MAXIMIZE) {
        VaxpRectF btn_rect = { btn_x, btn_y, bar->button_width, bar->height };
        
        if (bar->pressed_button == btn_index) {
            VaxpPaint pressed = vaxp_paint_fill(bar->button_pressed);
            vaxp_canvas_draw_rect(canvas, btn_rect, &pressed);
        } else if (bar->hover_button == btn_index) {
            VaxpPaint hover = vaxp_paint_fill(bar->button_hover);
            vaxp_canvas_draw_rect(canvas, btn_rect, &hover);
        }
        
        draw_maximize_icon(canvas, btn_x + bar->button_width / 2,
                           btn_y + bar->height / 2, bar->icon_color, bar->is_maximized);
        
        btn_x += bar->button_width;
        btn_index++;
    }
    
    if (bar->visible_buttons & VAXP_TITLEBAR_BUTTON_CLOSE) {
        VaxpRectF btn_rect = { btn_x, btn_y, bar->button_width, bar->height };
        
        VaxpColor icon_color = bar->icon_color;
        if (bar->pressed_button == btn_index) {
            VaxpPaint pressed = vaxp_paint_fill(bar->close_pressed);
            vaxp_canvas_draw_rect(canvas, btn_rect, &pressed);
            icon_color = vaxp_color_rgb(255, 255, 255);
        } else if (bar->hover_button == btn_index) {
            VaxpPaint hover = vaxp_paint_fill(bar->close_hover);
            vaxp_canvas_draw_rect(canvas, btn_rect, &hover);
            icon_color = vaxp_color_rgb(255, 255, 255);
        }
        
        draw_close_icon(canvas, btn_x + bar->button_width / 2,
                        btn_y + bar->height / 2, icon_color);
    }
}

static VaxpI32 titlebar_hit_test_button(VaxpTitleBar* bar, VaxpF32 x) {
    VaxpRectF bounds = bar->base.bounds;
    VaxpI32 button_count = count_visible_buttons(bar->visible_buttons);
    VaxpF32 buttons_width = button_count * bar->button_width;
    VaxpF32 buttons_start = bounds.x + bounds.width - buttons_width;
    
    if (x < buttons_start) return -1;
    
    VaxpF32 rel_x = x - buttons_start;
    VaxpI32 index = (VaxpI32)(rel_x / bar->button_width);
    
    if (index >= 0 && index < button_count) {
        return index;
    }
    return -1;
}

static VaxpBool titlebar_on_event(VaxpWidget* widget, const VaxpEvent* event) {
    VaxpTitleBar* bar = (VaxpTitleBar*)widget;
    
    switch (event->type) {
        case VAXP_EVENT_MOUSE_MOVE: {
            VaxpI32 button = titlebar_hit_test_button(bar, (VaxpF32)event->mouse.x);
            
            if (bar->is_dragging && bar->on_drag) {
                VaxpI32 dx = event->mouse.x - bar->drag_start_x;
                VaxpI32 dy = event->mouse.y - bar->drag_start_y;
                bar->on_drag(bar, dx, dy, bar->user_data);
                bar->drag_start_x = event->mouse.x;
                bar->drag_start_y = event->mouse.y;
                return VAXP_TRUE;
            }
            
            if (button != bar->hover_button) {
                bar->hover_button = button;
                vaxp_widget_invalidate(widget);
            }
            return VAXP_TRUE;
        }
        
        case VAXP_EVENT_MOUSE_LEAVE:
            bar->hover_button = -1;
            bar->pressed_button = -1;
            bar->is_dragging = VAXP_FALSE;
            vaxp_widget_invalidate(widget);
            return VAXP_TRUE;
            
        case VAXP_EVENT_MOUSE_BUTTON_DOWN: {
            if (event->mouse.button != VAXP_MOUSE_BUTTON_LEFT) return VAXP_FALSE;
            
            VaxpI32 button = titlebar_hit_test_button(bar, (VaxpF32)event->mouse.x);
            if (button >= 0) {
                bar->pressed_button = button;
                vaxp_widget_invalidate(widget);
                return VAXP_TRUE;
            }
            
            bar->is_dragging = VAXP_TRUE;
            bar->drag_start_x = event->mouse.x;
            bar->drag_start_y = event->mouse.y;
            return VAXP_TRUE;
        }
        
        case VAXP_EVENT_MOUSE_BUTTON_UP: {
            if (event->mouse.button != VAXP_MOUSE_BUTTON_LEFT) return VAXP_FALSE;
            
            VaxpI32 pressed = bar->pressed_button;
            bar->pressed_button = -1;
            bar->is_dragging = VAXP_FALSE;
            
            if (pressed >= 0) {
                VaxpI32 button = titlebar_hit_test_button(bar, (VaxpF32)event->mouse.x);
                if (button == pressed) {
                    VaxpI32 idx = 0;
                    if (bar->visible_buttons & VAXP_TITLEBAR_BUTTON_MINIMIZE) {
                        if (pressed == idx && bar->on_minimize) {
                            bar->on_minimize(bar, bar->user_data);
                        }
                        idx++;
                    }
                    if (bar->visible_buttons & VAXP_TITLEBAR_BUTTON_MAXIMIZE) {
                        if (pressed == idx) {
                            if (bar->is_maximized && bar->on_restore) {
                                bar->on_restore(bar, bar->user_data);
                            } else if (!bar->is_maximized && bar->on_maximize) {
                                bar->on_maximize(bar, bar->user_data);
                            }
                        }
                        idx++;
                    }
                    if (bar->visible_buttons & VAXP_TITLEBAR_BUTTON_CLOSE) {
                        if (pressed == idx && bar->on_close) {
                            bar->on_close(bar, bar->user_data);
                        }
                    }
                }
                vaxp_widget_invalidate(widget);
                return VAXP_TRUE;
            }
            return VAXP_FALSE;
        }
        
        default:
            return VAXP_FALSE;
    }
}
