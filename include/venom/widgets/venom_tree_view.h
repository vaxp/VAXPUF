/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_tree_view.h - Tree view widget
 */

#ifndef VENOM_TREE_VIEW_H
#define VENOM_TREE_VIEW_H

#include "venom/widgets/venom_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VenomTreeNode VenomTreeNode;
typedef struct VenomTreeView VenomTreeView;

typedef VenomWidget* (*VenomTreeNodeBuilder)(VenomTreeNode* node, void* data, void* user_data);
typedef void (*VenomTreeSelectCallback)(VenomTreeView* tree, VenomTreeNode* node, void* data);

struct VenomTreeNode {
    char* label;
    void* data;
    VenomBool expanded;
    VenomBool selected;
    VenomBool selectable;
    
    VenomTreeNode* parent;
    VenomTreeNode* children;
    VenomTreeNode* next;
    VenomU32 child_count;
    VenomU32 depth;
};

struct VenomTreeView {
    VenomWidget base;
    
    VenomTreeNode* root;
    VenomTreeNode* selected;
    
    VenomTreeNodeBuilder node_builder;
    void* builder_data;
    VenomTreeSelectCallback on_select;
    void* select_data;
    
    VenomF32 node_height;
    VenomF32 indent;
    VenomF32 scroll_offset;
    VenomI32 hover_index;
    
    VenomColor selection_color;
    VenomColor hover_color;
    VenomColor expand_color;
};

VenomResultPtr venom_tree_view_create(void);
VenomTreeNode* venom_tree_node_create(const char* label, void* data);
VenomResult venom_tree_view_add_root(VenomTreeView* tree, VenomTreeNode* node);
VenomResult venom_tree_node_add_child(VenomTreeNode* parent, VenomTreeNode* child);
void venom_tree_view_expand_all(VenomTreeView* tree);
void venom_tree_view_collapse_all(VenomTreeView* tree);
void venom_tree_view_set_on_select(VenomTreeView* tree, VenomTreeSelectCallback callback, void* data);

extern const VenomWidgetClass venom_tree_view_class;

#ifdef __cplusplus
}
#endif

#endif /* VENOM_TREE_VIEW_H */
