/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_chip.h - Chip widget (tag/filter element)
 */

#ifndef VAXP_CHIP_H
#define VAXP_CHIP_H

#include "vaxp/widgets/vaxp_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VaxpChip VaxpChip;
typedef void (*VaxpChipCallback)(VaxpChip* chip, void* data);

typedef enum VaxpChipType {
    VAXP_CHIP_INPUT,        /* With delete button */
    VAXP_CHIP_CHOICE,       /* Selectable */
    VAXP_CHIP_FILTER,       /* With checkmark when selected */
    VAXP_CHIP_ACTION,       /* Clickable action */
} VaxpChipType;

struct VaxpChip {
    VaxpWidget base;
    
    char* label;
    char* icon;              /* Optional leading icon */
    VaxpChipType type;
    VaxpBool selected;
    VaxpBool deletable;
    
    VaxpChipCallback on_click;
    VaxpChipCallback on_delete;
    void* callback_data;
    
    VaxpColor background_color;
    VaxpColor selected_color;
    VaxpColor text_color;
    VaxpColor delete_color;
    VaxpF32 height;
};

VaxpResultPtr vaxp_chip_create(void);
void vaxp_chip_set_label(VaxpChip* chip, const char* label);
void vaxp_chip_set_selected(VaxpChip* chip, VaxpBool selected);
void vaxp_chip_set_on_click(VaxpChip* chip, VaxpChipCallback callback, void* data);
void vaxp_chip_set_on_delete(VaxpChip* chip, VaxpChipCallback callback, void* data);

extern const VaxpWidgetClass vaxp_chip_class;

#define vaxp_chip(...) _vaxp_chip_build(&(VaxpChipConfig){ __VA_ARGS__ })

typedef struct VaxpChipConfig {
    const char* label;
    VaxpChipType type;
    VaxpBool selected;
    VaxpChipCallback on_click;
    void* data;
} VaxpChipConfig;

VaxpWidget* _vaxp_chip_build(const VaxpChipConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VAXP_CHIP_H */
