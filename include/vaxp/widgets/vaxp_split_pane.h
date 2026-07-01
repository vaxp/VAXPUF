/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_split_pane.h - Split pane widget (resizable panels)
 */

#ifndef VAXP_SPLIT_PANE_H
#define VAXP_SPLIT_PANE_H

#include "vaxp/widgets/vaxp_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum VaxpSplitDirection {
    VAXP_SPLIT_HORIZONTAL,      /* Left-right */
    VAXP_SPLIT_VERTICAL,        /* Top-bottom */
} VaxpSplitDirection;

typedef struct VaxpSplitPane {
    VaxpWidget base;
    
    VaxpWidget* first;          /* First panel */
    VaxpWidget* second;         /* Second panel */
    
    VaxpSplitDirection direction;
    VaxpF32 position;           /* 0.0-1.0 ratio, or absolute pixels if > 1 */
    VaxpF32 min_first;          /* Min size of first panel */
    VaxpF32 min_second;         /* Min size of second panel */
    VaxpF32 divider_size;
    
    VaxpBool dragging;
    VaxpColor divider_color;
    VaxpColor divider_hover;
    VaxpBool hovering;
    
} VaxpSplitPane;

VaxpResultPtr vaxp_split_pane_create(void);
VaxpResult vaxp_split_pane_set_first(VaxpSplitPane* pane, VaxpWidget* widget);
VaxpResult vaxp_split_pane_set_second(VaxpSplitPane* pane, VaxpWidget* widget);
void vaxp_split_pane_set_position(VaxpSplitPane* pane, VaxpF32 position);
void vaxp_split_pane_set_direction(VaxpSplitPane* pane, VaxpSplitDirection dir);
void vaxp_split_pane_set_min_sizes(VaxpSplitPane* pane, VaxpF32 min_first, VaxpF32 min_second);

extern const VaxpWidgetClass vaxp_split_pane_class;

#define vaxp_split_pane(...) _vaxp_split_pane_build(&(VaxpSplitPaneConfig){ __VA_ARGS__ })

typedef struct VaxpSplitPaneConfig {
    VaxpWidget* first;
    VaxpWidget* second;
    VaxpSplitDirection direction;
    VaxpF32 position;
} VaxpSplitPaneConfig;

VaxpWidget* _vaxp_split_pane_build(const VaxpSplitPaneConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VAXP_SPLIT_PANE_H */
