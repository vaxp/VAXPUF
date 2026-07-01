/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_chart.c - Chart/Graph Widget implementation
 */

#include "vaxp/widgets/vaxp_chart.h"
#include "vaxp/core/vaxp_memory.h"
#include "vaxp/graphics/vaxp_canvas.h"
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

static void chart_init(VaxpWidget* widget);
static void chart_destroy(VaxpWidget* widget);
static void chart_measure(VaxpWidget* widget, VaxpF32 aw, VaxpF32 ah,
                           VaxpF32* ow, VaxpF32* oh);
static void chart_layout(VaxpWidget* widget, VaxpRectF bounds);
static void chart_draw(VaxpWidget* widget, VaxpCanvas* canvas);

/* ============================================================================
 * CLASS DEFINITION
 * ============================================================================ */

const VaxpWidgetClass vaxp_chart_class = {
    .class_name = "VaxpChart",
    .instance_size = sizeof(VaxpChart),
    .parent_class = &vaxp_widget_class,
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

static void calculate_range(VaxpChart* chart) {
    if (!chart->auto_scale) return;
    
    chart->min_value = 0;
    chart->max_value = 0;
    
    for (VaxpU32 d = 0; d < chart->dataset_count; d++) {
        for (VaxpU32 i = 0; i < chart->datasets[d].value_count; i++) {
            VaxpF32 v = chart->datasets[d].values[i];
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

static void chart_init(VaxpWidget* widget) {
    VaxpChart* chart = (VaxpChart*)widget;
    
    chart->type = VAXP_CHART_LINE;
    chart->labels = NULL;
    chart->label_count = 0;
    chart->datasets = NULL;
    chart->dataset_count = 0;
    
    chart->background = VAXP_COLOR_WHITE;
    chart->grid_color = vaxp_color_rgb(230, 230, 235);
    chart->text_color = vaxp_color_rgb(100, 100, 110);
    chart->show_grid = VAXP_TRUE;
    chart->show_legend = VAXP_TRUE;
    chart->show_labels = VAXP_TRUE;
    chart->show_values = VAXP_FALSE;
    chart->animate = VAXP_TRUE;
    chart->anim_progress = 0;
    chart->padding = CHART_PADDING;
    chart->title = NULL;
    chart->min_value = 0;
    chart->max_value = 100;
    chart->auto_scale = VAXP_TRUE;
}

static void chart_destroy(VaxpWidget* widget) {
    VaxpChart* chart = (VaxpChart*)widget;
    
    if (chart->datasets) {
        for (VaxpU32 i = 0; i < chart->dataset_count; i++) {
            if (chart->datasets[i].values) {
                vaxp_free(chart->datasets[i].values, 
                          chart->datasets[i].value_count * sizeof(VaxpF32));
            }
        }
        vaxp_free(chart->datasets, chart->dataset_count * sizeof(VaxpDataset));
        chart->datasets = NULL;
    }
    chart->dataset_count = 0;
    
    vaxp_widget_class.destroy(widget);
}

/* ============================================================================
 * LAYOUT
 * ============================================================================ */

static void chart_measure(VaxpWidget* widget, VaxpF32 aw, VaxpF32 ah,
                           VaxpF32* ow, VaxpF32* oh) {
    (void)widget;
    *ow = aw > 0 ? aw : 300;
    *oh = ah > 0 ? ah : 200;
}

static void chart_layout(VaxpWidget* widget, VaxpRectF bounds) {
    widget->bounds = bounds;
}

/* ============================================================================
 * DRAWING - LINE CHART
 * ============================================================================ */

static void draw_line_chart(VaxpChart* chart, VaxpCanvas* canvas, VaxpRectF area) {
    if (chart->dataset_count == 0 || chart->label_count == 0) return;
    
    VaxpF32 range = chart->max_value - chart->min_value;
    if (range == 0) range = 1;
    
    VaxpF32 step_x = area.width / (VaxpF32)(chart->label_count > 1 ? chart->label_count - 1 : 1);
    
    for (VaxpU32 d = 0; d < chart->dataset_count; d++) {
        VaxpDataset* ds = &chart->datasets[d];
        if (ds->value_count == 0) continue;
        
        VaxpColor line_color = ds->color;
        VaxpF32 line_width = ds->line_width > 0 ? ds->line_width : 2.0f;
        
        /* Draw area fill if type is AREA */
        if (chart->type == VAXP_CHART_AREA) {
            /* Simple fill approximation */
            for (VaxpU32 i = 0; i < ds->value_count && i < chart->label_count; i++) {
                VaxpF32 x = area.x + i * step_x;
                VaxpF32 val = ds->values[i] * chart->anim_progress;
                VaxpF32 y = area.y + area.height - (val - chart->min_value) / range * area.height;
                
                VaxpRectF fill = {x - step_x/2, y, step_x, area.y + area.height - y};
                VaxpColor fill_color = ds->fill_color.a > 0 ? ds->fill_color : 
                                        vaxp_color_rgba(line_color.r, line_color.g, line_color.b, 50);
                VaxpPaint fp = vaxp_paint_fill(fill_color);
                vaxp_canvas_draw_rect(canvas, fill, &fp);
            }
        }
        
        /* Draw line segments */
        for (VaxpU32 i = 1; i < ds->value_count && i < chart->label_count; i++) {
            VaxpF32 x1 = area.x + (i - 1) * step_x;
            VaxpF32 x2 = area.x + i * step_x;
            VaxpF32 v1 = ds->values[i - 1] * chart->anim_progress;
            VaxpF32 v2 = ds->values[i] * chart->anim_progress;
            VaxpF32 y1 = area.y + area.height - (v1 - chart->min_value) / range * area.height;
            VaxpF32 y2 = area.y + area.height - (v2 - chart->min_value) / range * area.height;
            
            VaxpPaint lp = vaxp_paint_stroke(line_color, line_width);
            vaxp_canvas_draw_line(canvas, x1, y1, x2, y2, &lp);
        }
        
        /* Draw points */
        if (ds->show_points) {
            for (VaxpU32 i = 0; i < ds->value_count && i < chart->label_count; i++) {
                VaxpF32 x = area.x + i * step_x;
                VaxpF32 val = ds->values[i] * chart->anim_progress;
                VaxpF32 y = area.y + area.height - (val - chart->min_value) / range * area.height;
                
                VaxpPaint pp = vaxp_paint_fill(line_color);
                vaxp_canvas_draw_circle(canvas, x, y, CHART_POINT_RADIUS, &pp);
            }
        }
    }
}

/* ============================================================================
 * DRAWING - BAR CHART
 * ============================================================================ */

static void draw_bar_chart(VaxpChart* chart, VaxpCanvas* canvas, VaxpRectF area) {
    if (chart->dataset_count == 0 || chart->label_count == 0) return;
    
    VaxpF32 range = chart->max_value - chart->min_value;
    if (range == 0) range = 1;
    
    VaxpF32 group_width = area.width / (VaxpF32)chart->label_count;
    VaxpF32 bar_width = (group_width - CHART_BAR_GAP * 2) / (VaxpF32)chart->dataset_count;
    
    for (VaxpU32 i = 0; i < chart->label_count; i++) {
        VaxpF32 group_x = area.x + i * group_width + CHART_BAR_GAP;
        
        for (VaxpU32 d = 0; d < chart->dataset_count; d++) {
            VaxpDataset* ds = &chart->datasets[d];
            if (i >= ds->value_count) continue;
            
            VaxpF32 val = ds->values[i] * chart->anim_progress;
            VaxpF32 bar_height = (val - chart->min_value) / range * area.height;
            
            VaxpRectF bar = {
                group_x + d * bar_width,
                area.y + area.height - bar_height,
                bar_width - 2,
                bar_height
            };
            
            VaxpPaint bp = vaxp_paint_fill(ds->color);
            vaxp_canvas_draw_rounded_rect(canvas, bar, 3, &bp);
        }
    }
}

/* ============================================================================
 * DRAWING - PIE/DONUT CHART
 * ============================================================================ */

static void draw_pie_chart(VaxpChart* chart, VaxpCanvas* canvas, VaxpRectF area, VaxpBool donut) {
    if (chart->dataset_count == 0) return;
    
    VaxpDataset* ds = &chart->datasets[0];  /* Use first dataset */
    if (ds->value_count == 0) return;
    
    /* Calculate total */
    VaxpF32 total = 0;
    for (VaxpU32 i = 0; i < ds->value_count; i++) {
        total += ds->values[i];
    }
    if (total == 0) return;
    
    VaxpF32 cx = area.x + area.width / 2;
    VaxpF32 cy = area.y + area.height / 2;
    VaxpF32 radius = (area.width < area.height ? area.width : area.height) / 2 - 10;
    VaxpF32 inner_radius = donut ? radius * 0.6f : 0;
    
    /* Color palette */
    VaxpColor colors[] = {
        vaxp_color_rgb(60, 120, 220),
        vaxp_color_rgb(60, 180, 100),
        vaxp_color_rgb(240, 180, 40),
        vaxp_color_rgb(220, 70, 70),
        vaxp_color_rgb(140, 100, 200),
        vaxp_color_rgb(80, 180, 220),
        vaxp_color_rgb(250, 130, 80),
        vaxp_color_rgb(180, 180, 190)
    };
    VaxpU32 color_count = sizeof(colors) / sizeof(colors[0]);
    
    VaxpF32 start_angle = -M_PI / 2;  /* Start from top */
    
    for (VaxpU32 i = 0; i < ds->value_count; i++) {
        VaxpF32 val = ds->values[i];
        VaxpF32 sweep = (val / total) * 2 * M_PI * chart->anim_progress;
        
        VaxpColor color = colors[i % color_count];
        
        /* Draw arc segments (approximated with lines) */
        VaxpU32 segments = 32;
        for (VaxpU32 s = 0; s < segments; s++) {
            VaxpF32 a1 = start_angle + sweep * s / segments;
            VaxpF32 a2 = start_angle + sweep * (s + 1) / segments;
            
            VaxpF32 x1 = cx + cosf(a1) * radius;
            VaxpF32 y1 = cy + sinf(a1) * radius;
            VaxpF32 x2 = cx + cosf(a2) * radius;
            VaxpF32 y2 = cy + sinf(a2) * radius;
            
            VaxpPaint pp = vaxp_paint_stroke(color, radius);
            vaxp_canvas_draw_line(canvas, cx, cy, x1, y1, &pp);
        }
        
        start_angle += sweep;
    }
    
    /* Draw center hole for donut */
    if (donut) {
        VaxpPaint cp = vaxp_paint_fill(chart->background);
        vaxp_canvas_draw_circle(canvas, cx, cy, inner_radius, &cp);
    }
}

/* ============================================================================
 * DRAWING - MAIN
 * ============================================================================ */

static void chart_draw(VaxpWidget* widget, VaxpCanvas* canvas) {
    VaxpChart* chart = (VaxpChart*)widget;
    VaxpRectF bounds = { 0, 0, widget->bounds.width, widget->bounds.height };
    
    /* Background */
    if (chart->animate && chart->anim_progress < 1.0f) {
        chart->anim_progress += 0.03f;
        if (chart->anim_progress > 1.0f) chart->anim_progress = 1.0f;
        widget->needs_redraw = VAXP_TRUE;
    }
    
    /* Calculate range */
    calculate_range(chart);
    
    /* Draw background */
    VaxpPaint bg = vaxp_paint_fill(chart->background);
    vaxp_canvas_draw_rect(canvas, bounds, &bg);
    
    /* Calculate chart area */
    VaxpF32 top = chart->padding;
    VaxpF32 bottom = chart->padding;
    if (chart->title) top += CHART_TITLE_HEIGHT;
    if (chart->show_legend) bottom += CHART_LEGEND_HEIGHT;
    
    VaxpRectF chart_area = {
        bounds.x + chart->padding,
        bounds.y + top,
        bounds.width - chart->padding * 2,
        bounds.height - top - bottom
    };
    
    /* Draw title */
    if (chart->title) {
        VaxpPaint tp = vaxp_paint_fill(chart->text_color);
        vaxp_canvas_draw_text(canvas, chart->title, 
            bounds.x + bounds.width / 2, bounds.y + 20, NULL, &tp);
    }
    
    /* Draw grid (line/bar/area only) */
    if (chart->show_grid && chart->type != VAXP_CHART_PIE && chart->type != VAXP_CHART_DONUT) {
        
        /* Horizontal lines */
        VaxpPaint grid_stroke = vaxp_paint_stroke(chart->grid_color, 1);
        for (int i = 0; i <= 5; i++) {
            VaxpF32 y = chart_area.y + chart_area.height * i / 5;
            vaxp_canvas_draw_line(canvas, chart_area.x, y, 
                                   chart_area.x + chart_area.width, y, &grid_stroke);
        }
        
        /* Vertical lines */
        if (chart->label_count > 1) {
            for (VaxpU32 i = 0; i < chart->label_count; i++) {
                VaxpF32 x = chart_area.x + chart_area.width * i / (chart->label_count - 1);
                vaxp_canvas_draw_line(canvas, x, chart_area.y, 
                                       x, chart_area.y + chart_area.height, &grid_stroke);
            }
        }
    }
    
    /* Draw chart by type */
    switch (chart->type) {
        case VAXP_CHART_LINE:
        case VAXP_CHART_AREA:
            draw_line_chart(chart, canvas, chart_area);
            break;
        case VAXP_CHART_BAR:
            draw_bar_chart(chart, canvas, chart_area);
            break;
        case VAXP_CHART_PIE:
            draw_pie_chart(chart, canvas, chart_area, VAXP_FALSE);
            break;
        case VAXP_CHART_DONUT:
            draw_pie_chart(chart, canvas, chart_area, VAXP_TRUE);
            break;
    }
    
    /* Draw labels (line/bar/area only) */
    if (chart->show_labels && chart->labels && chart->type != VAXP_CHART_PIE && chart->type != VAXP_CHART_DONUT) {
        VaxpPaint lp = vaxp_paint_fill(chart->text_color);
        VaxpF32 step = chart_area.width / (VaxpF32)(chart->label_count > 1 ? chart->label_count - 1 : 1);
        
        for (VaxpU32 i = 0; i < chart->label_count; i++) {
            if (chart->labels[i]) {
                VaxpF32 x = chart_area.x + i * step;
                vaxp_canvas_draw_text(canvas, chart->labels[i], 
                    x, chart_area.y + chart_area.height + 15, NULL, &lp);
            }
        }
    }
    
    /* Draw legend */
    if (chart->show_legend && chart->dataset_count > 0) {
        VaxpF32 legend_y = bounds.y + bounds.height - CHART_LEGEND_HEIGHT + 10;
        VaxpF32 legend_x = bounds.x + chart->padding;
        
        for (VaxpU32 d = 0; d < chart->dataset_count; d++) {
            VaxpDataset* ds = &chart->datasets[d];
            
            /* Color box */
            VaxpRectF box = {legend_x, legend_y - 6, 12, 12};
            VaxpPaint bp = vaxp_paint_fill(ds->color);
            vaxp_canvas_draw_rounded_rect(canvas, box, 2, &bp);
            
            /* Label */
            if (ds->label) {
                VaxpPaint lp = vaxp_paint_fill(chart->text_color);
                vaxp_canvas_draw_text(canvas, ds->label, legend_x + 18, legend_y + 5, NULL, &lp);
                legend_x += 80;  /* Approximate spacing */
            }
        }
    }
}

/* ============================================================================
 * PUBLIC API
 * ============================================================================ */

VaxpResultPtr vaxp_chart_create(VaxpChartType type) {
    VaxpResultPtr result = vaxp_widget_create(&vaxp_chart_class);
    if (result.ok) {
        VaxpChart* chart = (VaxpChart*)result.value;
        chart->type = type;
        chart->anim_progress = chart->animate ? 0 : 1;
    }
    return result;
}

void vaxp_chart_set_labels(VaxpChart* chart, const char** labels, VaxpU32 count) {
    if (!chart) return;
    chart->labels = labels;
    chart->label_count = count;
    vaxp_widget_invalidate((VaxpWidget*)chart);
}

void vaxp_chart_add_dataset(VaxpChart* chart, const VaxpDataset* dataset) {
    if (!chart || !dataset) return;
    
    VaxpU32 new_count = chart->dataset_count + 1;
    VaxpDataset* new_ds = vaxp_realloc(chart->datasets,
        chart->dataset_count * sizeof(VaxpDataset),
        new_count * sizeof(VaxpDataset));
    
    if (!new_ds) return;
    
    chart->datasets = new_ds;
    chart->datasets[chart->dataset_count] = *dataset;
    
    /* Copy values */
    if (dataset->values && dataset->value_count > 0) {
        VaxpF32* values = vaxp_alloc(dataset->value_count * sizeof(VaxpF32));
        if (values) {
            memcpy(values, dataset->values, dataset->value_count * sizeof(VaxpF32));
            chart->datasets[chart->dataset_count].values = values;
        }
    }
    
    chart->dataset_count = new_count;
    vaxp_widget_invalidate((VaxpWidget*)chart);
}

void vaxp_chart_clear(VaxpChart* chart) {
    if (!chart) return;
    
    for (VaxpU32 i = 0; i < chart->dataset_count; i++) {
        if (chart->datasets[i].values) {
            vaxp_free(chart->datasets[i].values,
                      chart->datasets[i].value_count * sizeof(VaxpF32));
        }
    }
    if (chart->datasets) {
        vaxp_free(chart->datasets, chart->dataset_count * sizeof(VaxpDataset));
        chart->datasets = NULL;
    }
    chart->dataset_count = 0;
    chart->labels = NULL;
    chart->label_count = 0;
    
    vaxp_widget_invalidate((VaxpWidget*)chart);
}

void vaxp_chart_update_dataset(VaxpChart* chart, VaxpU32 index,
                                 const VaxpF32* values, VaxpU32 count) {
    if (!chart || index >= chart->dataset_count) return;
    
    VaxpDataset* ds = &chart->datasets[index];
    
    if (ds->values) {
        vaxp_free(ds->values, ds->value_count * sizeof(VaxpF32));
    }
    
    ds->values = vaxp_alloc(count * sizeof(VaxpF32));
    if (ds->values) {
        memcpy(ds->values, values, count * sizeof(VaxpF32));
        ds->value_count = count;
    }
    
    vaxp_widget_invalidate((VaxpWidget*)chart);
}

void vaxp_chart_set_type(VaxpChart* chart, VaxpChartType type) {
    if (!chart) return;
    chart->type = type;
    vaxp_widget_invalidate((VaxpWidget*)chart);
}

void vaxp_chart_set_title(VaxpChart* chart, const char* title) {
    if (!chart) return;
    chart->title = title;
    vaxp_widget_invalidate((VaxpWidget*)chart);
}

void vaxp_chart_set_show_grid(VaxpChart* chart, VaxpBool show) {
    if (!chart) return;
    chart->show_grid = show;
    vaxp_widget_invalidate((VaxpWidget*)chart);
}

void vaxp_chart_set_show_legend(VaxpChart* chart, VaxpBool show) {
    if (!chart) return;
    chart->show_legend = show;
    vaxp_widget_invalidate((VaxpWidget*)chart);
}

void vaxp_chart_set_animate(VaxpChart* chart, VaxpBool animate) {
    if (!chart) return;
    chart->animate = animate;
    if (!animate) chart->anim_progress = 1.0f;
}

void vaxp_chart_set_range(VaxpChart* chart, VaxpF32 min_val, VaxpF32 max_val) {
    if (!chart) return;
    chart->min_value = min_val;
    chart->max_value = max_val;
    chart->auto_scale = VAXP_FALSE;
    vaxp_widget_invalidate((VaxpWidget*)chart);
}

void vaxp_chart_set_auto_scale(VaxpChart* chart, VaxpBool auto_scale) {
    if (!chart) return;
    chart->auto_scale = auto_scale;
    vaxp_widget_invalidate((VaxpWidget*)chart);
}

void vaxp_chart_refresh(VaxpChart* chart) {
    if (!chart) return;
    chart->anim_progress = 0;
    vaxp_widget_invalidate((VaxpWidget*)chart);
}
