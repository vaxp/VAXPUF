/*
 * VAXPUI - FileChooser widget implementation
 */

#include "vaxp/widgets/vaxp_file_chooser.h"
#include "vaxp/core/vaxp_memory.h"
#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>

#define ITEM_HEIGHT 32.0f
#define ICON_SIZE 20.0f
#define PATH_BAR_HEIGHT 40.0f

static void clear_entries(VaxpFileChooser* chooser) {
    for (VaxpU32 i = 0; i < chooser->entry_count; i++) {
        if (chooser->entries[i].name) vaxp_free(chooser->entries[i].name, strlen(chooser->entries[i].name) + 1);
        if (chooser->entries[i].path) vaxp_free(chooser->entries[i].path, strlen(chooser->entries[i].path) + 1);
    }
    chooser->entry_count = 0;
}

static char* strdup_alloc(const char* s) {
    if (!s) return NULL;
    size_t len = strlen(s) + 1;
    char* copy = (char*)vaxp_alloc(len);
    if (copy) memcpy(copy, s, len);
    return copy;
}

static void add_entry(VaxpFileChooser* chooser, const char* name, const char* path, VaxpBool is_dir, VaxpSize size) {
    if (chooser->entry_count >= chooser->entry_capacity) {
        VaxpU32 new_cap = chooser->entry_capacity == 0 ? 64 : chooser->entry_capacity * 2;
        VaxpFileEntry* new_entries = (VaxpFileEntry*)vaxp_alloc(new_cap * sizeof(VaxpFileEntry));
        if (!new_entries) return;
        if (chooser->entries) {
            memcpy(new_entries, chooser->entries, chooser->entry_count * sizeof(VaxpFileEntry));
            vaxp_free(chooser->entries, chooser->entry_capacity * sizeof(VaxpFileEntry));
        }
        chooser->entries = new_entries;
        chooser->entry_capacity = new_cap;
    }
    
    VaxpFileEntry* e = &chooser->entries[chooser->entry_count++];
    e->name = strdup_alloc(name);
    e->path = strdup_alloc(path);
    e->is_directory = is_dir;
    e->size = size;
    e->selected = VAXP_FALSE;
}

static void load_directory(VaxpFileChooser* chooser) {
    clear_entries(chooser);
    if (!chooser->current_path) return;
    
    DIR* dir = opendir(chooser->current_path);
    if (!dir) return;
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0) continue;
        if (!chooser->show_hidden && entry->d_name[0] == '.' && strcmp(entry->d_name, "..") != 0) continue;
        
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", chooser->current_path, entry->d_name);
        
        struct stat st;
        VaxpBool is_dir = VAXP_FALSE;
        VaxpSize size = 0;
        if (stat(full_path, &st) == 0) {
            is_dir = S_ISDIR(st.st_mode);
            size = (VaxpSize)st.st_size;
        }
        
        /* Apply filter for files */
        if (!is_dir && chooser->filter_pattern) {
            /* Simple extension matching */
            const char* ext = strrchr(entry->d_name, '.');
            if (ext && !strstr(chooser->filter_pattern, ext)) continue;
        }
        
        add_entry(chooser, entry->d_name, full_path, is_dir, size);
    }
    closedir(dir);
    
    /* Sort: directories first, then alphabetically */
    for (VaxpU32 i = 0; i < chooser->entry_count; i++) {
        for (VaxpU32 j = i + 1; j < chooser->entry_count; j++) {
            VaxpBool swap = VAXP_FALSE;
            if (chooser->entries[j].is_directory && !chooser->entries[i].is_directory) swap = VAXP_TRUE;
            else if (chooser->entries[i].is_directory == chooser->entries[j].is_directory &&
                     strcmp(chooser->entries[i].name, chooser->entries[j].name) > 0) swap = VAXP_TRUE;
            
            if (swap) {
                VaxpFileEntry tmp = chooser->entries[i];
                chooser->entries[i] = chooser->entries[j];
                chooser->entries[j] = tmp;
            }
        }
    }
}

static void file_chooser_init(VaxpWidget* widget) {
    VaxpFileChooser* chooser = (VaxpFileChooser*)widget;
    
    chooser->mode = VAXP_FILE_CHOOSER_OPEN;
    chooser->current_path = strdup_alloc(getenv("HOME") ? getenv("HOME") : "/");
    chooser->selected_path = NULL;
    chooser->filename = NULL;
    chooser->entries = NULL;
    chooser->entry_count = 0;
    chooser->entry_capacity = 0;
    chooser->selected_paths = NULL;
    chooser->selected_count = 0;
    chooser->filter = NULL;
    chooser->filter_data = NULL;
    chooser->filter_pattern = NULL;
    chooser->show_hidden = VAXP_FALSE;
    chooser->show_path_bar = VAXP_TRUE;
    chooser->hover_index = -1;
    chooser->scroll_offset = 0;
    chooser->item_height = ITEM_HEIGHT;
    
    chooser->background_color = (VaxpColor){ 255, 255, 255, 255 };
    chooser->item_color = (VaxpColor){ 0, 0, 0, 10 };
    chooser->selected_color = (VaxpColor){ 63, 81, 181, 50 };
    chooser->folder_color = (VaxpColor){ 255, 193, 7, 255 };
    chooser->file_color = (VaxpColor){ 97, 97, 97, 255 };
    
    chooser->on_select = NULL;
    chooser->on_multi_select = NULL;
    chooser->on_confirm = NULL;
    chooser->callback_data = NULL;
    
    widget->focusable = VAXP_TRUE;
    load_directory(chooser);
}

static void file_chooser_destroy(VaxpWidget* widget) {
    VaxpFileChooser* chooser = (VaxpFileChooser*)widget;
    clear_entries(chooser);
    if (chooser->entries) vaxp_free(chooser->entries, chooser->entry_capacity * sizeof(VaxpFileEntry));
    if (chooser->current_path) vaxp_free(chooser->current_path, strlen(chooser->current_path) + 1);
    if (chooser->selected_path) vaxp_free(chooser->selected_path, strlen(chooser->selected_path) + 1);
    if (chooser->filter_pattern) vaxp_free(chooser->filter_pattern, strlen(chooser->filter_pattern) + 1);
    vaxp_widget_class.destroy(widget);
}

static void file_chooser_measure(VaxpWidget* widget, VaxpF32 aw, VaxpF32 ah, VaxpF32* w, VaxpF32* h) {
    (void)aw; (void)ah;
    *w = widget->layout.preferred_width > 0 ? widget->layout.preferred_width : 400;
    *h = widget->layout.preferred_height > 0 ? widget->layout.preferred_height : 300;
}

static void file_chooser_draw(VaxpWidget* widget, VaxpCanvas* canvas) {
    VaxpFileChooser* chooser = (VaxpFileChooser*)widget;
    VaxpF32 w = widget->bounds.width;
    VaxpF32 h = widget->bounds.height;
    
    /* Background */
    VaxpRectF bg = {0, 0, w, h};
    VaxpPaint bgp = vaxp_paint_fill(chooser->background_color);
    vaxp_canvas_draw_rect(canvas, bg, &bgp);
    
    /* Path bar */
    VaxpF32 y = 0;
    if (chooser->show_path_bar) {
        VaxpRectF pb = {0, 0, w, PATH_BAR_HEIGHT};
        VaxpPaint pbp = vaxp_paint_fill((VaxpColor){245,245,245,255});
        vaxp_canvas_draw_rect(canvas, pb, &pbp);
        
        /* Up button */
        VaxpPaint ip = vaxp_paint_fill((VaxpColor){97,97,97,255});
        vaxp_canvas_draw_text(canvas, "⬆", 8, PATH_BAR_HEIGHT/2 + 5, NULL, &ip);
        
        /* Path */
        if (chooser->current_path) {
            VaxpPaint tp = vaxp_paint_fill((VaxpColor){33,33,33,255});
            vaxp_canvas_draw_text(canvas, chooser->current_path, 35, PATH_BAR_HEIGHT/2 + 5, NULL, &tp);
        }
        
        y = PATH_BAR_HEIGHT;
    }
    
    /* File list */
    VaxpF32 list_h = h - y;
    VaxpI32 visible_items = (VaxpI32)(list_h / chooser->item_height);
    VaxpI32 start_idx = chooser->scroll_offset / (VaxpI32)chooser->item_height;
    
    for (VaxpI32 i = start_idx; i < (VaxpI32)chooser->entry_count && i < start_idx + visible_items + 1; i++) {
        VaxpFileEntry* e = &chooser->entries[i];
        VaxpF32 iy = y + (i - start_idx) * chooser->item_height;
        
        /* Hover/selection background */
        if (i == chooser->hover_index) {
            VaxpRectF hr = {0, iy, w, chooser->item_height};
            VaxpPaint hp = vaxp_paint_fill(chooser->item_color);
            vaxp_canvas_draw_rect(canvas, hr, &hp);
        }
        if (e->selected) {
            VaxpRectF sr = {0, iy, w, chooser->item_height};
            VaxpPaint sp = vaxp_paint_fill(chooser->selected_color);
            vaxp_canvas_draw_rect(canvas, sr, &sp);
        }
        
        /* Icon */
        VaxpPaint icp = vaxp_paint_fill(e->is_directory ? chooser->folder_color : chooser->file_color);
        const char* icon = e->is_directory ? "📁" : "📄";
        vaxp_canvas_draw_text(canvas, icon, 12, iy + chooser->item_height/2 + 5, NULL, &icp);
        
        /* Name */
        VaxpPaint np = vaxp_paint_fill((VaxpColor){33,33,33,255});
        vaxp_canvas_draw_text(canvas, e->name, 40, iy + chooser->item_height/2 + 5, NULL, &np);
        
        /* Size for files */
        if (!e->is_directory) {
            char size_str[32];
            if (e->size >= 1024*1024) snprintf(size_str, sizeof(size_str), "%.1f MB", e->size / (1024.0*1024.0));
            else if (e->size >= 1024) snprintf(size_str, sizeof(size_str), "%.1f KB", e->size / 1024.0);
            else snprintf(size_str, sizeof(size_str), "%zu B", e->size);
            
            VaxpPaint szp = vaxp_paint_fill((VaxpColor){158,158,158,255});
            vaxp_canvas_draw_text(canvas, size_str, w - 80, iy + chooser->item_height/2 + 5, NULL, &szp);
        }
    }
}

static VaxpBool file_chooser_on_event(VaxpWidget* widget, const VaxpEvent* event) {
    VaxpFileChooser* chooser = (VaxpFileChooser*)widget;
    VaxpF32 y_off = chooser->show_path_bar ? PATH_BAR_HEIGHT : 0;
    
    if (event->type == VAXP_EVENT_MOUSE_MOVE) {
        VaxpF32 my = (VaxpF32)event->mouse.y - y_off + chooser->scroll_offset;
        VaxpI32 new_hover = (my >= 0) ? (VaxpI32)(my / chooser->item_height) : -1;
        if (new_hover >= (VaxpI32)chooser->entry_count) new_hover = -1;
        if (new_hover != chooser->hover_index) {
            chooser->hover_index = new_hover;
            widget->needs_redraw = VAXP_TRUE;
        }
    }
    
    if (event->type == VAXP_EVENT_MOUSE_BUTTON_DOWN && event->mouse.button == VAXP_MOUSE_BUTTON_LEFT) {
        VaxpF32 mx = (VaxpF32)event->mouse.x;
        VaxpF32 my = (VaxpF32)event->mouse.y;
        
        /* Up button */
        if (chooser->show_path_bar && my < PATH_BAR_HEIGHT && mx < 30) {
            vaxp_file_chooser_go_up(chooser);
            return VAXP_TRUE;
        }
        
        /* Select item */
        if (chooser->hover_index >= 0) {
            VaxpFileEntry* e = &chooser->entries[chooser->hover_index];
            
            if (e->is_directory) {
                /* Navigate into directory */
                if (chooser->current_path) vaxp_free(chooser->current_path, strlen(chooser->current_path) + 1);
                chooser->current_path = strdup_alloc(e->path);
                chooser->scroll_offset = 0;
                load_directory(chooser);
            } else {
                /* Select file */
                e->selected = !e->selected;
                if (chooser->selected_path) vaxp_free(chooser->selected_path, strlen(chooser->selected_path) + 1);
                chooser->selected_path = strdup_alloc(e->path);
                
                if (chooser->on_select) {
                    chooser->on_select(chooser, e->path, chooser->callback_data);
                }
            }
            widget->needs_redraw = VAXP_TRUE;
            return VAXP_TRUE;
        }
    }
    
    if (event->type == VAXP_EVENT_MOUSE_BUTTON_DOWN && event->mouse.button == VAXP_MOUSE_BUTTON_LEFT) {
        /* Double-click to confirm */
        VaxpFileEntry* e = chooser->hover_index >= 0 ? &chooser->entries[chooser->hover_index] : NULL;
        if (e && !e->is_directory && chooser->on_confirm) {
            chooser->on_confirm(chooser, e->path, chooser->callback_data);
        }
    }
    
    if (event->type == VAXP_EVENT_MOUSE_SCROLL) {
        chooser->scroll_offset -= (VaxpI32)(event->scroll.y * chooser->item_height * 3);
        if (chooser->scroll_offset < 0) chooser->scroll_offset = 0;
        VaxpI32 max = (VaxpI32)(chooser->entry_count * chooser->item_height - widget->bounds.height);
        if (max < 0) max = 0;
        if (chooser->scroll_offset > max) chooser->scroll_offset = max;
        widget->needs_redraw = VAXP_TRUE;
        return VAXP_TRUE;
    }
    
    return VAXP_FALSE;
}

const VaxpWidgetClass vaxp_file_chooser_class = {
    .class_name = "VaxpFileChooser",
    .instance_size = sizeof(VaxpFileChooser),
    .parent_class = &vaxp_widget_class,
    .init = file_chooser_init,
    .destroy = file_chooser_destroy,
    .measure = file_chooser_measure,
    .draw = file_chooser_draw,
    .on_event = file_chooser_on_event,
};

VaxpResultPtr vaxp_file_chooser_create(void) { return vaxp_widget_create(&vaxp_file_chooser_class); }

void vaxp_file_chooser_set_mode(VaxpFileChooser* c, VaxpFileChooserMode m) { if (c) c->mode = m; }

void vaxp_file_chooser_set_path(VaxpFileChooser* chooser, const char* path) {
    if (!chooser) return;
    if (chooser->current_path) vaxp_free(chooser->current_path, strlen(chooser->current_path) + 1);
    chooser->current_path = strdup_alloc(path);
    chooser->scroll_offset = 0;
    load_directory(chooser);
    vaxp_widget_invalidate((VaxpWidget*)chooser);
}

const char* vaxp_file_chooser_get_path(const VaxpFileChooser* c) { return c ? c->current_path : NULL; }
const char* vaxp_file_chooser_get_selected(const VaxpFileChooser* c) { return c ? c->selected_path : NULL; }

void vaxp_file_chooser_set_filter(VaxpFileChooser* chooser, const char* pattern) {
    if (!chooser) return;
    if (chooser->filter_pattern) vaxp_free(chooser->filter_pattern, strlen(chooser->filter_pattern) + 1);
    chooser->filter_pattern = strdup_alloc(pattern);
    load_directory(chooser);
    vaxp_widget_invalidate((VaxpWidget*)chooser);
}

void vaxp_file_chooser_set_show_hidden(VaxpFileChooser* c, VaxpBool s) {
    if (c) { c->show_hidden = s; load_directory(c); vaxp_widget_invalidate((VaxpWidget*)c); }
}

void vaxp_file_chooser_refresh(VaxpFileChooser* c) {
    if (c) { load_directory(c); vaxp_widget_invalidate((VaxpWidget*)c); }
}

void vaxp_file_chooser_go_up(VaxpFileChooser* chooser) {
    if (!chooser || !chooser->current_path) return;
    char* last_slash = strrchr(chooser->current_path, '/');
    if (last_slash && last_slash != chooser->current_path) {
        *last_slash = '\0';
    } else if (last_slash == chooser->current_path) {
        chooser->current_path[1] = '\0';
    }
    chooser->scroll_offset = 0;
    load_directory(chooser);
    vaxp_widget_invalidate((VaxpWidget*)chooser);
}

void vaxp_file_chooser_set_on_select(VaxpFileChooser* c, VaxpFileCallback cb, void* data) {
    if (c) { c->on_select = cb; c->callback_data = data; }
}

void vaxp_file_chooser_set_on_confirm(VaxpFileChooser* c, VaxpFileCallback cb, void* data) {
    if (c) { c->on_confirm = cb; c->callback_data = data; }
}

VaxpWidget* _vaxp_file_chooser_build(const VaxpFileChooserConfig* config) {
    VaxpResultPtr r = vaxp_file_chooser_create();
    if (!r.ok) return NULL;
    VaxpFileChooser* c = (VaxpFileChooser*)r.value;
    c->mode = config->mode;
    if (config->initial_path) vaxp_file_chooser_set_path(c, config->initial_path);
    if (config->filter) vaxp_file_chooser_set_filter(c, config->filter);
    c->on_confirm = config->on_confirm;
    c->callback_data = config->data;
    return (VaxpWidget*)c;
}
