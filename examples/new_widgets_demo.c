/*
 * VENOMUI - Complete Demo using Column Layout
 */
#include <venom/venomui.h>
#include <stdio.h>

static VenomI32 g_page = 0;

static void on_nav(VenomI32 idx, void* d) {
    (void)d;
    printf("Page: %d\n", idx);
    g_page = idx;
    venom_rebuild();
}

/* Table */
static VenomWidget* make_table(void) {
    VenomTableColumn cols[] = {
        { .key = "id", .title = "ID", .width = 50 },
        { .key = "name", .title = "Name", .flex = 1 },
        { .key = "status", .title = "Status", .width = 100 },
    };
    VenomResultPtr r = venom_table_create(cols, 3);
    if (!r.ok) return NULL;
    VenomTable* t = (VenomTable*)r.value;
    
    const char* r1[] = {"1", "Ahmed", "Active"};
    const char* r2[] = {"2", "Sara", "Pending"};
    const char* r3[] = {"3", "Ali", "Active"};
    venom_table_add_row(t, r1, 3);
    venom_table_add_row(t, r2, 3);
    venom_table_add_row(t, r3, 3);
    
    /* Set flex_grow so it takes remaining space */
    ((VenomWidget*)t)->layout.flex_grow = 1;
    
    return (VenomWidget*)t;
}

/* Chart */
static VenomWidget* make_chart(void) {
    VenomResultPtr r = venom_chart_create(VENOM_CHART_BAR);
    if (!r.ok) return NULL;
    VenomChart* c = (VenomChart*)r.value;
    
    static const char* labels[] = {"A", "B", "C", "D"};
    static VenomF32 vals[] = {30, 60, 45, 80};
    venom_chart_set_labels(c, labels, 4);
    VenomDataset ds = { .label = "Data", .values = vals, .value_count = 4, .color = venom_color_rgb(80, 140, 220) };
    venom_chart_add_dataset(c, &ds);
    
    ((VenomWidget*)c)->layout.flex_grow = 1;
    
    return (VenomWidget*)c;
}

/* Carousel */
static VenomWidget* make_carousel(void) {
    VenomResultPtr r = venom_carousel_create();
    if (!r.ok) return NULL;
    VenomCarousel* carousel = (VenomCarousel*)r.value;
    
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
    
    venom_carousel_set_auto_play(carousel, VENOM_TRUE, 2000);
    ((VenomWidget*)carousel)->layout.flex_grow = 1;
    
    return (VenomWidget*)carousel;
}

/* Build with Column layout */
static VenomWidget* build(void* d) {
    (void)d;
    
    /* Use Column instead of Stack */
    VenomResultPtr cr = venom_container_create_column();
    if (!cr.ok) return NULL;
    VenomContainer* col = (VenomContainer*)cr.value;
    venom_container_set_gap(col, 0);
    
    /* AppBar */
    VenomResultPtr ar = venom_appbar_create("VENOMUI Demo");
    if (ar.ok) {
        VenomAppBar* bar = (VenomAppBar*)ar.value;
        venom_appbar_set_subtitle(bar, "All Widgets");
        venom_appbar_set_background(bar, venom_color_rgb(55, 65, 85));
        VenomResult res = venom_widget_add_child((VenomWidget*)col, (VenomWidget*)bar);
        printf("Add AppBar: %s, col children: %u\n", res.ok ? "OK" : "FAIL", ((VenomWidget*)col)->children_count);
    }
    
    /* Content */
    VenomWidget* content = NULL;
    switch (g_page) {
        case 0: content = make_table(); break;
        case 1: content = make_chart(); break;
        case 2: content = make_carousel(); break;
    }
    if (content) {
        VenomResult res = venom_widget_add_child((VenomWidget*)col, content);
        printf("Add Content: %s, col children: %u\n", res.ok ? "OK" : "FAIL", ((VenomWidget*)col)->children_count);
    }
    
    /* BottomNav */
    VenomNavItem items[] = {
        { .icon = "T", .label = "Table" },
        { .icon = "C", .label = "Chart" },
        { .icon = "S", .label = "Slides" },
    };
    VenomResultPtr nr = venom_bottom_nav_create(items, 3);
    if (nr.ok) {
        VenomBottomNav* nav = (VenomBottomNav*)nr.value;
        venom_bottom_nav_set_selected(nav, g_page);
        venom_bottom_nav_set_on_change(nav, on_nav, NULL);
        VenomResult res = venom_widget_add_child((VenomWidget*)col, (VenomWidget*)nav);
        printf("Add BottomNav: %s, col children: %u\n", res.ok ? "OK" : "FAIL", ((VenomWidget*)col)->children_count);
    }
    
    printf("==> Column has %u children\n", ((VenomWidget*)col)->children_count);
    
    return (VenomWidget*)col;
}

int main(void) {
    setbuf(stdout, NULL);  /* Disable buffering */
    printf("=== VENOMUI Demo (Column Layout) ===\n");
    
    VenomAppConfig cfg = {
        .title = "Complete Demo",
        .width = 500,
        .height = 600,
        .background = venom_color_rgb(240, 242, 245),
        .build = build,
    };
    
    return venom_run_app(&cfg);
}
