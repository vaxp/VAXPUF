/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_chip.h - Chip widget (tag/filter element)
 */

#ifndef VENOM_CHIP_H
#define VENOM_CHIP_H

#include "venom/widgets/venom_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VenomChip VenomChip;
typedef void (*VenomChipCallback)(VenomChip* chip, void* data);

typedef enum VenomChipType {
    VENOM_CHIP_INPUT,        /* With delete button */
    VENOM_CHIP_CHOICE,       /* Selectable */
    VENOM_CHIP_FILTER,       /* With checkmark when selected */
    VENOM_CHIP_ACTION,       /* Clickable action */
} VenomChipType;

struct VenomChip {
    VenomWidget base;
    
    char* label;
    char* icon;              /* Optional leading icon */
    VenomChipType type;
    VenomBool selected;
    VenomBool deletable;
    
    VenomChipCallback on_click;
    VenomChipCallback on_delete;
    void* callback_data;
    
    VenomColor background_color;
    VenomColor selected_color;
    VenomColor text_color;
    VenomColor delete_color;
    VenomF32 height;
};

VenomResultPtr venom_chip_create(void);
void venom_chip_set_label(VenomChip* chip, const char* label);
void venom_chip_set_selected(VenomChip* chip, VenomBool selected);
void venom_chip_set_on_click(VenomChip* chip, VenomChipCallback callback, void* data);
void venom_chip_set_on_delete(VenomChip* chip, VenomChipCallback callback, void* data);

extern const VenomWidgetClass venom_chip_class;

#define venom_chip(...) _venom_chip_build(&(VenomChipConfig){ __VA_ARGS__ })

typedef struct VenomChipConfig {
    const char* label;
    VenomChipType type;
    VenomBool selected;
    VenomChipCallback on_click;
    void* data;
} VenomChipConfig;

VenomWidget* _venom_chip_build(const VenomChipConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VENOM_CHIP_H */
