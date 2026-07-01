/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_focus.c - Input focus management implementation
 */

#include "vaxp/core/vaxp_focus.h"
#include <stdio.h>

/* ============================================================================
 * FOCUS STATE
 * ============================================================================ */

static struct {
    VaxpWidget* focused;       /* Currently focused widget */
    VaxpWidget* root;          /* Root for traversal */
    VaxpBool initialized;
} g_focus = {0};

/* ============================================================================
 * FOCUS TRAVERSAL HELPERS
 * ============================================================================ */

static void collect_focusable(VaxpWidget* widget, VaxpWidget** list, 
                               VaxpU32* count, VaxpU32 max_count) {
    if (!widget || *count >= max_count) return;
    
    /* Add this widget if focusable */
    if (vaxp_widget_is_focusable(widget) && widget->visible) {
        list[(*count)++] = widget;
    }
    
    /* Recurse to children */
    for (VaxpU32 i = 0; i < widget->children_count; i++) {
        collect_focusable(widget->children[i], list, count, max_count);
    }
}

/* ============================================================================
 * PUBLIC API
 * ============================================================================ */

void vaxp_focus_init(void) {
    g_focus.focused = NULL;
    g_focus.root = NULL;
    g_focus.initialized = VAXP_TRUE;
}

void vaxp_focus_shutdown(void) {
    g_focus.focused = NULL;
    g_focus.root = NULL;
    g_focus.initialized = VAXP_FALSE;
}

void vaxp_focus_set(VaxpWidget* widget) {
    if (!g_focus.initialized) return;
    
    /* Nothing to do if same widget */
    if (g_focus.focused == widget) return;
    
    /* Remove focus from old widget */
    if (g_focus.focused) {
        vaxp_widget_remove_state(g_focus.focused, VAXP_WIDGET_STATE_FOCUSED);
        g_focus.focused->needs_redraw = VAXP_TRUE;
    }
    
    /* Set focus to new widget */
    g_focus.focused = widget;
    
    if (widget) {
        vaxp_widget_add_state(widget, VAXP_WIDGET_STATE_FOCUSED);
        widget->needs_redraw = VAXP_TRUE;
    }
}

VaxpWidget* vaxp_focus_get(void) {
    return g_focus.focused;
}

void vaxp_focus_next(void) {
    if (!g_focus.root) return;
    
    /* Collect all focusable widgets */
    VaxpWidget* list[256];
    VaxpU32 count = 0;
    collect_focusable(g_focus.root, list, &count, 256);
    
    if (count == 0) return;
    
    /* Find current index */
    VaxpU32 current = 0;
    for (VaxpU32 i = 0; i < count; i++) {
        if (list[i] == g_focus.focused) {
            current = i;
            break;
        }
    }
    
    /* Move to next (wrap around) */
    VaxpU32 next = (current + 1) % count;
    vaxp_focus_set(list[next]);
}

void vaxp_focus_prev(void) {
    if (!g_focus.root) return;
    
    /* Collect all focusable widgets */
    VaxpWidget* list[256];
    VaxpU32 count = 0;
    collect_focusable(g_focus.root, list, &count, 256);
    
    if (count == 0) return;
    
    /* Find current index */
    VaxpU32 current = 0;
    for (VaxpU32 i = 0; i < count; i++) {
        if (list[i] == g_focus.focused) {
            current = i;
            break;
        }
    }
    
    /* Move to previous (wrap around) */
    VaxpU32 prev = (current == 0) ? count - 1 : current - 1;
    vaxp_focus_set(list[prev]);
}

void vaxp_focus_clear(void) {
    vaxp_focus_set(NULL);
}

VaxpBool vaxp_focus_has(const VaxpWidget* widget) {
    return widget && g_focus.focused == widget;
}

void vaxp_focus_set_root(VaxpWidget* root) {
    g_focus.root = root;
    
    /* If current focused widget is no longer in tree, clear focus */
    if (g_focus.focused) {
        VaxpWidget* list[256];
        VaxpU32 count = 0;
        collect_focusable(root, list, &count, 256);
        
        VaxpBool found = VAXP_FALSE;
        for (VaxpU32 i = 0; i < count; i++) {
            if (list[i] == g_focus.focused) {
                found = VAXP_TRUE;
                break;
            }
        }
        
        if (!found) {
            vaxp_focus_clear();
        }
    }
}

VaxpU32 vaxp_focus_get_focusable_list(VaxpWidget** widgets, VaxpU32 max_count) {
    VaxpU32 count = 0;
    if (g_focus.root) {
        collect_focusable(g_focus.root, widgets, &count, max_count);
    }
    return count;
}

/* ============================================================================
 * WIDGET HELPERS
 * ============================================================================ */

void vaxp_widget_set_focusable(VaxpWidget* widget, VaxpBool focusable) {
    if (!widget) return;
    widget->focusable = focusable;
}

VaxpBool vaxp_widget_is_focusable(const VaxpWidget* widget) {
    return widget ? widget->focusable : VAXP_FALSE;
}

VaxpBool vaxp_widget_request_focus(VaxpWidget* widget) {
    if (!widget || !widget->focusable || !widget->visible) {
        return VAXP_FALSE;
    }
    vaxp_focus_set(widget);
    return VAXP_TRUE;
}

VaxpBool vaxp_widget_has_focus(const VaxpWidget* widget) {
    return vaxp_focus_has(widget);
}
