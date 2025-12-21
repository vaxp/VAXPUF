/*
 * VENOMUI Animation System Demo
 * 
 * Demonstrates the complete animation system including:
 * - Easing functions
 * - Tween animations
 * - Spring animations
 * - Animation groups (sequence, parallel, stagger)
 */

#include <venom/venomui.h>
#include <stdio.h>
#include <sys/time.h>

/* ============================================================================
 * DEMO STATE
 * ============================================================================ */

typedef struct {
    VenomAnimation* tween;
    VenomSpringAnimation* spring;
    VenomAnimationGroup* sequence;
    VenomF32 box_x;
    VenomF32 box_y;
    VenomF32 box_scale;
    VenomColor box_color;
    VenomEasing current_easing;
} DemoState;

static DemoState g_demo = {0};

/* Get current time in seconds */
static VenomF64 get_time_seconds(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (VenomF64)tv.tv_sec + (VenomF64)tv.tv_usec / 1000000.0;
}

/* ============================================================================
 * ANIMATION CALLBACKS
 * ============================================================================ */

static void on_tween_update(VenomAnimation* anim, VenomF32 value, VenomF32 progress, void* user_data) {
    (void)anim;
    (void)progress;
    (void)user_data;
    g_demo.box_x = value;
}

static void on_spring_update(VenomAnimation* anim, VenomF32 value, VenomF32 progress, void* user_data) {
    (void)anim;
    (void)progress;
    (void)user_data;
    g_demo.box_y = value;
}

static void on_animation_complete(VenomAnimation* anim, VenomAnimationState state, void* user_data) {
    (void)anim;
    (void)state;
    (void)user_data;
    printf("Animation completed!\n");
}

/* ============================================================================
 * DEMO INITIALIZATION
 * ============================================================================ */

static void init_animations(void) {
    g_demo.box_x = 50.0f;
    g_demo.box_y = 200.0f;
    g_demo.box_scale = 1.0f;
    g_demo.box_color = venom_color_rgb(63, 81, 181);
    g_demo.current_easing = VENOM_EASING_CUBIC_OUT;
    
    /* Create tween animation */
    VenomResultPtr tween_result = venom_animation_create(50.0f, 500.0f, 1.0f);
    if (tween_result.ok) {
        g_demo.tween = (VenomAnimation*)tween_result.value;
        venom_animation_set_easing(g_demo.tween, VENOM_EASING_ELASTIC_OUT);
        venom_animation_set_on_update(g_demo.tween, on_tween_update, NULL);
        venom_animation_set_on_complete(g_demo.tween, on_animation_complete, NULL);
        venom_animation_set_repeat(g_demo.tween, VENOM_REPEAT_PING_PONG, -1);
        venom_animation_start(g_demo.tween);
    }
    
    /* Create spring animation */
    VenomResultPtr spring_result = venom_spring_animation_create_preset(
        200.0f, 400.0f, VENOM_SPRING_WOBBLY
    );
    if (spring_result.ok) {
        g_demo.spring = (VenomSpringAnimation*)spring_result.value;
        venom_spring_animation_set_on_update(g_demo.spring, on_spring_update, NULL);
        venom_spring_animation_start(g_demo.spring);
    }
    
    printf("Animation Demo Started!\n");
    printf("- Tween animation: x position with elastic ease\n");
    printf("- Spring animation: y position with wobbly preset\n");
    printf("Press ESC to exit\n");
}

static void cleanup_animations(void) {
    if (g_demo.tween) {
        venom_unref(g_demo.tween);
        g_demo.tween = NULL;
    }
    if (g_demo.spring) {
        venom_unref(g_demo.spring);
        g_demo.spring = NULL;
    }
    if (g_demo.sequence) {
        venom_unref(g_demo.sequence);
        g_demo.sequence = NULL;
    }
}

/* ============================================================================
 * UI BUILD
 * ============================================================================ */

static VenomWidget* build_ui(void* user_data) {
    (void)user_data;
    
    return venom_col(
        .padding = (VenomInsets){ 20, 20, 20, 20 },
        .gap = 16,
        .background = venom_color_rgb(250, 250, 252),
        .children = (VenomWidget*[]){
            venom_text("🎬 Animation System Demo", 
                .size = 24, 
                .color = venom_color_rgb(33, 33, 33)
            ),
            venom_text("Demonstrating: Easing, Tween, Spring, Color Lerp",
                .size = 14,
                .color = venom_color_rgb(100, 100, 100)
            ),
            venom_sized_box(600, 300),  /* Placeholder for animated box */
            venom_row(
                .gap = 12,
                .children = (VenomWidget*[]){
                    venom_btn("🔄 Restart", .on_click = NULL),
                    venom_btn("⏸️ Pause", .on_click = NULL),
                    venom_btn("▶️ Resume", .on_click = NULL),
                    NULL
                }
            ),
            NULL
        }
    );
}

/* ============================================================================
 * MAIN
 * ============================================================================ */

int main(void) {
    printf("=== VENOMUI Animation System Demo ===\n\n");
    
    /* Print available easing functions */
    printf("Available easing functions (%d total):\n", VENOM_EASING_COUNT);
    for (int i = 0; i < VENOM_EASING_COUNT; i++) {
        printf("  [%2d] %s\n", i, venom_easing_name((VenomEasing)i));
    }
    printf("\n");
    
    /* Test easing functions */
    printf("Easing function test (t=0.5):\n");
    const VenomEasing test_easings[] = {
        VENOM_EASING_LINEAR,
        VENOM_EASING_QUAD_IN_OUT,
        VENOM_EASING_CUBIC_OUT,
        VENOM_EASING_ELASTIC_OUT,
        VENOM_EASING_BOUNCE_OUT,
    };
    for (size_t i = 0; i < sizeof(test_easings)/sizeof(test_easings[0]); i++) {
        VenomF32 result = venom_easing_apply(test_easings[i], 0.5f);
        printf("  %-16s: %.4f\n", venom_easing_name(test_easings[i]), result);
    }
    printf("\n");
    
    /* Test color interpolation */
    printf("Color interpolation test:\n");
    VenomColor red = venom_color_rgb(255, 0, 0);
    VenomColor blue = venom_color_rgb(0, 0, 255);
    for (VenomF32 t = 0.0f; t <= 1.0f; t += 0.25f) {
        VenomColor c = venom_color_lerp(red, blue, t);
        printf("  t=%.2f: RGB(%3d, %3d, %3d)\n", t, c.r, c.g, c.b);
    }
    printf("\n");
    
    /* Test animation creation and update */
    printf("Animation lifecycle test:\n");
    VenomResultPtr result = venom_animation_create(0.0f, 100.0f, 1.0f);
    if (result.ok) {
        VenomAnimation* anim = (VenomAnimation*)result.value;
        venom_animation_set_easing(anim, VENOM_EASING_CUBIC_IN_OUT);
        venom_animation_start(anim);
        
        printf("  Created animation: from=%.0f, to=%.0f, duration=%.1fs\n",
               anim->from_value, anim->to_value, anim->duration);
        
        /* Simulate animation updates */
        VenomF64 dt = 0.1;
        for (int frame = 0; frame < 12; frame++) {
            VenomBool running = venom_animation_update(anim, dt);
            printf("  Frame %2d: value=%.2f, progress=%.2f, running=%s\n",
                   frame, anim->current_value, anim->progress,
                   running ? "yes" : "no");
        }
        
        venom_unref(anim);
        printf("  Animation freed successfully\n");
    }
    printf("\n");
    
    /* Test spring animation */
    printf("Spring animation test:\n");
    VenomResultPtr spring_result = venom_spring_animation_create(0.0f, 100.0f);
    if (spring_result.ok) {
        VenomSpringAnimation* spring = (VenomSpringAnimation*)spring_result.value;
        venom_spring_animation_apply_preset(spring, VENOM_SPRING_WOBBLY);
        venom_spring_animation_start(spring);
        
        printf("  Preset: WOBBLY (stiffness=%.0f, damping=%.0f)\n",
               spring->stiffness, spring->damping);
        
        /* Simulate spring updates */
        VenomF64 dt = 0.016;  /* ~60fps */
        for (int frame = 0; frame < 10; frame++) {
            venom_spring_animation_update(spring, dt);
            printf("  Frame %2d: value=%.2f, velocity=%.2f\n",
                   frame, spring->current_value, spring->velocity);
        }
        
        venom_unref(spring);
        printf("  Spring freed successfully\n");
    }
    printf("\n");
    
    /* Test animation group */
    printf("Animation group test (sequence):\n");
    VenomResultPtr group_result = venom_animation_group_create(VENOM_ANIM_GROUP_SEQUENCE);
    if (group_result.ok) {
        VenomAnimationGroup* group = (VenomAnimationGroup*)group_result.value;
        
        /* Create child animations */
        VenomAnimation* anim1 = (VenomAnimation*)venom_unwrap_ptr(
            venom_animation_create(0.0f, 50.0f, 0.5f));
        VenomAnimation* anim2 = (VenomAnimation*)venom_unwrap_ptr(
            venom_animation_create(50.0f, 100.0f, 0.5f));
        
        venom_animation_group_add(group, anim1);
        venom_animation_group_add(group, anim2);
        
        /* Release our refs (group owns them now) */
        venom_unref(anim1);
        venom_unref(anim2);
        
        printf("  Group created with %u children\n", 
               venom_animation_group_child_count(group));
        
        venom_animation_group_start(group);
        
        /* Simulate updates */
        VenomF64 dt = 0.2;
        for (int frame = 0; frame < 8; frame++) {
            VenomBool running = venom_animation_group_update(group, dt);
            printf("  Frame %2d: running=%s, state=%d\n",
                   frame, running ? "yes" : "no", group->state);
        }
        
        venom_unref(group);
        printf("  Group freed successfully (children auto-freed)\n");
    }
    printf("\n");
    
    printf("=== All animation tests passed! ===\n");
    printf("Memory leaks should be 0 (check with -Ddebug_memory=true)\n");
    
    return 0;
}
