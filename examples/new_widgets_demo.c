/*
 * VAXPUI - Complete Demo using Column Layout
 */
#include <vaxp/vaxpui.h>
#include <stdio.h>

static VaxpI32 g_page = 0;

static void on_nav(VaxpI32 idx, void* d) {
    (void)d;
    printf("Page: %d\n", idx);
    g_page = idx;
    vaxp_rebuild();
}

/* Table */
static VaxpWidget* make_table(void) {
    VaxpTableColumn cols[] = {
        { .key = "id", .title = "ID", .width = 50 },
        { .key = "name", .title = "Name", .flex = 1 },
        { .key = "status", .title = "Status", .width = 100 },
    };
    VaxpResultPtr r = vaxp_table_create(cols, 3);
    if (!r.ok) return NULL;
    VaxpTable* t = (VaxpTable*)r.value;
    
    const char* r1[] = {"1", "Ahmed", "Active"};
    const char* r2[] = {"2", "Sara", "Pending"};
    const char* r3[] = {"3", "Ali", "Active"};
    vaxp_table_add_row(t, r1, 3);
    vaxp_table_add_row(t, r2, 3);
    vaxp_table_add_row(t, r3, 3);
    
    /* Set flex_grow so it takes remaining space */
    ((VaxpWidget*)t)->layout.flex_grow = 1;
    
    return (VaxpWidget*)t;
}

/* Chart */
static VaxpWidget* make_chart(void) {
    VaxpResultPtr r = vaxp_chart_create(VAXP_CHART_BAR);
    if (!r.ok) return NULL;
    VaxpChart* c = (VaxpChart*)r.value;
    
    static const char* labels[] = {"A", "B", "C", "D"};
    static VaxpF32 vals[] = {30, 60, 45, 80};
    vaxp_chart_set_labels(c, labels, 4);
    VaxpDataset ds = { .label = "Data", .values = vals, .value_count = 4, .color = vaxp_color_rgb(80, 140, 220) };
    vaxp_chart_add_dataset(c, &ds);
    
    ((VaxpWidget*)c)->layout.flex_grow = 1;
    
    return (VaxpWidget*)c;
}

/* Carousel */
static VaxpWidget* make_carousel(void) {
    VaxpResultPtr r = vaxp_carousel_create();
    if (!r.ok) return NULL;
    VaxpCarousel* carousel = (VaxpCarousel*)r.value;
    
    VaxpResultPtr s1 = vaxp_container_create();
    if (s1.ok) {
        vaxp_container_set_background((VaxpContainer*)s1.value, vaxp_color_rgb(60, 120, 220));
        vaxp_carousel_add_item(carousel, (VaxpWidget*)s1.value);
    }
    VaxpResultPtr s2 = vaxp_container_create();
    if (s2.ok) {
        vaxp_container_set_background((VaxpContainer*)s2.value, vaxp_color_rgb(60, 180, 100));
        vaxp_carousel_add_item(carousel, (VaxpWidget*)s2.value);
    }
    VaxpResultPtr s3 = vaxp_container_create();
    if (s3.ok) {
        vaxp_container_set_background((VaxpContainer*)s3.value, vaxp_color_rgb(240, 140, 60));
        vaxp_carousel_add_item(carousel, (VaxpWidget*)s3.value);
    }
    
    vaxp_carousel_set_auto_play(carousel, VAXP_TRUE, 2000);
    ((VaxpWidget*)carousel)->layout.flex_grow = 1;
    
    return (VaxpWidget*)carousel;
}

/* Build with Column layout */
static VaxpWidget* build(void* d) {
    (void)d;
    
    /* Use Column instead of Stack */
    VaxpResultPtr cr = vaxp_container_create_column();
    if (!cr.ok) return NULL;
    VaxpContainer* col = (VaxpContainer*)cr.value;
    vaxp_container_set_gap(col, 0);
    
    /* AppBar */
    VaxpResultPtr ar = vaxp_appbar_create("VAXPUI Demo");
    if (ar.ok) {
        VaxpAppBar* bar = (VaxpAppBar*)ar.value;
        vaxp_appbar_set_subtitle(bar, "All Widgets");
        vaxp_appbar_set_background(bar, vaxp_color_rgb(55, 65, 85));
        VaxpResult res = vaxp_widget_add_child((VaxpWidget*)col, (VaxpWidget*)bar);
        printf("Add AppBar: %s, col children: %u\n", res.ok ? "OK" : "FAIL", ((VaxpWidget*)col)->children_count);
    }
    
    /* Content */
    VaxpWidget* content = NULL;
    switch (g_page) {
        case 0: content = make_table(); break;
        case 1: content = make_chart(); break;
        case 2: content = make_carousel(); break;
    }
    if (content) {
        VaxpResult res = vaxp_widget_add_child((VaxpWidget*)col, content);
        printf("Add Content: %s, col children: %u\n", res.ok ? "OK" : "FAIL", ((VaxpWidget*)col)->children_count);
    }
    
    /* BottomNav */
    VaxpNavItem items[] = {
        { .icon = "T", .label = "Table" },
        { .icon = "C", .label = "Chart" },
        { .icon = "S", .label = "Slides" },
    };
    VaxpResultPtr nr = vaxp_bottom_nav_create(items, 3);
    if (nr.ok) {
        VaxpBottomNav* nav = (VaxpBottomNav*)nr.value;
        vaxp_bottom_nav_set_selected(nav, g_page);
        vaxp_bottom_nav_set_on_change(nav, on_nav, NULL);
        VaxpResult res = vaxp_widget_add_child((VaxpWidget*)col, (VaxpWidget*)nav);
        printf("Add BottomNav: %s, col children: %u\n", res.ok ? "OK" : "FAIL", ((VaxpWidget*)col)->children_count);
    }
    
    printf("==> Column has %u children\n", ((VaxpWidget*)col)->children_count);
    
    return (VaxpWidget*)col;
}

int main(void) {
    setbuf(stdout, NULL);  /* Disable buffering */
    printf("=== VAXPUI Demo (Column Layout) ===\n");
    
    VaxpAppConfig cfg = {
        .title = "Complete Demo",
        .width = 500,
        .height = 600,
        .background = vaxp_color_rgb(240, 242, 245),
        .build = build,
    };
    
    return vaxp_run_app(&cfg);
}
