/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_rating.h - Star rating widget
 */

#ifndef VENOM_RATING_H
#define VENOM_RATING_H

#include "venom/widgets/venom_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VenomRating VenomRating;
typedef void (*VenomRatingCallback)(VenomRating* rating, VenomF32 value, void* data);

struct VenomRating {
    VenomWidget base;
    
    VenomF32 value;              /* 0.0 - max_value */
    VenomU32 max_value;          /* Number of stars */
    VenomBool allow_half;
    VenomBool read_only;
    
    VenomRatingCallback on_change;
    void* callback_data;
    
    VenomF32 star_size;
    VenomF32 spacing;
    VenomColor filled_color;
    VenomColor empty_color;
    VenomI32 hover_index;
};

VenomResultPtr venom_rating_create(void);
void venom_rating_set_value(VenomRating* rating, VenomF32 value);
VenomF32 venom_rating_get_value(const VenomRating* rating);
void venom_rating_set_max(VenomRating* rating, VenomU32 max);
void venom_rating_set_on_change(VenomRating* rating, VenomRatingCallback callback, void* data);

extern const VenomWidgetClass venom_rating_class;

#define venom_rating(...) _venom_rating_build(&(VenomRatingConfig){ __VA_ARGS__ })

typedef struct VenomRatingConfig {
    VenomF32 value;
    VenomU32 max;
    VenomBool read_only;
    VenomRatingCallback on_change;
    void* data;
} VenomRatingConfig;

VenomWidget* _venom_rating_build(const VenomRatingConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VENOM_RATING_H */
