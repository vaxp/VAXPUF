/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_tree_view.c - Tree view implementation
 */

#include "vaxp/widgets/vaxp_tree_view.h"
#include "vaxp/core/vaxp_memory.h"
#include <string.h>

#define DEFAULT_NODE_HEIGHT 32.0f
#define DEFAULT_INDENT 20.0f

static void tree_view_init(VaxpWidget* widget) {
    VaxpTreeView* tree = (VaxpTreeView*)widget;
    
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
    
    tree->selection_color = (VaxpColor){ 63, 81, 181, 50 };
    tree->hover_color = (VaxpColor){ 0, 0, 0, 20 };
    tree->expand_color = (VaxpColor){ 97, 97, 97, 255 };
    
    widget->focusable = VAXP_TRUE;
}

static void tree_node_free(VaxpTreeNode* node) {
    if (!node) return;
    
    VaxpTreeNode* child = node->children;
    while (child) {
        VaxpTreeNode* next = child->next;
        tree_node_free(child);
        child = next;
    }
    
    if (node->label) vaxp_free(node->label, strlen(node->label) + 1);
    vaxp_free(node, sizeof(VaxpTreeNode));
}

static void tree_view_destroy(VaxpWidget* widget) {
    VaxpTreeView* tree = (VaxpTreeView*)widget;
    tree_node_free(tree->root);
    vaxp_widget_class.destroy(widget);
}

static VaxpU32 count_visible_nodes(VaxpTreeNode* node) {
    VaxpU32 count = 1;
    if (node->expanded) {
        VaxpTreeNode* child = node->children;
        while (child) {
            count += count_visible_nodes(child);
            child = child->next;
        }
    }
    return count;
}

static void tree_view_measure(VaxpWidget* widget, VaxpF32 available_width, VaxpF32 available_height,
                              VaxpF32* out_width, VaxpF32* out_height) {
    VaxpTreeView* tree = (VaxpTreeView*)widget;
    (void)available_height;
    
    *out_width = available_width;
    
    VaxpU32 visible = 0;
    if (tree->root) visible = count_visible_nodes(tree->root);
    *out_height = visible * tree->node_height;
}

static void draw_node(VaxpTreeView* tree, VaxpCanvas* canvas, VaxpTreeNode* node,
                      VaxpF32* y, VaxpI32* index, VaxpF32 w) {
    VaxpF32 x = node->depth * tree->indent;
    VaxpRectF row = { 0, *y, w, tree->node_height };
    
    /* Hover highlight */
    if (*index == tree->hover_index) {
        VaxpPaint hover_paint = vaxp_paint_fill(tree->hover_color);
        vaxp_canvas_draw_rect(canvas, row, &hover_paint);
    }
    
    /* Selection */
    if (node->selected) {
        VaxpPaint sel_paint = vaxp_paint_fill(tree->selection_color);
        vaxp_canvas_draw_rect(canvas, row, &sel_paint);
    }
    
    /* Expand indicator */
    if (node->child_count > 0) {
        const char* icon = node->expanded ? "▼" : "▶";
        VaxpPaint exp_paint = vaxp_paint_fill(tree->expand_color);
        vaxp_canvas_draw_text(canvas, icon, x + 4, *y + tree->node_height / 2 + 4, NULL, &exp_paint);
    }
    
    /* Label */
    if (node->label) {
        VaxpPaint text_paint = vaxp_paint_fill((VaxpColor){ 33, 33, 33, 255 });
        vaxp_canvas_draw_text(canvas, node->label, x + 20, *y + tree->node_height / 2 + 4, NULL, &text_paint);
    }
    
    *y += tree->node_height;
    (*index)++;
    
    /* Draw children */
    if (node->expanded) {
        VaxpTreeNode* child = node->children;
        while (child) {
            draw_node(tree, canvas, child, y, index, w);
            child = child->next;
        }
    }
}

static void tree_view_draw(VaxpWidget* widget, VaxpCanvas* canvas) {
    VaxpTreeView* tree = (VaxpTreeView*)widget;
    
    if (!tree->root) return;
    
    VaxpF32 y = -tree->scroll_offset;
    VaxpI32 index = 0;
    draw_node(tree, canvas, tree->root, &y, &index, widget->bounds.width);
}

static VaxpTreeNode* find_node_at_index(VaxpTreeNode* node, VaxpI32* current, VaxpI32 target) {
    if (*current == target) return node;
    (*current)++;
    
    if (node->expanded) {
        VaxpTreeNode* child = node->children;
        while (child) {
            VaxpTreeNode* found = find_node_at_index(child, current, target);
            if (found) return found;
            child = child->next;
        }
    }
    return NULL;
}

static VaxpBool tree_view_on_event(VaxpWidget* widget, const VaxpEvent* event) {
    VaxpTreeView* tree = (VaxpTreeView*)widget;
    
    if (!tree->root) return VAXP_FALSE;
    
    switch (event->type) {
        case VAXP_EVENT_MOUSE_MOVE: {
            VaxpF32 my = (VaxpF32)event->mouse.y + tree->scroll_offset;
            VaxpI32 new_hover = (VaxpI32)(my / tree->node_height);
            if (new_hover != tree->hover_index) {
                tree->hover_index = new_hover;
                widget->needs_redraw = VAXP_TRUE;
            }
            break;
        }
        
        case VAXP_EVENT_MOUSE_BUTTON_DOWN:
            if (event->mouse.button == VAXP_MOUSE_BUTTON_LEFT && tree->hover_index >= 0) {
                VaxpI32 current = 0;
                VaxpTreeNode* node = find_node_at_index(tree->root, &current, tree->hover_index);
                if (node) {
                    VaxpF32 x_offset = node->depth * tree->indent;
                    
                    /* Click on expand icon? */
                    if (event->mouse.x < x_offset + 20 && node->child_count > 0) {
                        node->expanded = !node->expanded;
                    } else {
                        /* Selection */
                        if (tree->selected) tree->selected->selected = VAXP_FALSE;
                        node->selected = VAXP_TRUE;
                        tree->selected = node;
                        
                        if (tree->on_select) {
                            tree->on_select(tree, node, tree->select_data);
                        }
                    }
                    widget->needs_redraw = VAXP_TRUE;
                }
                return VAXP_TRUE;
            }
            break;
            
        default:
            break;
    }
    
    return VAXP_FALSE;
}

const VaxpWidgetClass vaxp_tree_view_class = {
    .class_name = "VaxpTreeView",
    .instance_size = sizeof(VaxpTreeView),
    .parent_class = &vaxp_widget_class,
    .init = tree_view_init,
    .destroy = tree_view_destroy,
    .measure = tree_view_measure,
    .layout = NULL,
    .draw = tree_view_draw,
    .on_event = tree_view_on_event,
    .on_state_changed = NULL,
};

VaxpResultPtr vaxp_tree_view_create(void) {
    return vaxp_widget_create(&vaxp_tree_view_class);
}

VaxpTreeNode* vaxp_tree_node_create(const char* label, void* data) {
    VaxpTreeNode* node = (VaxpTreeNode*)vaxp_alloc(sizeof(VaxpTreeNode));
    if (!node) return NULL;
    
    memset(node, 0, sizeof(VaxpTreeNode));
    
    if (label) {
        VaxpSize len = strlen(label) + 1;
        node->label = (char*)vaxp_alloc(len);
        if (node->label) memcpy(node->label, label, len);
    }
    
    node->data = data;
    node->expanded = VAXP_FALSE;
    node->selectable = VAXP_TRUE;
    
    return node;
}

VaxpResult vaxp_tree_view_add_root(VaxpTreeView* tree, VaxpTreeNode* node) {
    VAXP_ENSURE_NOT_NULL(tree);
    VAXP_ENSURE_NOT_NULL(node);
    
    tree->root = node;
    node->depth = 0;
    node->expanded = VAXP_TRUE;
    
    return VAXP_OK_UNIT();
}

VaxpResult vaxp_tree_node_add_child(VaxpTreeNode* parent, VaxpTreeNode* child) {
    VAXP_ENSURE_NOT_NULL(parent);
    VAXP_ENSURE_NOT_NULL(child);
    
    child->parent = parent;
    child->depth = parent->depth + 1;
    
    if (!parent->children) {
        parent->children = child;
    } else {
        VaxpTreeNode* last = parent->children;
        while (last->next) last = last->next;
        last->next = child;
    }
    
    parent->child_count++;
    return VAXP_OK_UNIT();
}

static void expand_recursive(VaxpTreeNode* node, VaxpBool expand) {
    node->expanded = expand;
    VaxpTreeNode* child = node->children;
    while (child) {
        expand_recursive(child, expand);
        child = child->next;
    }
}

void vaxp_tree_view_expand_all(VaxpTreeView* tree) {
    if (tree && tree->root) {
        expand_recursive(tree->root, VAXP_TRUE);
        vaxp_widget_invalidate((VaxpWidget*)tree);
    }
}

void vaxp_tree_view_collapse_all(VaxpTreeView* tree) {
    if (tree && tree->root) {
        expand_recursive(tree->root, VAXP_FALSE);
        tree->root->expanded = VAXP_TRUE;
        vaxp_widget_invalidate((VaxpWidget*)tree);
    }
}

void vaxp_tree_view_set_on_select(VaxpTreeView* tree, VaxpTreeSelectCallback callback, void* data) {
    if (tree) {
        tree->on_select = callback;
        tree->select_data = data;
    }
}
