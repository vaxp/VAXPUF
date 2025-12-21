/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_tree_view.c - Tree view implementation
 */

#include "venom/widgets/venom_tree_view.h"
#include "venom/core/venom_memory.h"
#include <string.h>

#define DEFAULT_NODE_HEIGHT 32.0f
#define DEFAULT_INDENT 20.0f

static void tree_view_init(VenomWidget* widget) {
    VenomTreeView* tree = (VenomTreeView*)widget;
    
    tree->root = NULL;
    tree->selected = NULL;
    tree->node_builder = NULL;
    tree->builder_data = NULL;
    tree->on_select = NULL;
    tree->select_data = NULL;
    
    tree->node_height = DEFAULT_NODE_HEIGHT;
    tree->indent = DEFAULT_INDENT;
    tree->scroll_offset = 0;
    tree->hover_index = -1;
    
    tree->selection_color = (VenomColor){ 63, 81, 181, 50 };
    tree->hover_color = (VenomColor){ 0, 0, 0, 20 };
    tree->expand_color = (VenomColor){ 97, 97, 97, 255 };
    
    widget->focusable = VENOM_TRUE;
}

static void tree_node_free(VenomTreeNode* node) {
    if (!node) return;
    
    VenomTreeNode* child = node->children;
    while (child) {
        VenomTreeNode* next = child->next;
        tree_node_free(child);
        child = next;
    }
    
    if (node->label) venom_free(node->label, strlen(node->label) + 1);
    venom_free(node, sizeof(VenomTreeNode));
}

static void tree_view_destroy(VenomWidget* widget) {
    VenomTreeView* tree = (VenomTreeView*)widget;
    tree_node_free(tree->root);
    venom_widget_class.destroy(widget);
}

static VenomU32 count_visible_nodes(VenomTreeNode* node) {
    VenomU32 count = 1;
    if (node->expanded) {
        VenomTreeNode* child = node->children;
        while (child) {
            count += count_visible_nodes(child);
            child = child->next;
        }
    }
    return count;
}

static void tree_view_measure(VenomWidget* widget, VenomF32 available_width, VenomF32 available_height,
                              VenomF32* out_width, VenomF32* out_height) {
    VenomTreeView* tree = (VenomTreeView*)widget;
    (void)available_height;
    
    *out_width = available_width;
    
    VenomU32 visible = 0;
    if (tree->root) visible = count_visible_nodes(tree->root);
    *out_height = visible * tree->node_height;
}

static void draw_node(VenomTreeView* tree, VenomCanvas* canvas, VenomTreeNode* node,
                      VenomF32* y, VenomI32* index, VenomF32 w) {
    VenomF32 x = node->depth * tree->indent;
    VenomRectF row = { 0, *y, w, tree->node_height };
    
    /* Hover highlight */
    if (*index == tree->hover_index) {
        VenomPaint hover_paint = venom_paint_fill(tree->hover_color);
        venom_canvas_draw_rect(canvas, row, &hover_paint);
    }
    
    /* Selection */
    if (node->selected) {
        VenomPaint sel_paint = venom_paint_fill(tree->selection_color);
        venom_canvas_draw_rect(canvas, row, &sel_paint);
    }
    
    /* Expand indicator */
    if (node->child_count > 0) {
        const char* icon = node->expanded ? "▼" : "▶";
        VenomPaint exp_paint = venom_paint_fill(tree->expand_color);
        venom_canvas_draw_text(canvas, icon, x + 4, *y + tree->node_height / 2 + 4, NULL, &exp_paint);
    }
    
    /* Label */
    if (node->label) {
        VenomPaint text_paint = venom_paint_fill((VenomColor){ 33, 33, 33, 255 });
        venom_canvas_draw_text(canvas, node->label, x + 20, *y + tree->node_height / 2 + 4, NULL, &text_paint);
    }
    
    *y += tree->node_height;
    (*index)++;
    
    /* Draw children */
    if (node->expanded) {
        VenomTreeNode* child = node->children;
        while (child) {
            draw_node(tree, canvas, child, y, index, w);
            child = child->next;
        }
    }
}

static void tree_view_draw(VenomWidget* widget, VenomCanvas* canvas) {
    VenomTreeView* tree = (VenomTreeView*)widget;
    
    if (!tree->root) return;
    
    VenomF32 y = -tree->scroll_offset;
    VenomI32 index = 0;
    draw_node(tree, canvas, tree->root, &y, &index, widget->bounds.width);
}

static VenomTreeNode* find_node_at_index(VenomTreeNode* node, VenomI32* current, VenomI32 target) {
    if (*current == target) return node;
    (*current)++;
    
    if (node->expanded) {
        VenomTreeNode* child = node->children;
        while (child) {
            VenomTreeNode* found = find_node_at_index(child, current, target);
            if (found) return found;
            child = child->next;
        }
    }
    return NULL;
}

static VenomBool tree_view_on_event(VenomWidget* widget, const VenomEvent* event) {
    VenomTreeView* tree = (VenomTreeView*)widget;
    
    if (!tree->root) return VENOM_FALSE;
    
    switch (event->type) {
        case VENOM_EVENT_MOUSE_MOVE: {
            VenomF32 my = (VenomF32)event->mouse.y + tree->scroll_offset;
            VenomI32 new_hover = (VenomI32)(my / tree->node_height);
            if (new_hover != tree->hover_index) {
                tree->hover_index = new_hover;
                widget->needs_redraw = VENOM_TRUE;
            }
            break;
        }
        
        case VENOM_EVENT_MOUSE_BUTTON_DOWN:
            if (event->mouse.button == VENOM_MOUSE_BUTTON_LEFT && tree->hover_index >= 0) {
                VenomI32 current = 0;
                VenomTreeNode* node = find_node_at_index(tree->root, &current, tree->hover_index);
                if (node) {
                    VenomF32 x_offset = node->depth * tree->indent;
                    
                    /* Click on expand icon? */
                    if (event->mouse.x < x_offset + 20 && node->child_count > 0) {
                        node->expanded = !node->expanded;
                    } else {
                        /* Selection */
                        if (tree->selected) tree->selected->selected = VENOM_FALSE;
                        node->selected = VENOM_TRUE;
                        tree->selected = node;
                        
                        if (tree->on_select) {
                            tree->on_select(tree, node, tree->select_data);
                        }
                    }
                    widget->needs_redraw = VENOM_TRUE;
                }
                return VENOM_TRUE;
            }
            break;
            
        default:
            break;
    }
    
    return VENOM_FALSE;
}

const VenomWidgetClass venom_tree_view_class = {
    .class_name = "VenomTreeView",
    .instance_size = sizeof(VenomTreeView),
    .parent_class = &venom_widget_class,
    .init = tree_view_init,
    .destroy = tree_view_destroy,
    .measure = tree_view_measure,
    .layout = NULL,
    .draw = tree_view_draw,
    .on_event = tree_view_on_event,
    .on_state_changed = NULL,
};

VenomResultPtr venom_tree_view_create(void) {
    return venom_widget_create(&venom_tree_view_class);
}

VenomTreeNode* venom_tree_node_create(const char* label, void* data) {
    VenomTreeNode* node = (VenomTreeNode*)venom_alloc(sizeof(VenomTreeNode));
    if (!node) return NULL;
    
    memset(node, 0, sizeof(VenomTreeNode));
    
    if (label) {
        VenomSize len = strlen(label) + 1;
        node->label = (char*)venom_alloc(len);
        if (node->label) memcpy(node->label, label, len);
    }
    
    node->data = data;
    node->expanded = VENOM_FALSE;
    node->selectable = VENOM_TRUE;
    
    return node;
}

VenomResult venom_tree_view_add_root(VenomTreeView* tree, VenomTreeNode* node) {
    VENOM_ENSURE_NOT_NULL(tree);
    VENOM_ENSURE_NOT_NULL(node);
    
    tree->root = node;
    node->depth = 0;
    node->expanded = VENOM_TRUE;
    
    return VENOM_OK_UNIT();
}

VenomResult venom_tree_node_add_child(VenomTreeNode* parent, VenomTreeNode* child) {
    VENOM_ENSURE_NOT_NULL(parent);
    VENOM_ENSURE_NOT_NULL(child);
    
    child->parent = parent;
    child->depth = parent->depth + 1;
    
    if (!parent->children) {
        parent->children = child;
    } else {
        VenomTreeNode* last = parent->children;
        while (last->next) last = last->next;
        last->next = child;
    }
    
    parent->child_count++;
    return VENOM_OK_UNIT();
}

static void expand_recursive(VenomTreeNode* node, VenomBool expand) {
    node->expanded = expand;
    VenomTreeNode* child = node->children;
    while (child) {
        expand_recursive(child, expand);
        child = child->next;
    }
}

void venom_tree_view_expand_all(VenomTreeView* tree) {
    if (tree && tree->root) {
        expand_recursive(tree->root, VENOM_TRUE);
        venom_widget_invalidate((VenomWidget*)tree);
    }
}

void venom_tree_view_collapse_all(VenomTreeView* tree) {
    if (tree && tree->root) {
        expand_recursive(tree->root, VENOM_FALSE);
        tree->root->expanded = VENOM_TRUE;
        venom_widget_invalidate((VenomWidget*)tree);
    }
}

void venom_tree_view_set_on_select(VenomTreeView* tree, VenomTreeSelectCallback callback, void* data) {
    if (tree) {
        tree->on_select = callback;
        tree->select_data = data;
    }
}
