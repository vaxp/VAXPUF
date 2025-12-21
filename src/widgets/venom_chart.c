/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_chart.c - Chart/Graph Widget implementation
 */

#include "venom/widgets/venom_chart.h"
#include "venom/core/venom_memory.h"
#include "venom/graphics/venom_canvas.h"
#include <string.h>
#include <math.h>
#include <stdio.h>

/* Dimensions */
#define CHART_PADDING 40.0f
#define CHART_LEGEND_HEIGHT 30.0f
#define CHART_TITLE_HEIGHT 30.0f
#define CHART_POINT_RADIUS 4.0f
#define CHART_BAR_GAP 4.0f

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* ============================================================================
 * FORWARD DECLARATIONS
 * ============================================================================ */

static void chart_init(VenomWidget* widget);
static void chart_destroy(VenomWidget* widget);
static void chart_measure(VenomWidget* widget, VenomF32 aw, VenomF32 ah,
                           VenomF32* ow, VenomF32* oh);
static void chart_layout(VenomWidget* widget, VenomRectF bounds);
static void chart_draw(VenomWidget* widget, VenomCanvas* canvas);

/* ============================================================================
 * CLASS DEFINITION
 * ============================================================================ */

const VenomWidgetClass venom_chart_class = {
    .class_name = "VenomChart",
    .instance_size = sizeof(VenomChart),
    .parent_class = &venom_widget_class,
    .init = chart_init,
    .destroy = chart_destroy,
    .measure = chart_measure,
    .layout = chart_layout,
    .draw = chart_draw,
    .on_event = NULL,
    .on_state_changed = NULL,
};

/* ============================================================================
 * HELPERS
 * ============================================================================ */

static void calculate_range(VenomChart* chart) {
    if (!chart->auto_scale) return;
    
    chart->min_value = 0;
    chart->max_value = 0;
    
    for (VenomU32 d = 0; d < chart->dataset_count; d++) {
        for (VenomU32 i = 0; i < chart->datasets[d].value_count; i++) {
            VenomF32 v = chart->datasets[d].values[i];
            if (v > chart->max_value) chart->max_value = v;
            if (v < chart->min_value) chart->min_value = v;
        }
    }
    
    /* Add padding to max */
    chart->max_value *= 1.1f;
    if (chart->max_value == 0) chart->max_value = 100;
}

/* ============================================================================
 * LIFECYCLE
 * ============================================================================ */

static void chart_init(VenomWidget* widget) {
    VenomChart* chart = (VenomChart*)widget;
    
    chart->type = VENOM_CHART_LINE;
    chart->labels = NULL;
    chart->label_count = 0;
    chart->datasets = NULL;
    chart->dataset_count = 0;
    
    chart->background = VENOM_COLOR_WHITE;
    chart->grid_color = venom_color_rgb(230, 230, 235);
    chart->text_color = venom_color_rgb(100, 100, 110);
    chart->show_grid = VENOM_TRUE;
    chart->show_legend = VENOM_TRUE;
    chart->show_labels = VENOM_TRUE;
    chart->show_values = VENOM_FALSE;
    chart->animate = VENOM_TRUE;
    chart->anim_progress = 0;
    chart->padding = CHART_PADDING;
    chart->title = NULL;
    chart->min_value = 0;
    chart->max_value = 100;
    chart->auto_scale = VENOM_TRUE;
}

static void chart_destroy(VenomWidget* widget) {
    VenomChart* chart = (VenomChart*)widget;
    
    if (chart->datasets) {
        for (VenomU32 i = 0; i < chart->dataset_count; i++) {
            if (chart->datasets[i].values) {
                venom_free(chart->datasets[i].values, 
                          chart->datasets[i].value_count * sizeof(VenomF32));
            }
        }
        venom_free(chart->datasets, chart->dataset_count * sizeof(VenomDataset));
        chart->datasets = NULL;
    }
    chart->dataset_count = 0;
    
    venom_widget_class.destroy(widget);
}

/* ============================================================================
 * LAYOUT
 * ============================================================================ */

static void chart_measure(VenomWidget* widget, VenomF32 aw, VenomF32 ah,
                           VenomF32* ow, VenomF32* oh) {
    (void)widget;
    *ow = aw > 0 ? aw : 300;
    *oh = ah > 0 ? ah : 200;
}

static void chart_layout(VenomWidget* widget, VenomRectF bounds) {
    widget->bounds = bounds;
}

/* ============================================================================
 * DRAWING - LINE CHART
 * ============================================================================ */

static void draw_line_chart(VenomChart* chart, VenomCanvas* canvas, VenomRectF area) {
    if (chart->dataset_count == 0 || chart->label_count == 0) return;
    
    VenomF32 range = chart->max_value - chart->min_value;
    if (range == 0) range = 1;
    
    VenomF32 step_x = area.width / (VenomF32)(chart->label_count > 1 ? chart->label_count - 1 : 1);
    
    for (VenomU32 d = 0; d < chart->dataset_count; d++) {
        VenomDataset* ds = &chart->datasets[d];
        if (ds->value_count == 0) continue;
        
        VenomColor line_color = ds->color;
        VenomF32 line_width = ds->line_width > 0 ? ds->line_width : 2.0f;
        
        /* Draw area fill if type is AREA */
        if (chart->type == VENOM_CHART_AREA) {
            /* Simple fill approximation */
            for (VenomU32 i = 0; i < ds->value_count && i < chart->label_count; i++) {
                VenomF32 x = area.x + i * step_x;
                VenomF32 val = ds->values[i] * chart->anim_progress;
                VenomF32 y = area.y + area.height - (val - chart->min_value) / range * area.height;
                
                VenomRectF fill = {x - step_x/2, y, step_x, area.y + area.height - y};
                VenomColor fill_color = ds->fill_color.a > 0 ? ds->fill_color : 
                                        venom_color_rgba(line_color.r, line_color.g, line_color.b, 50);
                VenomPaint fp = venom_paint_fill(fill_color);
                venom_canvas_draw_rect(canvas, fill, &fp);
            }
        }
        
        /* Draw line segments */
        for (VenomU32 i = 1; i < ds->value_count && i < chart->label_count; i++) {
            VenomF32 x1 = area.x + (i - 1) * step_x;
            VenomF32 x2 = area.x + i * step_x;
            VenomF32 v1 = ds->values[i - 1] * chart->anim_progress;
            VenomF32 v2 = ds->values[i] * chart->anim_progress;
            VenomF32 y1 = area.y + area.height - (v1 - chart->min_value) / range * area.height;
            VenomF32 y2 = area.y + area.height - (v2 - chart->min_value) / range * area.height;
            
            VenomPaint lp = venom_paint_stroke(line_color, line_width);
            venom_canvas_draw_line(canvas, x1, y1, x2, y2, &lp);
        }
        
        /* Draw points */
        if (ds->show_points) {
            for (VenomU32 i = 0; i < ds->value_count && i < chart->label_count; i++) {
                VenomF32 x = area.x + i * step_x;
                VenomF32 val = ds->values[i] * chart->anim_progress;
                VenomF32 y = area.y + area.height - (val - chart->min_value) / range * area.height;
                
                VenomPaint pp = venom_paint_fill(line_color);
                venom_canvas_draw_circle(canvas, x, y, CHART_POINT_RADIUS, &pp);
            }
        }
    }
}

/* ============================================================================
 * DRAWING - BAR CHART
 * ============================================================================ */

static void draw_bar_chart(VenomChart* chart, VenomCanvas* canvas, VenomRectF area) {
    if (chart->dataset_count == 0 || chart->label_count == 0) return;
    
    VenomF32 range = chart->max_value - chart->min_value;
    if (range == 0) range = 1;
    
    VenomF32 group_width = area.width / (VenomF32)chart->label_count;
    VenomF32 bar_width = (group_width - CHART_BAR_GAP * 2) / (VenomF32)chart->dataset_count;
    
    for (VenomU32 i = 0; i < chart->label_count; i++) {
        VenomF32 group_x = area.x + i * group_width + CHART_BAR_GAP;
        
        for (VenomU32 d = 0; d < chart->dataset_count; d++) {
            VenomDataset* ds = &chart->datasets[d];
            if (i >= ds->value_count) continue;
            
            VenomF32 val = ds->values[i] * chart->anim_progress;
            VenomF32 bar_height = (val - chart->min_value) / range * area.height;
            
            VenomRectF bar = {
                group_x + d * bar_width,
                area.y + area.height - bar_height,
                bar_width - 2,
                bar_height
            };
            
            VenomPaint bp = venom_paint_fill(ds->color);
            venom_canvas_draw_rounded_rect(canvas, bar, 3, &bp);
        }
    }
}

/* ============================================================================
 * DRAWING - PIE/DONUT CHART
 * ============================================================================ */

static void draw_pie_chart(VenomChart* chart, VenomCanvas* canvas, VenomRectF area, VenomBool donut) {
    if (chart->dataset_count == 0) return;
    
    VenomDataset* ds = &chart->datasets[0];  /* Use first dataset */
    if (ds->value_count == 0) return;
    
    /* Calculate total */
    VenomF32 total = 0;
    for (VenomU32 i = 0; i < ds->value_count; i++) {
        total += ds->values[i];
    }
    if (total == 0) return;
    
    VenomF32 cx = area.x + area.width / 2;
    VenomF32 cy = area.y + area.height / 2;
    VenomF32 radius = (area.width < area.height ? area.width : area.height) / 2 - 10;
    VenomF32 inner_radius = donut ? radius * 0.6f : 0;
    
    /* Color palette */
    VenomColor colors[] = {
        venom_color_rgb(60, 120, 220),
        venom_color_rgb(60, 180, 100),
        venom_color_rgb(240, 180, 40),
        venom_color_rgb(220, 70, 70),
        venom_color_rgb(140, 100, 200),
        venom_color_rgb(80, 180, 220),
        venom_color_rgb(250, 130, 80),
        venom_color_rgb(180, 180, 190)
    };
    VenomU32 color_count = sizeof(colors) / sizeof(colors[0]);
    
    VenomF32 start_angle = -M_PI / 2;  /* Start from top */
    
    for (VenomU32 i = 0; i < ds->value_count; i++) {
        VenomF32 val = ds->values[i];
        VenomF32 sweep = (val / total) * 2 * M_PI * chart->anim_progress;
        
        VenomColor color = colors[i % color_count];
        
        /* Draw arc segments (approximated with lines) */
        VenomU32 segments = 32;
        for (VenomU32 s = 0; s < segments; s++) {
            VenomF32 a1 = start_angle + sweep * s / segments;
            VenomF32 a2 = start_angle + sweep * (s + 1) / segments;
            
            VenomF32 x1 = cx + cosf(a1) * radius;
            VenomF32 y1 = cy + sinf(a1) * radius;
            VenomF32 x2 = cx + cosf(a2) * radius;
            VenomF32 y2 = cy + sinf(a2) * radius;
            
            VenomPaint pp = venom_paint_stroke(color, radius);
            venom_canvas_draw_line(canvas, cx, cy, x1, y1, &pp);
        }
        
        start_angle += sweep;
    }
    
    /* Draw center hole for donut */
    if (donut) {
        VenomPaint cp = venom_paint_fill(chart->background);
        venom_canvas_draw_circle(canvas, cx, cy, inner_radius, &cp);
    }
}

/* ============================================================================
 * DRAWING - MAIN
 * ============================================================================ */

static void chart_draw(VenomWidget* widget, VenomCanvas* canvas) {
    VenomChart* chart = (VenomChart*)widget;
    VenomRectF bounds = { 0, 0, widget->bounds.width, widget->bounds.height };
    
    /* Background */
    if (chart->animate && chart->anim_progress < 1.0f) {
        chart->anim_progress += 0.03f;
        if (chart->anim_progress > 1.0f) chart->anim_progress = 1.0f;
        widget->needs_redraw = VENOM_TRUE;
    }
    
    /* Calculate range */
    calculate_range(chart);
    
    /* Draw background */
    VenomPaint bg = venom_paint_fill(chart->background);
    venom_canvas_draw_rect(canvas, bounds, &bg);
    
    /* Calculate chart area */
    VenomF32 top = chart->padding;
    VenomF32 bottom = chart->padding;
    if (chart->title) top += CHART_TITLE_HEIGHT;
    if (chart->show_legend) bottom += CHART_LEGEND_HEIGHT;
    
    VenomRectF chart_area = {
        bounds.x + chart->padding,
        bounds.y + top,
        bounds.width - chart->padding * 2,
        bounds.height - top - bottom
    };
    
    /* Draw title */
    if (chart->title) {
        VenomPaint tp = venom_paint_fill(chart->text_color);
        venom_canvas_draw_text(canvas, chart->title, 
            bounds.x + bounds.width / 2, bounds.y + 20, NULL, &tp);
    }
    
    /* Draw grid (line/bar/area only) */
    if (chart->show_grid && chart->type != VENOM_CHART_PIE && chart->type != VENOM_CHART_DONUT) {
        
        /* Horizontal lines */
        VenomPaint grid_stroke = venom_paint_stroke(chart->grid_color, 1);
        for (int i = 0; i <= 5; i++) {
            VenomF32 y = chart_area.y + chart_area.height * i / 5;
            venom_canvas_draw_line(canvas, chart_area.x, y, 
                                   chart_area.x + chart_area.width, y, &grid_stroke);
        }
        
        /* Vertical lines */
        if (chart->label_count > 1) {
            for (VenomU32 i = 0; i < chart->label_count; i++) {
                VenomF32 x = chart_area.x + chart_area.width * i / (chart->label_count - 1);
                venom_canvas_draw_line(canvas, x, chart_area.y, 
                                       x, chart_area.y + chart_area.height, &grid_stroke);
            }
        }
    }
    
    /* Draw chart by type */
    switch (chart->type) {
        case VENOM_CHART_LINE:
        case VENOM_CHART_AREA:
            draw_line_chart(chart, canvas, chart_area);
            break;
        case VENOM_CHART_BAR:
            draw_bar_chart(chart, canvas, chart_area);
            break;
        case VENOM_CHART_PIE:
            draw_pie_chart(chart, canvas, chart_area, VENOM_FALSE);
            break;
        case VENOM_CHART_DONUT:
            draw_pie_chart(chart, canvas, chart_area, VENOM_TRUE);
            break;
    }
    
    /* Draw labels (line/bar/area only) */
    if (chart->show_labels && chart->labels && chart->type != VENOM_CHART_PIE && chart->type != VENOM_CHART_DONUT) {
        VenomPaint lp = venom_paint_fill(chart->text_color);
        VenomF32 step = chart_area.width / (VenomF32)(chart->label_count > 1 ? chart->label_count - 1 : 1);
        
        for (VenomU32 i = 0; i < chart->label_count; i++) {
            if (chart->labels[i]) {
                VenomF32 x = chart_area.x + i * step;
                venom_canvas_draw_text(canvas, chart->labels[i], 
                    x, chart_area.y + chart_area.height + 15, NULL, &lp);
            }
        }
    }
    
    /* Draw legend */
    if (chart->show_legend && chart->dataset_count > 0) {
        VenomF32 legend_y = bounds.y + bounds.height - CHART_LEGEND_HEIGHT + 10;
        VenomF32 legend_x = bounds.x + chart->padding;
        
        for (VenomU32 d = 0; d < chart->dataset_count; d++) {
            VenomDataset* ds = &chart->datasets[d];
            
            /* Color box */
            VenomRectF box = {legend_x, legend_y - 6, 12, 12};
            VenomPaint bp = venom_paint_fill(ds->color);
            venom_canvas_draw_rounded_rect(canvas, box, 2, &bp);
            
            /* Label */
            if (ds->label) {
                VenomPaint lp = venom_paint_fill(chart->text_color);
                venom_canvas_draw_text(canvas, ds->label, legend_x + 18, legend_y + 5, NULL, &lp);
                legend_x += 80;  /* Approximate spacing */
            }
        }
    }
}

/* ============================================================================
 * PUBLIC API
 * ============================================================================ */

VenomResultPtr venom_chart_create(VenomChartType type) {
    VenomResultPtr result = venom_widget_create(&venom_chart_class);
    if (result.ok) {
        VenomChart* chart = (VenomChart*)result.value;
        chart->type = type;
        chart->anim_progress = chart->animate ? 0 : 1;
    }
    return result;
}

void venom_chart_set_labels(VenomChart* chart, const char** labels, VenomU32 count) {
    if (!chart) return;
    chart->labels = labels;
    chart->label_count = count;
    venom_widget_invalidate((VenomWidget*)chart);
}

void venom_chart_add_dataset(VenomChart* chart, const VenomDataset* dataset) {
    if (!chart || !dataset) return;
    
    VenomU32 new_count = chart->dataset_count + 1;
    VenomDataset* new_ds = venom_realloc(chart->datasets,
        chart->dataset_count * sizeof(VenomDataset),
        new_count * sizeof(VenomDataset));
    
    if (!new_ds) return;
    
    chart->datasets = new_ds;
    chart->datasets[chart->dataset_count] = *dataset;
    
    /* Copy values */
    if (dataset->values && dataset->value_count > 0) {
        VenomF32* values = venom_alloc(dataset->value_count * sizeof(VenomF32));
        if (values) {
            memcpy(values, dataset->values, dataset->value_count * sizeof(VenomF32));
            chart->datasets[chart->dataset_count].values = values;
        }
    }
    
    chart->dataset_count = new_count;
    venom_widget_invalidate((VenomWidget*)chart);
}

void venom_chart_clear(VenomChart* chart) {
    if (!chart) return;
    
    for (VenomU32 i = 0; i < chart->dataset_count; i++) {
        if (chart->datasets[i].values) {
            venom_free(chart->datasets[i].values,
                      chart->datasets[i].value_count * sizeof(VenomF32));
        }
    }
    if (chart->datasets) {
        venom_free(chart->datasets, chart->dataset_count * sizeof(VenomDataset));
        chart->datasets = NULL;
    }
    chart->dataset_count = 0;
    chart->labels = NULL;
    chart->label_count = 0;
    
    venom_widget_invalidate((VenomWidget*)chart);
}

void venom_chart_update_dataset(VenomChart* chart, VenomU32 index,
                                 const VenomF32* values, VenomU32 count) {
    if (!chart || index >= chart->dataset_count) return;
    
    VenomDataset* ds = &chart->datasets[index];
    
    if (ds->values) {
        venom_free(ds->values, ds->value_count * sizeof(VenomF32));
    }
    
    ds->values = venom_alloc(count * sizeof(VenomF32));
    if (ds->values) {
        memcpy(ds->values, values, count * sizeof(VenomF32));
        ds->value_count = count;
    }
    
    venom_widget_invalidate((VenomWidget*)chart);
}

void venom_chart_set_type(VenomChart* chart, VenomChartType type) {
    if (!chart) return;
    chart->type = type;
    venom_widget_invalidate((VenomWidget*)chart);
}

void venom_chart_set_title(VenomChart* chart, const char* title) {
    if (!chart) return;
    chart->title = title;
    venom_widget_invalidate((VenomWidget*)chart);
}

void venom_chart_set_show_grid(VenomChart* chart, VenomBool show) {
    if (!chart) return;
    chart->show_grid = show;
    venom_widget_invalidate((VenomWidget*)chart);
}

void venom_chart_set_show_legend(VenomChart* chart, VenomBool show) {
    if (!chart) return;
    chart->show_legend = show;
    venom_widget_invalidate((VenomWidget*)chart);
}

void venom_chart_set_animate(VenomChart* chart, VenomBool animate) {
    if (!chart) return;
    chart->animate = animate;
    if (!animate) chart->anim_progress = 1.0f;
}

void venom_chart_set_range(VenomChart* chart, VenomF32 min_val, VenomF32 max_val) {
    if (!chart) return;
    chart->min_value = min_val;
    chart->max_value = max_val;
    chart->auto_scale = VENOM_FALSE;
    venom_widget_invalidate((VenomWidget*)chart);
}

void venom_chart_set_auto_scale(VenomChart* chart, VenomBool auto_scale) {
    if (!chart) return;
    chart->auto_scale = auto_scale;
    venom_widget_invalidate((VenomWidget*)chart);
}

void venom_chart_refresh(VenomChart* chart) {
    if (!chart) return;
    chart->anim_progress = 0;
    venom_widget_invalidate((VenomWidget*)chart);
}
