/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_titlebar.c - Custom Window Title Bar implementation
 */

#include "venom/widgets/venom_titlebar.h"
#include "venom/core/venom_memory.h"
#include "venom/graphics/venom_canvas.h"
#include <string.h>

/* ============================================================================
 * FORWARD DECLARATIONS
 * ============================================================================ */

static void titlebar_init(VenomWidget* widget);
static void titlebar_destroy(VenomWidget* widget);
static void titlebar_measure(VenomWidget* widget, VenomF32 available_width, VenomF32 available_height,
                              VenomF32* out_width, VenomF32* out_height);
static void titlebar_layout(VenomWidget* widget, VenomRectF bounds);
static void titlebar_draw(VenomWidget* widget, VenomCanvas* canvas);
static VenomBool titlebar_on_event(VenomWidget* widget, const VenomEvent* event);

/* ============================================================================
 * WIDGET CLASS
 * ============================================================================ */

const VenomWidgetClass venom_titlebar_class = {
    .class_name = "VenomTitleBar",
    .instance_size = sizeof(VenomTitleBar),
    .parent_class = &venom_widget_class,
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
    char* copy = venom_alloc(len + 1);
    if (copy) memcpy(copy, s, len + 1);
    return copy;
}

static void str_free(char* s) {
    if (s) venom_free(s, strlen(s) + 1);
}

static VenomI32 count_visible_buttons(VenomTitleBarButton buttons) {
    VenomI32 count = 0;
    if (buttons & VENOM_TITLEBAR_BUTTON_MINIMIZE) count++;
    if (buttons & VENOM_TITLEBAR_BUTTON_MAXIMIZE) count++;
    if (buttons & VENOM_TITLEBAR_BUTTON_CLOSE) count++;
    return count;
}

/* ============================================================================
 * LIFECYCLE
 * ============================================================================ */

static void titlebar_init(VenomWidget* widget) {
    VenomTitleBar* bar = (VenomTitleBar*)widget;
    
    bar->title = NULL;
    bar->icon = NULL;
    bar->left_widgets = NULL;
    bar->left_widget_count = 0;
    bar->right_widgets = NULL;
    bar->right_widget_count = 0;
    
    bar->visible_buttons = VENOM_TITLEBAR_BUTTON_ALL;
    bar->hover_button = -1;
    bar->pressed_button = -1;
    bar->is_maximized = VENOM_FALSE;
    
    bar->on_minimize = NULL;
    bar->on_maximize = NULL;
    bar->on_restore = NULL;
    bar->on_close = NULL;
    bar->on_drag = NULL;
    bar->user_data = NULL;
    
    bar->is_dragging = VENOM_FALSE;
    bar->drag_start_x = 0;
    bar->drag_start_y = 0;
    
    bar->background = venom_color_rgb(245, 245, 245);
    bar->title_color = venom_color_rgb(30, 30, 30);
    bar->button_hover = venom_color_rgba(0, 0, 0, 20);
    bar->button_pressed = venom_color_rgba(0, 0, 0, 40);
    bar->close_hover = venom_color_rgb(232, 17, 35);
    bar->close_pressed = venom_color_rgb(200, 15, 30);
    bar->icon_color = venom_color_rgb(50, 50, 50);
    bar->height = 32.0f;
    bar->button_width = 46.0f;
    bar->padding_h = 12.0f;
    bar->icon_padding = 8.0f;
    bar->title_font_size = 13.0f;
    bar->center_title = VENOM_FALSE;
    
    widget->layout.min_height = bar->height;
}

static void titlebar_destroy(VenomWidget* widget) {
    VenomTitleBar* bar = (VenomTitleBar*)widget;
    str_free(bar->title);
    
    if (bar->left_widgets) {
        venom_free(bar->left_widgets, bar->left_widget_count * sizeof(VenomWidget*));
    }
    if (bar->right_widgets) {
        venom_free(bar->right_widgets, bar->right_widget_count * sizeof(VenomWidget*));
    }
    
    venom_widget_class.destroy(widget);
}

VenomResultPtr venom_titlebar_create(const char* title) {
    VenomResultPtr result = venom_widget_create(&venom_titlebar_class);
    if (!result.ok) return result;
    
    VenomTitleBar* bar = (VenomTitleBar*)result.value;
    bar->title = str_dup(title);
    
    return result;
}

/* ============================================================================
 * CONFIGURATION
 * ============================================================================ */

void venom_titlebar_set_title(VenomTitleBar* bar, const char* title) {
    if (!bar) return;
    str_free(bar->title);
    bar->title = str_dup(title);
    venom_widget_invalidate(&bar->base);
}

const char* venom_titlebar_get_title(const VenomTitleBar* bar) {
    return bar ? bar->title : NULL;
}

void venom_titlebar_set_icon(VenomTitleBar* bar, VenomWidget* icon) {
    if (!bar) return;
    bar->icon = icon;
    venom_widget_invalidate(&bar->base);
}

void venom_titlebar_set_buttons(VenomTitleBar* bar, VenomTitleBarButton buttons) {
    if (!bar) return;
    bar->visible_buttons = buttons;
    venom_widget_invalidate(&bar->base);
}

void venom_titlebar_set_maximized(VenomTitleBar* bar, VenomBool maximized) {
    if (!bar) return;
    bar->is_maximized = maximized;
    venom_widget_invalidate(&bar->base);
}

VenomBool venom_titlebar_is_maximized(const VenomTitleBar* bar) {
    return bar ? bar->is_maximized : VENOM_FALSE;
}

/* ============================================================================
 * CUSTOM WIDGETS
 * ============================================================================ */

VenomResult venom_titlebar_add_left_widget(VenomTitleBar* bar, VenomWidget* widget) {
    if (!bar || !widget) {
        return VENOM_ERR_UNIT(VENOM_ERROR_INVALID_ARGUMENT);
    }
    
    VenomU32 new_count = bar->left_widget_count + 1;
    VenomWidget** new_widgets = venom_alloc(new_count * sizeof(VenomWidget*));
    if (!new_widgets) {
        return VENOM_ERR_UNIT(VENOM_ERROR_OUT_OF_MEMORY);
    }
    
    if (bar->left_widgets) {
        memcpy(new_widgets, bar->left_widgets, bar->left_widget_count * sizeof(VenomWidget*));
        venom_free(bar->left_widgets, bar->left_widget_count * sizeof(VenomWidget*));
    }
    
    new_widgets[bar->left_widget_count] = widget;
    bar->left_widgets = new_widgets;
    bar->left_widget_count = new_count;
    
    venom_widget_invalidate(&bar->base);
    return VENOM_OK_UNIT();
}

VenomResult venom_titlebar_add_right_widget(VenomTitleBar* bar, VenomWidget* widget) {
    if (!bar || !widget) {
        return VENOM_ERR_UNIT(VENOM_ERROR_INVALID_ARGUMENT);
    }
    
    VenomU32 new_count = bar->right_widget_count + 1;
    VenomWidget** new_widgets = venom_alloc(new_count * sizeof(VenomWidget*));
    if (!new_widgets) {
        return VENOM_ERR_UNIT(VENOM_ERROR_OUT_OF_MEMORY);
    }
    
    if (bar->right_widgets) {
        memcpy(new_widgets, bar->right_widgets, bar->right_widget_count * sizeof(VenomWidget*));
        venom_free(bar->right_widgets, bar->right_widget_count * sizeof(VenomWidget*));
    }
    
    new_widgets[bar->right_widget_count] = widget;
    bar->right_widgets = new_widgets;
    bar->right_widget_count = new_count;
    
    venom_widget_invalidate(&bar->base);
    return VENOM_OK_UNIT();
}

void venom_titlebar_clear_widgets(VenomTitleBar* bar) {
    if (!bar) return;
    
    if (bar->left_widgets) {
        venom_free(bar->left_widgets, bar->left_widget_count * sizeof(VenomWidget*));
        bar->left_widgets = NULL;
        bar->left_widget_count = 0;
    }
    
    if (bar->right_widgets) {
        venom_free(bar->right_widgets, bar->right_widget_count * sizeof(VenomWidget*));
        bar->right_widgets = NULL;
        bar->right_widget_count = 0;
    }
    
    venom_widget_invalidate(&bar->base);
}

/* ============================================================================
 * CALLBACKS
 * ============================================================================ */

void venom_titlebar_on_minimize(VenomTitleBar* bar, VenomTitleBarCallback callback, void* data) {
    if (!bar) return;
    bar->on_minimize = callback;
    bar->user_data = data;
}

void venom_titlebar_on_maximize(VenomTitleBar* bar, VenomTitleBarCallback callback, void* data) {
    if (!bar) return;
    bar->on_maximize = callback;
    bar->user_data = data;
}

void venom_titlebar_on_restore(VenomTitleBar* bar, VenomTitleBarCallback callback, void* data) {
    if (!bar) return;
    bar->on_restore = callback;
    bar->user_data = data;
}

void venom_titlebar_on_close(VenomTitleBar* bar, VenomTitleBarCallback callback, void* data) {
    if (!bar) return;
    bar->on_close = callback;
    bar->user_data = data;
}

void venom_titlebar_on_drag(VenomTitleBar* bar, VenomTitleBarDragCallback callback, void* data) {
    if (!bar) return;
    bar->on_drag = callback;
    bar->user_data = data;
}

/* ============================================================================
 * APPEARANCE
 * ============================================================================ */

void venom_titlebar_set_background(VenomTitleBar* bar, VenomColor color) {
    if (!bar) return;
    bar->background = color;
    venom_widget_invalidate(&bar->base);
}

void venom_titlebar_set_title_color(VenomTitleBar* bar, VenomColor color) {
    if (!bar) return;
    bar->title_color = color;
    venom_widget_invalidate(&bar->base);
}

void venom_titlebar_set_height(VenomTitleBar* bar, VenomF32 height) {
    if (!bar) return;
    bar->height = height;
    bar->base.layout.min_height = height;
    venom_widget_invalidate(&bar->base);
}

void venom_titlebar_set_center_title(VenomTitleBar* bar, VenomBool centered) {
    if (!bar) return;
    bar->center_title = centered;
    venom_widget_invalidate(&bar->base);
}

void venom_titlebar_set_button_width(VenomTitleBar* bar, VenomF32 width) {
    if (!bar) return;
    bar->button_width = width;
    venom_widget_invalidate(&bar->base);
}

/* ============================================================================
 * THEMES
 * ============================================================================ */

void venom_titlebar_apply_light_theme(VenomTitleBar* bar) {
    if (!bar) return;
    bar->background = venom_color_rgb(245, 245, 245);
    bar->title_color = venom_color_rgb(30, 30, 30);
    bar->button_hover = venom_color_rgba(0, 0, 0, 20);
    bar->button_pressed = venom_color_rgba(0, 0, 0, 40);
    bar->close_hover = venom_color_rgb(232, 17, 35);
    bar->close_pressed = venom_color_rgb(200, 15, 30);
    bar->icon_color = venom_color_rgb(50, 50, 50);
    venom_widget_invalidate(&bar->base);
}

void venom_titlebar_apply_dark_theme(VenomTitleBar* bar) {
    if (!bar) return;
    bar->background = venom_color_rgb(32, 32, 32);
    bar->title_color = venom_color_rgb(255, 255, 255);
    bar->button_hover = venom_color_rgba(255, 255, 255, 25);
    bar->button_pressed = venom_color_rgba(255, 255, 255, 40);
    bar->close_hover = venom_color_rgb(232, 17, 35);
    bar->close_pressed = venom_color_rgb(200, 15, 30);
    bar->icon_color = venom_color_rgb(255, 255, 255);
    venom_widget_invalidate(&bar->base);
}

/* ============================================================================
 * WIDGET CALLBACKS
 * ============================================================================ */

static void titlebar_measure(VenomWidget* widget, VenomF32 available_width, 
                              VenomF32 available_height, VenomF32* out_width, VenomF32* out_height) {
    VenomTitleBar* bar = (VenomTitleBar*)widget;
    (void)available_height;
    
    *out_width = available_width > 0 ? available_width : 400.0f;
    *out_height = bar->height;
}

static void titlebar_layout(VenomWidget* widget, VenomRectF bounds) {
    widget->bounds = bounds;
}

static void draw_minimize_icon(VenomCanvas* canvas, VenomF32 cx, VenomF32 cy, VenomColor color) {
    VenomPaint paint = venom_paint_stroke(color, 1.0f);
    venom_canvas_draw_line(canvas, cx - 5, cy, cx + 5, cy, &paint);
}

static void draw_maximize_icon(VenomCanvas* canvas, VenomF32 cx, VenomF32 cy,
                                VenomColor color, VenomBool is_maximized) {
    VenomPaint paint = venom_paint_stroke(color, 1.0f);
    
    if (is_maximized) {
        VenomRectF rect1 = { cx - 3, cy - 5, 8, 8 };
        VenomRectF rect2 = { cx - 5, cy - 3, 8, 8 };
        venom_canvas_draw_rect(canvas, rect1, &paint);
        venom_canvas_draw_rect(canvas, rect2, &paint);
    } else {
        VenomRectF rect = { cx - 5, cy - 5, 10, 10 };
        venom_canvas_draw_rect(canvas, rect, &paint);
    }
}

static void draw_close_icon(VenomCanvas* canvas, VenomF32 cx, VenomF32 cy, VenomColor color) {
    VenomPaint paint = venom_paint_stroke(color, 1.2f);
    venom_canvas_draw_line(canvas, cx - 5, cy - 5, cx + 5, cy + 5, &paint);
    venom_canvas_draw_line(canvas, cx + 5, cy - 5, cx - 5, cy + 5, &paint);
}

static void titlebar_draw(VenomWidget* widget, VenomCanvas* canvas) {
    VenomTitleBar* bar = (VenomTitleBar*)widget;
    VenomRectF bounds = widget->bounds;
    
    VenomPaint bg_paint = venom_paint_fill(bar->background);
    venom_canvas_draw_rect(canvas, bounds, &bg_paint);
    
    VenomPaint border_paint = venom_paint_stroke(venom_color_rgba(0, 0, 0, 25), 1.0f);
    venom_canvas_draw_line(canvas, bounds.x, bounds.y + bounds.height - 0.5f,
                            bounds.x + bounds.width, bounds.y + bounds.height - 0.5f,
                            &border_paint);
    
    VenomI32 button_count = count_visible_buttons(bar->visible_buttons);
    VenomF32 buttons_width = button_count * bar->button_width;
    
    VenomF32 title_x;
    if (bar->center_title) {
        VenomF32 title_width = bar->title ? (VenomF32)strlen(bar->title) * bar->title_font_size * 0.6f : 0;
        title_x = bounds.x + (bounds.width - title_width) / 2;
    } else {
        title_x = bounds.x + bar->padding_h;
        if (bar->icon) {
            title_x += 20 + bar->icon_padding;
        }
    }
    
    if (bar->title) {
        VenomPaint title_paint = venom_paint_fill(bar->title_color);
        VenomF32 title_y = bounds.y + bounds.height / 2 + bar->title_font_size * 0.35f;
        venom_canvas_draw_text(canvas, bar->title, title_x, title_y, NULL, &title_paint);
    }
    
    VenomF32 btn_x = bounds.x + bounds.width - buttons_width;
    VenomF32 btn_y = bounds.y;
    VenomI32 btn_index = 0;
    
    if (bar->visible_buttons & VENOM_TITLEBAR_BUTTON_MINIMIZE) {
        VenomRectF btn_rect = { btn_x, btn_y, bar->button_width, bar->height };
        
        if (bar->pressed_button == btn_index) {
            VenomPaint pressed = venom_paint_fill(bar->button_pressed);
            venom_canvas_draw_rect(canvas, btn_rect, &pressed);
        } else if (bar->hover_button == btn_index) {
            VenomPaint hover = venom_paint_fill(bar->button_hover);
            venom_canvas_draw_rect(canvas, btn_rect, &hover);
        }
        
        draw_minimize_icon(canvas, btn_x + bar->button_width / 2, 
                           btn_y + bar->height / 2, bar->icon_color);
        
        btn_x += bar->button_width;
        btn_index++;
    }
    
    if (bar->visible_buttons & VENOM_TITLEBAR_BUTTON_MAXIMIZE) {
        VenomRectF btn_rect = { btn_x, btn_y, bar->button_width, bar->height };
        
        if (bar->pressed_button == btn_index) {
            VenomPaint pressed = venom_paint_fill(bar->button_pressed);
            venom_canvas_draw_rect(canvas, btn_rect, &pressed);
        } else if (bar->hover_button == btn_index) {
            VenomPaint hover = venom_paint_fill(bar->button_hover);
            venom_canvas_draw_rect(canvas, btn_rect, &hover);
        }
        
        draw_maximize_icon(canvas, btn_x + bar->button_width / 2,
                           btn_y + bar->height / 2, bar->icon_color, bar->is_maximized);
        
        btn_x += bar->button_width;
        btn_index++;
    }
    
    if (bar->visible_buttons & VENOM_TITLEBAR_BUTTON_CLOSE) {
        VenomRectF btn_rect = { btn_x, btn_y, bar->button_width, bar->height };
        
        VenomColor icon_color = bar->icon_color;
        if (bar->pressed_button == btn_index) {
            VenomPaint pressed = venom_paint_fill(bar->close_pressed);
            venom_canvas_draw_rect(canvas, btn_rect, &pressed);
            icon_color = venom_color_rgb(255, 255, 255);
        } else if (bar->hover_button == btn_index) {
            VenomPaint hover = venom_paint_fill(bar->close_hover);
            venom_canvas_draw_rect(canvas, btn_rect, &hover);
            icon_color = venom_color_rgb(255, 255, 255);
        }
        
        draw_close_icon(canvas, btn_x + bar->button_width / 2,
                        btn_y + bar->height / 2, icon_color);
    }
}

static VenomI32 titlebar_hit_test_button(VenomTitleBar* bar, VenomF32 x) {
    VenomRectF bounds = bar->base.bounds;
    VenomI32 button_count = count_visible_buttons(bar->visible_buttons);
    VenomF32 buttons_width = button_count * bar->button_width;
    VenomF32 buttons_start = bounds.x + bounds.width - buttons_width;
    
    if (x < buttons_start) return -1;
    
    VenomF32 rel_x = x - buttons_start;
    VenomI32 index = (VenomI32)(rel_x / bar->button_width);
    
    if (index >= 0 && index < button_count) {
        return index;
    }
    return -1;
}

static VenomBool titlebar_on_event(VenomWidget* widget, const VenomEvent* event) {
    VenomTitleBar* bar = (VenomTitleBar*)widget;
    
    switch (event->type) {
        case VENOM_EVENT_MOUSE_MOVE: {
            VenomI32 button = titlebar_hit_test_button(bar, (VenomF32)event->mouse.x);
            
            if (bar->is_dragging && bar->on_drag) {
                VenomI32 dx = event->mouse.x - bar->drag_start_x;
                VenomI32 dy = event->mouse.y - bar->drag_start_y;
                bar->on_drag(bar, dx, dy, bar->user_data);
                bar->drag_start_x = event->mouse.x;
                bar->drag_start_y = event->mouse.y;
                return VENOM_TRUE;
            }
            
            if (button != bar->hover_button) {
                bar->hover_button = button;
                venom_widget_invalidate(widget);
            }
            return VENOM_TRUE;
        }
        
        case VENOM_EVENT_MOUSE_LEAVE:
            bar->hover_button = -1;
            bar->pressed_button = -1;
            bar->is_dragging = VENOM_FALSE;
            venom_widget_invalidate(widget);
            return VENOM_TRUE;
            
        case VENOM_EVENT_MOUSE_BUTTON_DOWN: {
            if (event->mouse.button != VENOM_MOUSE_BUTTON_LEFT) return VENOM_FALSE;
            
            VenomI32 button = titlebar_hit_test_button(bar, (VenomF32)event->mouse.x);
            if (button >= 0) {
                bar->pressed_button = button;
                venom_widget_invalidate(widget);
                return VENOM_TRUE;
            }
            
            bar->is_dragging = VENOM_TRUE;
            bar->drag_start_x = event->mouse.x;
            bar->drag_start_y = event->mouse.y;
            return VENOM_TRUE;
        }
        
        case VENOM_EVENT_MOUSE_BUTTON_UP: {
            if (event->mouse.button != VENOM_MOUSE_BUTTON_LEFT) return VENOM_FALSE;
            
            VenomI32 pressed = bar->pressed_button;
            bar->pressed_button = -1;
            bar->is_dragging = VENOM_FALSE;
            
            if (pressed >= 0) {
                VenomI32 button = titlebar_hit_test_button(bar, (VenomF32)event->mouse.x);
                if (button == pressed) {
                    VenomI32 idx = 0;
                    if (bar->visible_buttons & VENOM_TITLEBAR_BUTTON_MINIMIZE) {
                        if (pressed == idx && bar->on_minimize) {
                            bar->on_minimize(bar, bar->user_data);
                        }
                        idx++;
                    }
                    if (bar->visible_buttons & VENOM_TITLEBAR_BUTTON_MAXIMIZE) {
                        if (pressed == idx) {
                            if (bar->is_maximized && bar->on_restore) {
                                bar->on_restore(bar, bar->user_data);
                            } else if (!bar->is_maximized && bar->on_maximize) {
                                bar->on_maximize(bar, bar->user_data);
                            }
                        }
                        idx++;
                    }
                    if (bar->visible_buttons & VENOM_TITLEBAR_BUTTON_CLOSE) {
                        if (pressed == idx && bar->on_close) {
                            bar->on_close(bar, bar->user_data);
                        }
                    }
                }
                venom_widget_invalidate(widget);
                return VENOM_TRUE;
            }
            return VENOM_FALSE;
        }
        
        default:
            return VENOM_FALSE;
    }
}
