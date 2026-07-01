/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_rating.h - Star rating widget
 */

#ifndef VAXP_RATING_H
#define VAXP_RATING_H

#include "vaxp/widgets/vaxp_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VaxpRating VaxpRating;
typedef void (*VaxpRatingCallback)(VaxpRating* rating, VaxpF32 value, void* data);

struct VaxpRating {
    VaxpWidget base;
    
    VaxpF32 value;              /* 0.0 - max_value */
    VaxpU32 max_value;          /* Number of stars */
    VaxpBool allow_half;
    VaxpBool read_only;
    
    VaxpRatingCallback on_change;
    void* callback_data;
    
    VaxpF32 star_size;
    VaxpF32 spacing;
    VaxpColor filled_color;
    VaxpColor empty_color;
    VaxpI32 hover_index;
};

VaxpResultPtr vaxp_rating_create(void);
void vaxp_rating_set_value(VaxpRating* rating, VaxpF32 value);
VaxpF32 vaxp_rating_get_value(const VaxpRating* rating);
void vaxp_rating_set_max(VaxpRating* rating, VaxpU32 max);
void vaxp_rating_set_on_change(VaxpRating* rating, VaxpRatingCallback callback, void* data);

extern const VaxpWidgetClass vaxp_rating_class;

#define vaxp_rating(...) _vaxp_rating_build(&(VaxpRatingConfig){ __VA_ARGS__ })

typedef struct VaxpRatingConfig {
    VaxpF32 value;
    VaxpU32 max;
    VaxpBool read_only;
    VaxpRatingCallback on_change;
    void* data;
} VaxpRatingConfig;

VaxpWidget* _vaxp_rating_build(const VaxpRatingConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VAXP_RATING_H */
