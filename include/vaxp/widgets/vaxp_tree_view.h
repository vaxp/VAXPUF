/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_tree_view.h - Tree view widget
 */

#ifndef VAXP_TREE_VIEW_H
#define VAXP_TREE_VIEW_H

#include "vaxp/widgets/vaxp_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VaxpTreeNode VaxpTreeNode;
typedef struct VaxpTreeView VaxpTreeView;

typedef VaxpWidget* (*VaxpTreeNodeBuilder)(VaxpTreeNode* node, void* data, void* user_data);
typedef void (*VaxpTreeSelectCallback)(VaxpTreeView* tree, VaxpTreeNode* node, void* data);

struct VaxpTreeNode {
    char* label;
    void* data;
    VaxpBool expanded;
    VaxpBool selected;
    VaxpBool selectable;
    
    VaxpTreeNode* parent;
    VaxpTreeNode* children;
    VaxpTreeNode* next;
    VaxpU32 child_count;
    VaxpU32 depth;
};

struct VaxpTreeView {
    VaxpWidget base;
    
    VaxpTreeNode* root;
    VaxpTreeNode* selected;
    
    VaxpTreeNodeBuilder node_builder;
    void* builder_data;
    VaxpTreeSelectCallback on_select;
    void* select_data;
    
    VaxpF32 node_height;
    VaxpF32 indent;
    VaxpF32 scroll_offset;
    VaxpI32 hover_index;
    
    VaxpColor selection_color;
    VaxpColor hover_color;
    VaxpColor expand_color;
};

VaxpResultPtr vaxp_tree_view_create(void);
VaxpTreeNode* vaxp_tree_node_create(const char* label, void* data);
VaxpResult vaxp_tree_view_add_root(VaxpTreeView* tree, VaxpTreeNode* node);
VaxpResult vaxp_tree_node_add_child(VaxpTreeNode* parent, VaxpTreeNode* child);
void vaxp_tree_view_expand_all(VaxpTreeView* tree);
void vaxp_tree_view_collapse_all(VaxpTreeView* tree);
void vaxp_tree_view_set_on_select(VaxpTreeView* tree, VaxpTreeSelectCallback callback, void* data);

extern const VaxpWidgetClass vaxp_tree_view_class;

#ifdef __cplusplus
}
#endif

#endif /* VAXP_TREE_VIEW_H */
