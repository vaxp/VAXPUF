/*
 * VAXPUI Calculator Demo
 * Tests state management (Cubit), grid layouts, and dynamic updates.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <vaxp/vaxpui.h>

/* ============================================================================
 * STATE MANAGEMENT (Cubit)
 * ============================================================================ */

typedef struct {
    char display[64];
    double operand1;
    char operator;
    VaxpBool new_number; // True if next digit should clear the display
} CalcState;

VAXP_DEFINE_CUBIT(Calc, CalcState);
static CalcCubit* g_cubit = NULL;

/* ============================================================================
 * HELPER FOR FLEX LAYOUT
 * ============================================================================ */

static VaxpWidget* vaxp_flex(VaxpWidget* widget, VaxpF32 grow) {
    if (widget) {
        widget->layout.flex_grow = grow;
        widget->layout.flex_shrink = 1;
    }
    return widget;
}

/* ============================================================================
 * CALCULATOR LOGIC
 * ============================================================================ */

static void do_digit(int digit) {
    CalcState s = g_cubit->state;
    
    if (s.new_number) {
        snprintf(s.display, sizeof(s.display), "%d", digit);
        s.new_number = VAXP_FALSE;
    } else {
        if (strcmp(s.display, "0") == 0) {
            snprintf(s.display, sizeof(s.display), "%d", digit);
        } else {
            int len = strlen(s.display);
            if (len < sizeof(s.display) - 2) {
                s.display[len] = '0' + digit;
                s.display[len+1] = '\0';
            }
        }
    }
    
    vaxp_cubit_emit_raw(g_cubit, &s);
    vaxp_rebuild();
}

static void do_dot(void) {
    CalcState s = g_cubit->state;
    if (s.new_number) {
        strcpy(s.display, "0.");
        s.new_number = VAXP_FALSE;
    } else if (strchr(s.display, '.') == NULL) {
        strcat(s.display, ".");
    }
    vaxp_cubit_emit_raw(g_cubit, &s);
    vaxp_rebuild();
}

static void do_operator(char op) {
    CalcState s = g_cubit->state;
    
    // If we already have an operator and we're not waiting for a new number, calculate first
    if (s.operator != '\0' && !s.new_number) {
        double operand2 = atof(s.display);
        double result = 0;
        if (s.operator == '+') result = s.operand1 + operand2;
        else if (s.operator == '-') result = s.operand1 - operand2;
        else if (s.operator == '*') result = s.operand1 * operand2;
        else if (s.operator == '/') result = operand2 != 0 ? s.operand1 / operand2 : 0;
        
        snprintf(s.display, sizeof(s.display), "%.6g", result);
        s.operand1 = result;
    } else {
        s.operand1 = atof(s.display);
    }
    
    s.operator = op;
    s.new_number = VAXP_TRUE;
    
    vaxp_cubit_emit_raw(g_cubit, &s);
    vaxp_rebuild();
}

static void do_equals(void) {
    CalcState s = g_cubit->state;
    
    if (s.operator == '\0') return; // Nothing to calculate
    
    double operand2 = atof(s.display);
    double result = 0;
    
    if (s.operator == '+') result = s.operand1 + operand2;
    else if (s.operator == '-') result = s.operand1 - operand2;
    else if (s.operator == '*') result = s.operand1 * operand2;
    else if (s.operator == '/') result = operand2 != 0 ? s.operand1 / operand2 : 0;
    
    snprintf(s.display, sizeof(s.display), "%.6g", result);
    s.operator = '\0';
    s.new_number = VAXP_TRUE;
    s.operand1 = result;
    
    vaxp_cubit_emit_raw(g_cubit, &s);
    vaxp_rebuild();
}

static void do_clear(void) {
    CalcState s = g_cubit->state;
    strcpy(s.display, "0");
    s.operand1 = 0;
    s.operator = '\0';
    s.new_number = VAXP_TRUE;
    vaxp_cubit_emit_raw(g_cubit, &s);
    vaxp_rebuild();
}

static void do_plus_minus(void) {
    CalcState s = g_cubit->state;
    if (s.display[0] == '-') {
        memmove(s.display, s.display + 1, strlen(s.display));
    } else if (strcmp(s.display, "0") != 0) {
        char temp[64];
        snprintf(temp, sizeof(temp), "-%s", s.display);
        strcpy(s.display, temp);
    }
    vaxp_cubit_emit_raw(g_cubit, &s);
    vaxp_rebuild();
}

static void do_percent(void) {
    CalcState s = g_cubit->state;
    double val = atof(s.display);
    snprintf(s.display, sizeof(s.display), "%.6g", val / 100.0);
    s.new_number = VAXP_TRUE;
    vaxp_cubit_emit_raw(g_cubit, &s);
    vaxp_rebuild();
}

/* ============================================================================
 * BUTTON CALLBACKS
 * ============================================================================ */

static void cb_digit(VaxpButton* b, void* d) { (void)b; do_digit((int)(intptr_t)d); }
static void cb_operator(VaxpButton* b, void* d) { (void)b; do_operator((char)(intptr_t)d); }
static void cb_equals(VaxpButton* b, void* d) { (void)b; (void)d; do_equals(); }
static void cb_clear(VaxpButton* b, void* d) { (void)b; (void)d; do_clear(); }
static void cb_dot(VaxpButton* b, void* d) { (void)b; (void)d; do_dot(); }
static void cb_pm(VaxpButton* b, void* d) { (void)b; (void)d; do_plus_minus(); }
static void cb_pct(VaxpButton* b, void* d) { (void)b; (void)d; do_percent(); }

/* ============================================================================
 * BUILD UI
 * ============================================================================ */

static VaxpWidget* build_ui(void* ud) {
    (void)ud;
    CalcState* s = &g_cubit->state;
    
    /* Theme Colors (Exact match from Flutter) */
    VaxpColor bg_color = vaxp_color_rgba(0, 0, 0, 0); /* Transparent */
    VaxpColor btn_num = vaxp_color_rgba(142, 142, 147, 25);  /* systemGrey with 0.1 alpha */
    VaxpColor btn_top = vaxp_color_rgb(142, 142, 147);       /* systemGrey */
    VaxpColor btn_op = vaxp_color_rgb(57, 0, 104);           /* Solid Purple */
    VaxpColor text_white = vaxp_color_rgba(255, 255, 255, 255); /* ARGB(221,255,255,255) */
    
    /* Helper macros for buttons */
    #define NUM_BTN(label, val) \
        vaxp_flex(vaxp_btn(label, .font_family = "Ubuntu", .size = 24, .color = btn_num, .text_color = text_white, .corner_radius = 12, .on_click = cb_digit, .on_click_data = (void*)(intptr_t)val), 1)
    
    #define TOP_BTN(label, cb) \
        vaxp_flex(vaxp_btn(label, .font_family = "Ubuntu", .size = 22, .color = btn_top, .text_color = text_white, .corner_radius = 12, .on_click = cb), 1)
        
    #define OP_BTN(label, op) \
        vaxp_flex(vaxp_btn(label, .font_family = "Ubuntu", .size = 28, .color = btn_op, .text_color = text_white, .corner_radius = 12, .on_click = cb_operator, .on_click_data = (void*)(intptr_t)op), 1)

    return vaxp_col(
        .align = VAXP_ALIGN_STRETCH,
        .background = bg_color,
        .children = VAXP_CHILDREN(
            
            /* =========================================================
             * 1. منطقة الشاشة (Display Area)
             * HOW TO ADJUST DISPLAY HEIGHT (كيفية تعديل ارتفاع الشاشة):
             * الرقم 2 في نهاية هذه الدالة (vaxp_flex(..., 2)) يمثل نسبة الارتفاع.
             * يمكنك تكبير هذا الرقم (مثلاً 3 أو 4) لزيادة المساحة العلوية للرقم 0.
             * ========================================================= */
            vaxp_flex(
                vaxp_col(
                    .justify = VAXP_JUSTIFY_END,
                    .align = VAXP_ALIGN_END,
                    .padding = (VaxpInsets){24, 24, 24, 24},
                    .children = VAXP_CHILDREN(
                        vaxp_text(" ", .size = 22, .color = vaxp_color_rgba(255, 255, 255, 255)),
                        /* 
                         * HOW TO ADJUST RESULT TEXT SIZE (حجم نص النتيجة أو الرقم):
                         * يمكنك تغيير الـ .size = 52 إلى أي رقم تريده لتكبير الرقم.
                         */
                        vaxp_text(s->display, .font_family = "Ubuntu", .size = 42, .color = VAXP_COLOR_WHITE)
                    )
                ), 2 /* <--- نسبة الارتفاع للشاشة (غيرها لتكبير أو تصغير المساحة) */
            ),
            
            /* =========================================================
             * 2. منطقة الأزرار (Keypad Area)
             * HOW TO ADJUST KEYPAD HEIGHT (كيفية تعديل ارتفاع الأزرار):
             * الرقم 3 في نهاية الـ vaxp_flex أدناه يمثل نسبة ارتفاع الأزرار.
             * التناسب الحالي هو (الشاشة: 2) إلى (الأزرار: 3).
             * ========================================================= */
            vaxp_flex(
                vaxp_col(
                    .padding = (VaxpInsets){12, 8, 12, 8}, /* horizontal: 8, vertical: 12 */
                    .gap = 12, /* المسافة العمودية بين الصفوف */
                    .align = VAXP_ALIGN_STRETCH,
                    .children = VAXP_CHILDREN(
                        /* 
                         * HOW TO ADJUST ROW HEIGHT (كيفية تعديل ارتفاع الصف الواحد):
                         * الـ vaxp_flex(..., 1) تجعل الصف يتمدد عمودياً بالتساوي.
                         * إذا أردت صفاً أطول من غيره، اجعل الرقم 2 مثلاً.
                         */
                        /* Row 1: AC, +/-, %, ÷ */
                        vaxp_flex(vaxp_row(.gap = 12, .align = VAXP_ALIGN_STRETCH, .children = VAXP_CHILDREN(
                            TOP_BTN("AC", cb_clear), TOP_BTN("+/-", cb_pm), TOP_BTN("%", cb_pct), OP_BTN("÷", '/')
                        )), 1),
                        
                        /* Row 2: 7, 8, 9, × */
                        vaxp_flex(vaxp_row(.gap = 12, .align = VAXP_ALIGN_STRETCH, .children = VAXP_CHILDREN(
                            NUM_BTN("7", 7), NUM_BTN("8", 8), NUM_BTN("9", 9), OP_BTN("×", '*')
                        )), 1),
                        
                        /* Row 3: 4, 5, 6, - */
                        vaxp_flex(vaxp_row(.gap = 12, .align = VAXP_ALIGN_STRETCH, .children = VAXP_CHILDREN(
                            NUM_BTN("4", 4), NUM_BTN("5", 5), NUM_BTN("6", 6), OP_BTN("-", '-')
                        )), 1),
                        
                        /* Row 4: 1, 2, 3, + */
                        vaxp_flex(vaxp_row(.gap = 12, .align = VAXP_ALIGN_STRETCH, .children = VAXP_CHILDREN(
                            NUM_BTN("1", 1), NUM_BTN("2", 2), NUM_BTN("3", 3), OP_BTN("+", '+')
                        )), 1),
                        
                        /* Row 5: 0, ., = */
                        vaxp_flex(vaxp_row(.gap = 12, .align = VAXP_ALIGN_STRETCH, .children = VAXP_CHILDREN(
                            /* 
                             * HOW TO ADJUST BUTTON WIDTH (كيفية تعديل عرض الزر الواحد):
                             * زر الـ "0" يأخذ vaxp_flex(..., 2) لكي يكون عرضه ضعف الأزرار الأخرى.
                             */
                            vaxp_flex(vaxp_btn("0", .font_family = "Ubuntu", .color = btn_num, .text_color = text_white, .corner_radius = 12, .on_click = cb_digit, .on_click_data = (void*)(intptr_t)0), 2),
                            vaxp_flex(vaxp_btn(".", .font_family = "Ubuntu", .color = btn_num, .text_color = text_white, .corner_radius = 12, .on_click = cb_dot), 1),
                            vaxp_flex(vaxp_btn("=", .font_family = "Ubuntu", .color = btn_op, .text_color = text_white, .corner_radius = 12, .on_click = cb_equals), 1)
                        )), 1)
                    )
                ), 3 /* <--- نسبة الارتفاع لمنطقة الأزرار مقارنة بالشاشة */
            )
        )
    );
}

/* ============================================================================
 * MAIN
 * ============================================================================ */

int main(void) {
    g_cubit = VAXP_CUBIT_CREATE(Calc, CalcState, { 
        .display = "0", 
        .operand1 = 0, 
        .operator = '\0', 
        .new_number = VAXP_TRUE 
    });
    
    if (!g_cubit) return 1;
    
    int ret = VAXP_APP(
        .title = "Calculator",
        .width = 405,
        .height = 700,
        .background = vaxp_color_rgba(0, 0, 0, 100), /* Scaffold background opacity */
        .decoration = VAXP_DECORATION_DARK,          /* Use native dark decoration bar */
        .build = build_ui
    );
    
    vaxp_cubit_destroy(g_cubit);
    return ret;
}
