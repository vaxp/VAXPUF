/*
 * VAXPUI - Decoration Demo
 *
 * يوضّح هذا المثال كيفية استخدام شريط الزخرفة الافتراضي (macOS-style)
 * الذي يُضاف تلقائياً بمجرد تمرير .decoration = VAXP_DECORATION_DEFAULT
 * (أو VAXP_DECORATION_DARK) في إعدادات التطبيق.
 */

#include <stdio.h>
#include <vaxp/vaxpui.h>

/* ============================================================================
 * BUILD UI
 * ============================================================================ */

static VaxpWidget* build_ui(void* ud) {
    (void)ud;

    /* ألوان */
    VaxpColor bg_card   = vaxp_color_rgba(255, 255, 255, 200);
    VaxpColor txt_dark  = vaxp_color_rgb(40, 40, 50);
    VaxpColor txt_muted = vaxp_color_rgb(120, 120, 130);
    VaxpColor accent    = vaxp_color_rgb(99, 102, 241);   /* Indigo-500 */
    VaxpColor btn_bg    = vaxp_color_rgba(99, 102, 241, 240);

    /* --- بطاقة المحتوى --- */
    VaxpWidget* heading = _vaxp_text_build(
        "Hello, VAXPUF!",
        &(VaxpTextConfig){ .size = 28, .color = txt_dark }
    );

    VaxpWidget* subtitle = _vaxp_text_build(
        "شريط الزخرفة يُنشأ تلقائياً.",
        &(VaxpTextConfig){ .size = 14, .color = txt_muted }
    );

    VaxpWidget* note = _vaxp_text_build(
        ".decoration = VAXP_DECORATION_DEFAULT",
        &(VaxpTextConfig){ .size = 12, .color = accent }
    );

    VaxpWidget* ok_btn = _vaxp_btn_build(
        "حسناً!",
        &(VaxpButtonConfig){
            .color = btn_bg,
            .text_color = vaxp_color_rgb(255, 255, 255),
            .corner_radius = 10
        }
    );

    /* NOTE: _vaxp_col_build consumes a ref from each child it receives.
     * Do NOT call vaxp_unref on heading/subtitle/note/ok_btn after this call. */
    VaxpWidget* card = _vaxp_col_build(&(VaxpContainerConfig){
        .gap          = 14,
        .background   = bg_card,
        .corner_radius = 16,
        .padding      = (VaxpInsets){28, 28, 28, 28},
        .children     = (VaxpWidget*[]){ heading, subtitle, note, ok_btn, NULL },
        .child_count  = 4
    });
    /* heading/subtitle/note/ok_btn refs consumed */

    /* --- حاوية مركزية --- */
    VaxpWidget* center = _vaxp_col_build(&(VaxpContainerConfig){
        .justify    = VAXP_JUSTIFY_CENTER,
        .align      = VAXP_ALIGN_CENTER,
        .background = vaxp_color_rgba(240, 242, 255, 200),
        .children   = (VaxpWidget*[]){ card, NULL },
        .child_count = 1
    });
    /* card ref consumed */

    return center;
}

/* ============================================================================
 * MAIN
 * ============================================================================ */

int main(void) {
    return VAXP_APP(
        .title      = "Decoration Demo",
        .width      = 500,
        .height     = 420,
        .background = vaxp_color_rgba(200, 200, 220, 180),
        .build      = build_ui,
        /* ← هنا يتم تفعيل شريط الزخرفة الافتراضي */
        .decoration = VAXP_DECORATION_DEFAULT
    );
}
