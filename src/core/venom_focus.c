/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_focus.c - Input focus management implementation
 */

#include "venom/core/venom_focus.h"
#include <stdio.h>

/* ============================================================================
 * FOCUS STATE
 * ============================================================================ */

static struct {
    VenomWidget* focused;       /* Currently focused widget */
    VenomWidget* root;          /* Root for traversal */
    VenomBool initialized;
} g_focus = {0};

/* ============================================================================
 * FOCUS TRAVERSAL HELPERS
 * ============================================================================ */

static void collect_focusable(VenomWidget* widget, VenomWidget** list, 
                               VenomU32* count, VenomU32 max_count) {
    if (!widget || *count >= max_count) return;
    
    /* Add this widget if focusable */
    if (venom_widget_is_focusable(widget) && widget->visible) {
        list[(*count)++] = widget;
    }
    
    /* Recurse to children */
    for (VenomU32 i = 0; i < widget->children_count; i++) {
        collect_focusable(widget->children[i], list, count, max_count);
    }
}

/* ============================================================================
 * PUBLIC API
 * ============================================================================ */

void venom_focus_init(void) {
    g_focus.focused = NULL;
    g_focus.root = NULL;
    g_focus.initialized = VENOM_TRUE;
}

void venom_focus_shutdown(void) {
    g_focus.focused = NULL;
    g_focus.root = NULL;
    g_focus.initialized = VENOM_FALSE;
}

void venom_focus_set(VenomWidget* widget) {
    if (!g_focus.initialized) return;
    
    /* Nothing to do if same widget */
    if (g_focus.focused == widget) return;
    
    /* Remove focus from old widget */
    if (g_focus.focused) {
        venom_widget_remove_state(g_focus.focused, VENOM_WIDGET_STATE_FOCUSED);
        g_focus.focused->needs_redraw = VENOM_TRUE;
    }
    
    /* Set focus to new widget */
    g_focus.focused = widget;
    
    if (widget) {
        venom_widget_add_state(widget, VENOM_WIDGET_STATE_FOCUSED);
        widget->needs_redraw = VENOM_TRUE;
    }
}

VenomWidget* venom_focus_get(void) {
    return g_focus.focused;
}

void venom_focus_next(void) {
    if (!g_focus.root) return;
    
    /* Collect all focusable widgets */
    VenomWidget* list[256];
    VenomU32 count = 0;
    collect_focusable(g_focus.root, list, &count, 256);
    
    if (count == 0) return;
    
    /* Find current index */
    VenomU32 current = 0;
    for (VenomU32 i = 0; i < count; i++) {
        if (list[i] == g_focus.focused) {
            current = i;
            break;
        }
    }
    
    /* Move to next (wrap around) */
    VenomU32 next = (current + 1) % count;
    venom_focus_set(list[next]);
}

void venom_focus_prev(void) {
    if (!g_focus.root) return;
    
    /* Collect all focusable widgets */
    VenomWidget* list[256];
    VenomU32 count = 0;
    collect_focusable(g_focus.root, list, &count, 256);
    
    if (count == 0) return;
    
    /* Find current index */
    VenomU32 current = 0;
    for (VenomU32 i = 0; i < count; i++) {
        if (list[i] == g_focus.focused) {
            current = i;
            break;
        }
    }
    
    /* Move to previous (wrap around) */
    VenomU32 prev = (current == 0) ? count - 1 : current - 1;
    venom_focus_set(list[prev]);
}

void venom_focus_clear(void) {
    venom_focus_set(NULL);
}

VenomBool venom_focus_has(const VenomWidget* widget) {
    return widget && g_focus.focused == widget;
}

void venom_focus_set_root(VenomWidget* root) {
    g_focus.root = root;
    
    /* If current focused widget is no longer in tree, clear focus */
    if (g_focus.focused) {
        VenomWidget* list[256];
        VenomU32 count = 0;
        collect_focusable(root, list, &count, 256);
        
        VenomBool found = VENOM_FALSE;
        for (VenomU32 i = 0; i < count; i++) {
            if (list[i] == g_focus.focused) {
                found = VENOM_TRUE;
                break;
            }
        }
        
        if (!found) {
            venom_focus_clear();
        }
    }
}

VenomU32 venom_focus_get_focusable_list(VenomWidget** widgets, VenomU32 max_count) {
    VenomU32 count = 0;
    if (g_focus.root) {
        collect_focusable(g_focus.root, widgets, &count, max_count);
    }
    return count;
}

/* ============================================================================
 * WIDGET HELPERS
 * ============================================================================ */

void venom_widget_set_focusable(VenomWidget* widget, VenomBool focusable) {
    if (!widget) return;
    widget->focusable = focusable;
}

VenomBool venom_widget_is_focusable(const VenomWidget* widget) {
    return widget ? widget->focusable : VENOM_FALSE;
}

VenomBool venom_widget_request_focus(VenomWidget* widget) {
    if (!widget || !widget->focusable || !widget->visible) {
        return VENOM_FALSE;
    }
    venom_focus_set(widget);
    return VENOM_TRUE;
}

VenomBool venom_widget_has_focus(const VenomWidget* widget) {
    return venom_focus_has(widget);
}
