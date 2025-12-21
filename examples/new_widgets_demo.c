/*
 * Test Carousel directly
 */
#include <venom/venomui.h>
#include <stdio.h>

static VenomWidget* build(void* d) {
    (void)d;
    printf("Building Carousel test...\n");
    
    VenomResultPtr r = venom_carousel_create();
    if (!r.ok) {
        printf("Carousel creation failed!\n");
        VenomResultPtr lr = venom_label_create("Carousel Error");
        return lr.ok ? (VenomWidget*)lr.value : NULL;
    }
    
    VenomCarousel* carousel = (VenomCarousel*)r.value;
    
    /* Add colored slides */
    VenomResultPtr s1 = venom_container_create();
    if (s1.ok) {
        venom_container_set_background((VenomContainer*)s1.value, venom_color_rgb(60, 120, 220));
        venom_carousel_add_item(carousel, (VenomWidget*)s1.value);
    }
    
    VenomResultPtr s2 = venom_container_create();
    if (s2.ok) {
        venom_container_set_background((VenomContainer*)s2.value, venom_color_rgb(60, 180, 100));
        venom_carousel_add_item(carousel, (VenomWidget*)s2.value);
    }
    
    VenomResultPtr s3 = venom_container_create();
    if (s3.ok) {
        venom_container_set_background((VenomContainer*)s3.value, venom_color_rgb(240, 140, 60));
        venom_carousel_add_item(carousel, (VenomWidget*)s3.value);
    }
    
    venom_carousel_set_auto_play(carousel, VENOM_TRUE, 1500);
    venom_carousel_set_show_arrows(carousel, VENOM_TRUE);
    
    printf("Carousel created with %u slides\n", carousel->item_count);
    
    return (VenomWidget*)carousel;
}

int main(void) {
    printf("=== Carousel Test ===\n\n");
    
    VenomAppConfig cfg = {
        .title = "Carousel Test",
        .width = 500,
        .height = 300,
        .background = venom_color_rgb(50, 50, 60),
        .build = build,
    };
    
    return venom_run_app(&cfg);
}
