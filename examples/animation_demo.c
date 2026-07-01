/*
 * VAXPUI Animation System Demo
 * 
 * Uses widget's draw() + needs_redraw for continuous animation updates.
 * State managed via VaxpCubit.
 */

#include <vaxp/vaxpui.h>
#include <stdio.h>
#include <sys/time.h>

/* ============================================================================
 * ANIMATION STATE (Cubit)
 * ============================================================================ */

typedef struct AnimState {
    VaxpAnimation* x_anim;
    VaxpAnimation* y_anim;
    VaxpAnimation* color_anim;
    VaxpSpringAnimation* spring;
    
    VaxpF32 box_x;
    VaxpF32 box_y;
    VaxpF32 spring_x;
    VaxpF32 color_t;
    
    VaxpEasing easing;
    int easing_idx;
    VaxpBool paused;
    
    VaxpF64 last_time;
    
    /* FPS counter */
    VaxpU32 frame_count;
    VaxpF64 fps_time;
    VaxpF32 current_fps;
} AnimState;

VAXP_DEFINE_CUBIT(Anim, AnimState);
static AnimCubit* g_cubit = NULL;

/* ============================================================================
 * TIME
 * ============================================================================ */

static VaxpF64 now_seconds(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (VaxpF64)tv.tv_sec + (VaxpF64)tv.tv_usec / 1e6;
}

/* ============================================================================
 * CUBIT INIT
 * ============================================================================ */

static void init_cubit(void) {
    g_cubit = VAXP_CUBIT_NEW(Anim, AnimState);
    if (!g_cubit) return;
    
    AnimState* s = &g_cubit->state;
    s->box_x = 50; s->box_y = 50; s->spring_x = 50; s->color_t = 0;
    s->easing = VAXP_EASING_ELASTIC_OUT;
    s->easing_idx = VAXP_EASING_ELASTIC_OUT;
    s->paused = VAXP_FALSE;
    s->last_time = now_seconds();
    s->frame_count = 0;
    s->fps_time = now_seconds();
    s->current_fps = 0;
    
    /* X anim */
    VaxpResultPtr r = vaxp_animation_create(50, 380, 2.0f);
    if (r.ok) {
        s->x_anim = (VaxpAnimation*)r.value;
        vaxp_animation_set_easing(s->x_anim, s->easing);
        vaxp_animation_set_repeat(s->x_anim, VAXP_REPEAT_PING_PONG, -1);
        vaxp_animation_start(s->x_anim);
    }
    
    /* Y anim */
    r = vaxp_animation_create(50, 160, 1.5f);
    if (r.ok) {
        s->y_anim = (VaxpAnimation*)r.value;
        vaxp_animation_set_easing(s->y_anim, VAXP_EASING_BOUNCE_OUT);
        vaxp_animation_set_repeat(s->y_anim, VAXP_REPEAT_PING_PONG, -1);
        vaxp_animation_start(s->y_anim);
    }
    
    /* Color anim */
    r = vaxp_animation_create(0, 1, 3.0f);
    if (r.ok) {
        s->color_anim = (VaxpAnimation*)r.value;
        vaxp_animation_set_easing(s->color_anim, VAXP_EASING_SINE_IN_OUT);
        vaxp_animation_set_repeat(s->color_anim, VAXP_REPEAT_PING_PONG, -1);
        vaxp_animation_start(s->color_anim);
    }
    
    /* Spring */
    r = vaxp_spring_animation_create_preset(50, 360, VAXP_SPRING_WOBBLY);
    if (r.ok) {
        s->spring = (VaxpSpringAnimation*)r.value;
        vaxp_spring_animation_start(s->spring);
    }
}

static void cleanup_cubit(void) {
    if (!g_cubit) return;
    AnimState* s = &g_cubit->state;
    if (s->x_anim) vaxp_unref(s->x_anim);
    if (s->y_anim) vaxp_unref(s->y_anim);
    if (s->color_anim) vaxp_unref(s->color_anim);
    if (s->spring) vaxp_unref(s->spring);
    vaxp_cubit_destroy(g_cubit);
    g_cubit = NULL;
}

/* ============================================================================
 * ACTIONS
 * ============================================================================ */

static void do_bounce(void) {
    if (!g_cubit || !g_cubit->state.spring) return;
    vaxp_spring_animation_add_velocity(g_cubit->state.spring, -400);
    if (g_cubit->state.spring->state != VAXP_ANIM_STATE_RUNNING)
        vaxp_spring_animation_start(g_cubit->state.spring);
    printf("🏀 Bounce!\n");
}

static void do_next_easing(void) {
    if (!g_cubit) return;
    AnimState* s = &g_cubit->state;
    s->easing_idx = (s->easing_idx + 1) % VAXP_EASING_COUNT;
    s->easing = (VaxpEasing)s->easing_idx;
    if (s->x_anim) {
        vaxp_animation_set_easing(s->x_anim, s->easing);
        vaxp_animation_start(s->x_anim);
    }
    printf("🔄 Easing: %s\n", vaxp_easing_name(s->easing));
}

static void do_pause(void) {
    if (!g_cubit) return;
    AnimState* s = &g_cubit->state;
    s->paused = !s->paused;
    if (s->paused) {
        if (s->x_anim) vaxp_animation_pause(s->x_anim);
        if (s->y_anim) vaxp_animation_pause(s->y_anim);
        if (s->color_anim) vaxp_animation_pause(s->color_anim);
        printf("⏸️ Paused\n");
    } else {
        if (s->x_anim) vaxp_animation_resume(s->x_anim);
        if (s->y_anim) vaxp_animation_resume(s->y_anim);
        if (s->color_anim) vaxp_animation_resume(s->color_anim);
        printf("▶️ Resumed\n");
    }
}

/* ============================================================================
 * ANIMATION CANVAS WIDGET
 * ============================================================================ */

typedef struct AnimCanvas {
    VaxpWidget base;
} AnimCanvas;

static void canvas_measure(VaxpWidget* w, VaxpF32 aw, VaxpF32 ah, VaxpF32* ow, VaxpF32* oh) {
    (void)w; (void)aw; (void)ah;
    *ow = 480; *oh = 260;
}

static void canvas_draw(VaxpWidget* widget, VaxpCanvas* canvas) {
    if (!g_cubit) return;
    AnimState* s = &g_cubit->state;
    
    /* Delta time calculation */
    VaxpF64 now = now_seconds();
    VaxpF64 dt = now - s->last_time;
    s->last_time = now;
    
    /* FPS calculation */
    s->frame_count++;
    VaxpF64 fps_elapsed = now - s->fps_time;
    if (fps_elapsed >= 1.0) {
        s->current_fps = (VaxpF32)s->frame_count / (VaxpF32)fps_elapsed;
        s->frame_count = 0;
        s->fps_time = now;
    }
    
    /* UPDATE ANIMATIONS */
    if (s->x_anim) s->x_anim->to_value = widget->bounds.width - 100;
    if (s->spring) vaxp_spring_animation_set_target(s->spring, widget->bounds.width - 120);

    if (!s->paused) {
        if (s->x_anim && vaxp_animation_is_running(s->x_anim)) {
            vaxp_animation_update(s->x_anim, dt);
            s->box_x = vaxp_animation_get_value(s->x_anim);
        }
        if (s->y_anim && vaxp_animation_is_running(s->y_anim)) {
            vaxp_animation_update(s->y_anim, dt);
            s->box_y = vaxp_animation_get_value(s->y_anim);
        }
        if (s->color_anim && vaxp_animation_is_running(s->color_anim)) {
            vaxp_animation_update(s->color_anim, dt);
            s->color_t = vaxp_animation_get_value(s->color_anim);
        }
        if (s->spring && vaxp_spring_animation_is_running(s->spring)) {
            vaxp_spring_animation_update(s->spring, dt);
            s->spring_x = vaxp_spring_animation_get_value(s->spring);
        }
    }
    
    /* DRAW BACKGROUND */
    VaxpRectF bg = {0, 0, widget->bounds.width, widget->bounds.height};
    VaxpPaint bgp = vaxp_paint_fill(vaxp_color_rgb(25, 28, 38));
    vaxp_canvas_draw_rounded_rect(canvas, bg, 10, &bgp);
    
    /* Grid */
    VaxpPaint grid = vaxp_paint_stroke(vaxp_color_rgba(70,70,90,60), 1);
    for (VaxpF32 x = 50; x < widget->bounds.width; x += 50)
        vaxp_canvas_draw_line(canvas, x, 0, x, widget->bounds.height, &grid);
    for (VaxpF32 y = 50; y < widget->bounds.height; y += 50)
        vaxp_canvas_draw_line(canvas, 0, y, widget->bounds.width, y, &grid);
    
    /* BOX 1: Tween with color */
    VaxpColor c1 = vaxp_color_rgb(255, 87, 34);
    VaxpColor c2 = vaxp_color_rgb(156, 39, 176);
    VaxpColor col = vaxp_color_lerp(c1, c2, s->color_t);
    VaxpRectF box1 = {s->box_x, s->box_y, 55, 55};
    VaxpPaint p1 = vaxp_paint_fill(col);
    vaxp_canvas_draw_rounded_rect(canvas, box1, 8, &p1);
    VaxpPaint txt = vaxp_paint_fill(VAXP_COLOR_WHITE);
    vaxp_canvas_draw_text(canvas, "Tween", s->box_x+5, s->box_y+33, NULL, &txt);
    
    /* BOX 2: Spring */
    VaxpRectF box2 = {s->spring_x, 170, 45, 45};
    VaxpPaint p2 = vaxp_paint_fill(vaxp_color_rgb(76, 175, 80));
    vaxp_canvas_draw_rounded_rect(canvas, box2, 22, &p2);
    vaxp_canvas_draw_text(canvas, "Spring", s->spring_x+1, 188, NULL, &txt);
    
    /* Circle following X */
    VaxpPaint p3 = vaxp_paint_fill(vaxp_color_rgb(33, 150, 243));
    vaxp_canvas_draw_circle(canvas, s->box_x + 27, 230, 18, &p3);
    
    /* FPS display */
    char fps_str[32];
    snprintf(fps_str, sizeof(fps_str), "%.1f FPS", s->current_fps);
    VaxpPaint fps_paint = vaxp_paint_fill(vaxp_color_rgb(0, 255, 100));
    vaxp_canvas_draw_text(canvas, fps_str, widget->bounds.width - 80, 20, NULL, &fps_paint);
    
    /* Info */
    char info[100];
    snprintf(info, sizeof(info), "%s | %s", 
             vaxp_easing_name(s->easing), s->paused ? "PAUSED" : "PLAYING");
    VaxpPaint inf = vaxp_paint_fill(vaxp_color_rgb(180, 180, 200));
    vaxp_canvas_draw_text(canvas, info, 10, widget->bounds.height - 10, NULL, &inf);
    
    /* REQUEST NEXT FRAME - This keeps animations running! */
    widget->needs_redraw = VAXP_TRUE;
}

static VaxpBool canvas_event(VaxpWidget* w, const VaxpEvent* e) {
    (void)w;
    if (e->type == VAXP_EVENT_MOUSE_BUTTON_DOWN) {
        do_bounce();
        do_next_easing();
        return VAXP_TRUE;
    }
    return VAXP_FALSE;
}

static const VaxpWidgetClass anim_canvas_class = {
    .class_name = "AnimCanvas",
    .instance_size = sizeof(AnimCanvas),
    .parent_class = &vaxp_widget_class,
    .init = NULL,
    .destroy = NULL,
    .measure = canvas_measure,
    .layout = NULL,
    .draw = canvas_draw,
    .on_event = canvas_event,
    .on_state_changed = NULL,
};

static VaxpWidget* anim_canvas_new(void) {
    VaxpResultPtr r = vaxp_widget_create(&anim_canvas_class);
    if (!r.ok) return NULL;
    VaxpWidget* w = (VaxpWidget*)r.value;
    w->layout.flex_grow = 1;
    return w;
}

/* ============================================================================
 * BUTTON CALLBACKS
 * ============================================================================ */

static void on_pause_btn(VaxpButton* b, void* d) { (void)b; (void)d; do_pause(); }
static void on_bounce_btn(VaxpButton* b, void* d) { (void)b; (void)d; do_bounce(); }
static void on_easing_btn(VaxpButton* b, void* d) { (void)b; (void)d; do_next_easing(); }

/* ============================================================================
 * UI BUILD
 * ============================================================================ */

static VaxpWidget* build_ui(void* ud) {
    (void)ud;
    
    return vaxp_col(
        .padding = (VaxpInsets){14,14,14,14},
        .gap = 10,
        .align = VAXP_ALIGN_STRETCH,
        .background = vaxp_color_rgb(35, 38, 48),
        .children = (VaxpWidget*[]){
            vaxp_text("🎬 VAXPUI Animation Demo", 
                .size = 18, .color = vaxp_color_rgb(255,255,255)),
            
            vaxp_row(
                .gap = 8,
                .children = (VaxpWidget*[]){
                    vaxp_btn("⏯️ Pause", .on_click = on_pause_btn, 
                              .color = vaxp_color_rgb(76, 175, 80)),
                    vaxp_btn("🏀 Bounce", .on_click = on_bounce_btn, 
                              .color = vaxp_color_rgb(255, 152, 0)),
                    vaxp_btn("🔄 Easing", .on_click = on_easing_btn, 
                              .color = vaxp_color_rgb(33, 150, 243)),
                    NULL
                }
            ),
            
            anim_canvas_new(),
            
            vaxp_text("Click canvas or buttons • ESC to exit",
                .size = 11, .color = vaxp_color_rgb(130, 130, 150)),
            NULL
        }
    );
}

/* ============================================================================
 * MAIN
 * ============================================================================ */

int main(void) {
    printf("=== VAXPUI Animation Demo ===\n");
    printf("Using VaxpCubit + needs_redraw pattern\n\n");
    
    init_cubit();
    
    int ret = VAXP_APP(
        .title = "VAXPUI Animation Demo",
        .width = 530,
        .height = 380,
        .background = vaxp_color_rgb(35, 38, 48),
        .build = build_ui,
        .debug = VAXP_TRUE
    );
    
    cleanup_cubit();
    
    printf("\nBye!\n");
    return ret;
}
