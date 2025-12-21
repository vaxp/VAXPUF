/*
 * VENOMUI Animation System Demo
 * 
 * Uses widget's draw() + needs_redraw for continuous animation updates.
 * State managed via VenomCubit.
 */

#include <venom/venomui.h>
#include <stdio.h>
#include <sys/time.h>

/* ============================================================================
 * ANIMATION STATE (Cubit)
 * ============================================================================ */

typedef struct AnimState {
    VenomAnimation* x_anim;
    VenomAnimation* y_anim;
    VenomAnimation* color_anim;
    VenomSpringAnimation* spring;
    
    VenomF32 box_x;
    VenomF32 box_y;
    VenomF32 spring_x;
    VenomF32 color_t;
    
    VenomEasing easing;
    int easing_idx;
    VenomBool paused;
    
    VenomF64 last_time;
    
    /* FPS counter */
    VenomU32 frame_count;
    VenomF64 fps_time;
    VenomF32 current_fps;
} AnimState;

VENOM_DEFINE_CUBIT(Anim, AnimState);
static AnimCubit* g_cubit = NULL;

/* ============================================================================
 * TIME
 * ============================================================================ */

static VenomF64 now_seconds(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (VenomF64)tv.tv_sec + (VenomF64)tv.tv_usec / 1e6;
}

/* ============================================================================
 * CUBIT INIT
 * ============================================================================ */

static void init_cubit(void) {
    g_cubit = VENOM_CUBIT_NEW(Anim, AnimState);
    if (!g_cubit) return;
    
    AnimState* s = &g_cubit->state;
    s->box_x = 50; s->box_y = 50; s->spring_x = 50; s->color_t = 0;
    s->easing = VENOM_EASING_ELASTIC_OUT;
    s->easing_idx = VENOM_EASING_ELASTIC_OUT;
    s->paused = VENOM_FALSE;
    s->last_time = now_seconds();
    s->frame_count = 0;
    s->fps_time = now_seconds();
    s->current_fps = 0;
    
    /* X anim */
    VenomResultPtr r = venom_animation_create(50, 380, 2.0f);
    if (r.ok) {
        s->x_anim = (VenomAnimation*)r.value;
        venom_animation_set_easing(s->x_anim, s->easing);
        venom_animation_set_repeat(s->x_anim, VENOM_REPEAT_PING_PONG, -1);
        venom_animation_start(s->x_anim);
    }
    
    /* Y anim */
    r = venom_animation_create(50, 160, 1.5f);
    if (r.ok) {
        s->y_anim = (VenomAnimation*)r.value;
        venom_animation_set_easing(s->y_anim, VENOM_EASING_BOUNCE_OUT);
        venom_animation_set_repeat(s->y_anim, VENOM_REPEAT_PING_PONG, -1);
        venom_animation_start(s->y_anim);
    }
    
    /* Color anim */
    r = venom_animation_create(0, 1, 3.0f);
    if (r.ok) {
        s->color_anim = (VenomAnimation*)r.value;
        venom_animation_set_easing(s->color_anim, VENOM_EASING_SINE_IN_OUT);
        venom_animation_set_repeat(s->color_anim, VENOM_REPEAT_PING_PONG, -1);
        venom_animation_start(s->color_anim);
    }
    
    /* Spring */
    r = venom_spring_animation_create_preset(50, 360, VENOM_SPRING_WOBBLY);
    if (r.ok) {
        s->spring = (VenomSpringAnimation*)r.value;
        venom_spring_animation_start(s->spring);
    }
}

static void cleanup_cubit(void) {
    if (!g_cubit) return;
    AnimState* s = &g_cubit->state;
    if (s->x_anim) venom_unref(s->x_anim);
    if (s->y_anim) venom_unref(s->y_anim);
    if (s->color_anim) venom_unref(s->color_anim);
    if (s->spring) venom_unref(s->spring);
    venom_cubit_destroy(g_cubit);
    g_cubit = NULL;
}

/* ============================================================================
 * ACTIONS
 * ============================================================================ */

static void do_bounce(void) {
    if (!g_cubit || !g_cubit->state.spring) return;
    venom_spring_animation_add_velocity(g_cubit->state.spring, -400);
    if (g_cubit->state.spring->state != VENOM_ANIM_STATE_RUNNING)
        venom_spring_animation_start(g_cubit->state.spring);
    printf("🏀 Bounce!\n");
}

static void do_next_easing(void) {
    if (!g_cubit) return;
    AnimState* s = &g_cubit->state;
    s->easing_idx = (s->easing_idx + 1) % VENOM_EASING_COUNT;
    s->easing = (VenomEasing)s->easing_idx;
    if (s->x_anim) {
        venom_animation_set_easing(s->x_anim, s->easing);
        venom_animation_start(s->x_anim);
    }
    printf("🔄 Easing: %s\n", venom_easing_name(s->easing));
}

static void do_pause(void) {
    if (!g_cubit) return;
    AnimState* s = &g_cubit->state;
    s->paused = !s->paused;
    if (s->paused) {
        if (s->x_anim) venom_animation_pause(s->x_anim);
        if (s->y_anim) venom_animation_pause(s->y_anim);
        if (s->color_anim) venom_animation_pause(s->color_anim);
        printf("⏸️ Paused\n");
    } else {
        if (s->x_anim) venom_animation_resume(s->x_anim);
        if (s->y_anim) venom_animation_resume(s->y_anim);
        if (s->color_anim) venom_animation_resume(s->color_anim);
        printf("▶️ Resumed\n");
    }
}

/* ============================================================================
 * ANIMATION CANVAS WIDGET
 * ============================================================================ */

typedef struct AnimCanvas {
    VenomWidget base;
} AnimCanvas;

static void canvas_measure(VenomWidget* w, VenomF32 aw, VenomF32 ah, VenomF32* ow, VenomF32* oh) {
    (void)w; (void)aw; (void)ah;
    *ow = 480; *oh = 260;
}

static void canvas_draw(VenomWidget* widget, VenomCanvas* canvas) {
    if (!g_cubit) return;
    AnimState* s = &g_cubit->state;
    
    /* Delta time calculation */
    VenomF64 now = now_seconds();
    VenomF64 dt = now - s->last_time;
    s->last_time = now;
    
    /* FPS calculation */
    s->frame_count++;
    VenomF64 fps_elapsed = now - s->fps_time;
    if (fps_elapsed >= 1.0) {
        s->current_fps = (VenomF32)s->frame_count / (VenomF32)fps_elapsed;
        s->frame_count = 0;
        s->fps_time = now;
    }
    
    /* UPDATE ANIMATIONS */
    if (!s->paused) {
        if (s->x_anim && venom_animation_is_running(s->x_anim)) {
            venom_animation_update(s->x_anim, dt);
            s->box_x = venom_animation_get_value(s->x_anim);
        }
        if (s->y_anim && venom_animation_is_running(s->y_anim)) {
            venom_animation_update(s->y_anim, dt);
            s->box_y = venom_animation_get_value(s->y_anim);
        }
        if (s->color_anim && venom_animation_is_running(s->color_anim)) {
            venom_animation_update(s->color_anim, dt);
            s->color_t = venom_animation_get_value(s->color_anim);
        }
        if (s->spring && venom_spring_animation_is_running(s->spring)) {
            venom_spring_animation_update(s->spring, dt);
            s->spring_x = venom_spring_animation_get_value(s->spring);
        }
    }
    
    /* DRAW BACKGROUND */
    VenomRectF bg = {0, 0, widget->bounds.width, widget->bounds.height};
    VenomPaint bgp = venom_paint_fill(venom_color_rgb(25, 28, 38));
    venom_canvas_draw_rounded_rect(canvas, bg, 10, &bgp);
    
    /* Grid */
    VenomPaint grid = venom_paint_stroke(venom_color_rgba(70,70,90,60), 1);
    for (VenomF32 x = 50; x < widget->bounds.width; x += 50)
        venom_canvas_draw_line(canvas, x, 0, x, widget->bounds.height, &grid);
    for (VenomF32 y = 50; y < widget->bounds.height; y += 50)
        venom_canvas_draw_line(canvas, 0, y, widget->bounds.width, y, &grid);
    
    /* BOX 1: Tween with color */
    VenomColor c1 = venom_color_rgb(255, 87, 34);
    VenomColor c2 = venom_color_rgb(156, 39, 176);
    VenomColor col = venom_color_lerp(c1, c2, s->color_t);
    VenomRectF box1 = {s->box_x, s->box_y, 55, 55};
    VenomPaint p1 = venom_paint_fill(col);
    venom_canvas_draw_rounded_rect(canvas, box1, 8, &p1);
    VenomPaint txt = venom_paint_fill(VENOM_COLOR_WHITE);
    venom_canvas_draw_text(canvas, "Tween", s->box_x+5, s->box_y+33, NULL, &txt);
    
    /* BOX 2: Spring */
    VenomRectF box2 = {s->spring_x, 170, 45, 45};
    VenomPaint p2 = venom_paint_fill(venom_color_rgb(76, 175, 80));
    venom_canvas_draw_rounded_rect(canvas, box2, 22, &p2);
    venom_canvas_draw_text(canvas, "Spring", s->spring_x+1, 188, NULL, &txt);
    
    /* Circle following X */
    VenomPaint p3 = venom_paint_fill(venom_color_rgb(33, 150, 243));
    venom_canvas_draw_circle(canvas, s->box_x + 27, 230, 18, &p3);
    
    /* FPS display */
    char fps_str[32];
    snprintf(fps_str, sizeof(fps_str), "%.1f FPS", s->current_fps);
    VenomPaint fps_paint = venom_paint_fill(venom_color_rgb(0, 255, 100));
    venom_canvas_draw_text(canvas, fps_str, widget->bounds.width - 80, 20, NULL, &fps_paint);
    
    /* Info */
    char info[100];
    snprintf(info, sizeof(info), "%s | %s", 
             venom_easing_name(s->easing), s->paused ? "PAUSED" : "PLAYING");
    VenomPaint inf = venom_paint_fill(venom_color_rgb(180, 180, 200));
    venom_canvas_draw_text(canvas, info, 10, widget->bounds.height - 10, NULL, &inf);
    
    /* REQUEST NEXT FRAME - This keeps animations running! */
    widget->needs_redraw = VENOM_TRUE;
}

static VenomBool canvas_event(VenomWidget* w, const VenomEvent* e) {
    (void)w;
    if (e->type == VENOM_EVENT_MOUSE_BUTTON_DOWN) {
        do_bounce();
        do_next_easing();
        return VENOM_TRUE;
    }
    return VENOM_FALSE;
}

static const VenomWidgetClass anim_canvas_class = {
    .class_name = "AnimCanvas",
    .instance_size = sizeof(AnimCanvas),
    .parent_class = &venom_widget_class,
    .init = NULL,
    .destroy = NULL,
    .measure = canvas_measure,
    .layout = NULL,
    .draw = canvas_draw,
    .on_event = canvas_event,
    .on_state_changed = NULL,
};

static VenomWidget* anim_canvas_new(void) {
    VenomResultPtr r = venom_widget_create(&anim_canvas_class);
    return r.ok ? (VenomWidget*)r.value : NULL;
}

/* ============================================================================
 * BUTTON CALLBACKS
 * ============================================================================ */

static void on_pause_btn(VenomButton* b, void* d) { (void)b; (void)d; do_pause(); }
static void on_bounce_btn(VenomButton* b, void* d) { (void)b; (void)d; do_bounce(); }
static void on_easing_btn(VenomButton* b, void* d) { (void)b; (void)d; do_next_easing(); }

/* ============================================================================
 * UI BUILD
 * ============================================================================ */

static VenomWidget* build_ui(void* ud) {
    (void)ud;
    
    return venom_col(
        .padding = (VenomInsets){14,14,14,14},
        .gap = 10,
        .background = venom_color_rgb(35, 38, 48),
        .children = (VenomWidget*[]){
            venom_text("🎬 VENOMUI Animation Demo", 
                .size = 18, .color = venom_color_rgb(255,255,255)),
            
            venom_row(
                .gap = 8,
                .children = (VenomWidget*[]){
                    venom_btn("⏯️ Pause", .on_click = on_pause_btn, 
                              .color = venom_color_rgb(76, 175, 80)),
                    venom_btn("🏀 Bounce", .on_click = on_bounce_btn, 
                              .color = venom_color_rgb(255, 152, 0)),
                    venom_btn("🔄 Easing", .on_click = on_easing_btn, 
                              .color = venom_color_rgb(33, 150, 243)),
                    NULL
                }
            ),
            
            anim_canvas_new(),
            
            venom_text("Click canvas or buttons • ESC to exit",
                .size = 11, .color = venom_color_rgb(130, 130, 150)),
            NULL
        }
    );
}

/* ============================================================================
 * MAIN
 * ============================================================================ */

int main(void) {
    printf("=== VENOMUI Animation Demo ===\n");
    printf("Using VenomCubit + needs_redraw pattern\n\n");
    
    init_cubit();
    
    int ret = VENOM_APP(
        .title = "VENOMUI Animation Demo",
        .width = 530,
        .height = 380,
        .background = venom_color_rgb(35, 38, 48),
        .build = build_ui,
        .debug = VENOM_TRUE
    );
    
    cleanup_cubit();
    
    printf("\nBye!\n");
    return ret;
}
