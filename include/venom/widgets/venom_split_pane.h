/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_split_pane.h - Split pane widget (resizable panels)
 */

#ifndef VENOM_SPLIT_PANE_H
#define VENOM_SPLIT_PANE_H

#include "venom/widgets/venom_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum VenomSplitDirection {
    VENOM_SPLIT_HORIZONTAL,      /* Left-right */
    VENOM_SPLIT_VERTICAL,        /* Top-bottom */
} VenomSplitDirection;

typedef struct VenomSplitPane {
    VenomWidget base;
    
    VenomWidget* first;          /* First panel */
    VenomWidget* second;         /* Second panel */
    
    VenomSplitDirection direction;
    VenomF32 position;           /* 0.0-1.0 ratio, or absolute pixels if > 1 */
    VenomF32 min_first;          /* Min size of first panel */
    VenomF32 min_second;         /* Min size of second panel */
    VenomF32 divider_size;
    
    VenomBool dragging;
    VenomColor divider_color;
    VenomColor divider_hover;
    VenomBool hovering;
    
} VenomSplitPane;

VenomResultPtr venom_split_pane_create(void);
VenomResult venom_split_pane_set_first(VenomSplitPane* pane, VenomWidget* widget);
VenomResult venom_split_pane_set_second(VenomSplitPane* pane, VenomWidget* widget);
void venom_split_pane_set_position(VenomSplitPane* pane, VenomF32 position);
void venom_split_pane_set_direction(VenomSplitPane* pane, VenomSplitDirection dir);
void venom_split_pane_set_min_sizes(VenomSplitPane* pane, VenomF32 min_first, VenomF32 min_second);

extern const VenomWidgetClass venom_split_pane_class;

#define venom_split_pane(...) _venom_split_pane_build(&(VenomSplitPaneConfig){ __VA_ARGS__ })

typedef struct VenomSplitPaneConfig {
    VenomWidget* first;
    VenomWidget* second;
    VenomSplitDirection direction;
    VenomF32 position;
} VenomSplitPaneConfig;

VenomWidget* _venom_split_pane_build(const VenomSplitPaneConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VENOM_SPLIT_PANE_H */
