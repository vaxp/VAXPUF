/*
 * VENOMUI - FileChooser widget implementation
 */

#include "venom/widgets/venom_file_chooser.h"
#include "venom/core/venom_memory.h"
#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>

#define ITEM_HEIGHT 32.0f
#define ICON_SIZE 20.0f
#define PATH_BAR_HEIGHT 40.0f

static void clear_entries(VenomFileChooser* chooser) {
    for (VenomU32 i = 0; i < chooser->entry_count; i++) {
        if (chooser->entries[i].name) venom_free(chooser->entries[i].name, strlen(chooser->entries[i].name) + 1);
        if (chooser->entries[i].path) venom_free(chooser->entries[i].path, strlen(chooser->entries[i].path) + 1);
    }
    chooser->entry_count = 0;
}

static char* strdup_alloc(const char* s) {
    if (!s) return NULL;
    size_t len = strlen(s) + 1;
    char* copy = (char*)venom_alloc(len);
    if (copy) memcpy(copy, s, len);
    return copy;
}

static void add_entry(VenomFileChooser* chooser, const char* name, const char* path, VenomBool is_dir, VenomSize size) {
    if (chooser->entry_count >= chooser->entry_capacity) {
        VenomU32 new_cap = chooser->entry_capacity == 0 ? 64 : chooser->entry_capacity * 2;
        VenomFileEntry* new_entries = (VenomFileEntry*)venom_alloc(new_cap * sizeof(VenomFileEntry));
        if (!new_entries) return;
        if (chooser->entries) {
            memcpy(new_entries, chooser->entries, chooser->entry_count * sizeof(VenomFileEntry));
            venom_free(chooser->entries, chooser->entry_capacity * sizeof(VenomFileEntry));
        }
        chooser->entries = new_entries;
        chooser->entry_capacity = new_cap;
    }
    
    VenomFileEntry* e = &chooser->entries[chooser->entry_count++];
    e->name = strdup_alloc(name);
    e->path = strdup_alloc(path);
    e->is_directory = is_dir;
    e->size = size;
    e->selected = VENOM_FALSE;
}

static void load_directory(VenomFileChooser* chooser) {
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
        VenomBool is_dir = VENOM_FALSE;
        VenomSize size = 0;
        if (stat(full_path, &st) == 0) {
            is_dir = S_ISDIR(st.st_mode);
            size = (VenomSize)st.st_size;
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
    for (VenomU32 i = 0; i < chooser->entry_count; i++) {
        for (VenomU32 j = i + 1; j < chooser->entry_count; j++) {
            VenomBool swap = VENOM_FALSE;
            if (chooser->entries[j].is_directory && !chooser->entries[i].is_directory) swap = VENOM_TRUE;
            else if (chooser->entries[i].is_directory == chooser->entries[j].is_directory &&
                     strcmp(chooser->entries[i].name, chooser->entries[j].name) > 0) swap = VENOM_TRUE;
            
            if (swap) {
                VenomFileEntry tmp = chooser->entries[i];
                chooser->entries[i] = chooser->entries[j];
                chooser->entries[j] = tmp;
            }
        }
    }
}

static void file_chooser_init(VenomWidget* widget) {
    VenomFileChooser* chooser = (VenomFileChooser*)widget;
    
    chooser->mode = VENOM_FILE_CHOOSER_OPEN;
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
    chooser->show_hidden = VENOM_FALSE;
    chooser->show_path_bar = VENOM_TRUE;
    chooser->hover_index = -1;
    chooser->scroll_offset = 0;
    chooser->item_height = ITEM_HEIGHT;
    
    chooser->background_color = (VenomColor){ 255, 255, 255, 255 };
    chooser->item_color = (VenomColor){ 0, 0, 0, 10 };
    chooser->selected_color = (VenomColor){ 63, 81, 181, 50 };
    chooser->folder_color = (VenomColor){ 255, 193, 7, 255 };
    chooser->file_color = (VenomColor){ 97, 97, 97, 255 };
    
    chooser->on_select = NULL;
    chooser->on_multi_select = NULL;
    chooser->on_confirm = NULL;
    chooser->callback_data = NULL;
    
    widget->focusable = VENOM_TRUE;
    load_directory(chooser);
}

static void file_chooser_destroy(VenomWidget* widget) {
    VenomFileChooser* chooser = (VenomFileChooser*)widget;
    clear_entries(chooser);
    if (chooser->entries) venom_free(chooser->entries, chooser->entry_capacity * sizeof(VenomFileEntry));
    if (chooser->current_path) venom_free(chooser->current_path, strlen(chooser->current_path) + 1);
    if (chooser->selected_path) venom_free(chooser->selected_path, strlen(chooser->selected_path) + 1);
    if (chooser->filter_pattern) venom_free(chooser->filter_pattern, strlen(chooser->filter_pattern) + 1);
    venom_widget_class.destroy(widget);
}

static void file_chooser_measure(VenomWidget* widget, VenomF32 aw, VenomF32 ah, VenomF32* w, VenomF32* h) {
    (void)aw; (void)ah;
    *w = widget->layout.preferred_width > 0 ? widget->layout.preferred_width : 400;
    *h = widget->layout.preferred_height > 0 ? widget->layout.preferred_height : 300;
}

static void file_chooser_draw(VenomWidget* widget, VenomCanvas* canvas) {
    VenomFileChooser* chooser = (VenomFileChooser*)widget;
    VenomF32 w = widget->bounds.width;
    VenomF32 h = widget->bounds.height;
    
    /* Background */
    VenomRectF bg = {0, 0, w, h};
    VenomPaint bgp = venom_paint_fill(chooser->background_color);
    venom_canvas_draw_rect(canvas, bg, &bgp);
    
    /* Path bar */
    VenomF32 y = 0;
    if (chooser->show_path_bar) {
        VenomRectF pb = {0, 0, w, PATH_BAR_HEIGHT};
        VenomPaint pbp = venom_paint_fill((VenomColor){245,245,245,255});
        venom_canvas_draw_rect(canvas, pb, &pbp);
        
        /* Up button */
        VenomPaint ip = venom_paint_fill((VenomColor){97,97,97,255});
        venom_canvas_draw_text(canvas, "⬆", 8, PATH_BAR_HEIGHT/2 + 5, NULL, &ip);
        
        /* Path */
        if (chooser->current_path) {
            VenomPaint tp = venom_paint_fill((VenomColor){33,33,33,255});
            venom_canvas_draw_text(canvas, chooser->current_path, 35, PATH_BAR_HEIGHT/2 + 5, NULL, &tp);
        }
        
        y = PATH_BAR_HEIGHT;
    }
    
    /* File list */
    VenomF32 list_h = h - y;
    VenomI32 visible_items = (VenomI32)(list_h / chooser->item_height);
    VenomI32 start_idx = chooser->scroll_offset / (VenomI32)chooser->item_height;
    
    for (VenomI32 i = start_idx; i < (VenomI32)chooser->entry_count && i < start_idx + visible_items + 1; i++) {
        VenomFileEntry* e = &chooser->entries[i];
        VenomF32 iy = y + (i - start_idx) * chooser->item_height;
        
        /* Hover/selection background */
        if (i == chooser->hover_index) {
            VenomRectF hr = {0, iy, w, chooser->item_height};
            VenomPaint hp = venom_paint_fill(chooser->item_color);
            venom_canvas_draw_rect(canvas, hr, &hp);
        }
        if (e->selected) {
            VenomRectF sr = {0, iy, w, chooser->item_height};
            VenomPaint sp = venom_paint_fill(chooser->selected_color);
            venom_canvas_draw_rect(canvas, sr, &sp);
        }
        
        /* Icon */
        VenomPaint icp = venom_paint_fill(e->is_directory ? chooser->folder_color : chooser->file_color);
        const char* icon = e->is_directory ? "📁" : "📄";
        venom_canvas_draw_text(canvas, icon, 12, iy + chooser->item_height/2 + 5, NULL, &icp);
        
        /* Name */
        VenomPaint np = venom_paint_fill((VenomColor){33,33,33,255});
        venom_canvas_draw_text(canvas, e->name, 40, iy + chooser->item_height/2 + 5, NULL, &np);
        
        /* Size for files */
        if (!e->is_directory) {
            char size_str[32];
            if (e->size >= 1024*1024) snprintf(size_str, sizeof(size_str), "%.1f MB", e->size / (1024.0*1024.0));
            else if (e->size >= 1024) snprintf(size_str, sizeof(size_str), "%.1f KB", e->size / 1024.0);
            else snprintf(size_str, sizeof(size_str), "%zu B", e->size);
            
            VenomPaint szp = venom_paint_fill((VenomColor){158,158,158,255});
            venom_canvas_draw_text(canvas, size_str, w - 80, iy + chooser->item_height/2 + 5, NULL, &szp);
        }
    }
}

static VenomBool file_chooser_on_event(VenomWidget* widget, const VenomEvent* event) {
    VenomFileChooser* chooser = (VenomFileChooser*)widget;
    VenomF32 y_off = chooser->show_path_bar ? PATH_BAR_HEIGHT : 0;
    
    if (event->type == VENOM_EVENT_MOUSE_MOVE) {
        VenomF32 my = (VenomF32)event->mouse.y - y_off + chooser->scroll_offset;
        VenomI32 new_hover = (my >= 0) ? (VenomI32)(my / chooser->item_height) : -1;
        if (new_hover >= (VenomI32)chooser->entry_count) new_hover = -1;
        if (new_hover != chooser->hover_index) {
            chooser->hover_index = new_hover;
            widget->needs_redraw = VENOM_TRUE;
        }
    }
    
    if (event->type == VENOM_EVENT_MOUSE_BUTTON_DOWN && event->mouse.button == VENOM_MOUSE_BUTTON_LEFT) {
        VenomF32 mx = (VenomF32)event->mouse.x;
        VenomF32 my = (VenomF32)event->mouse.y;
        
        /* Up button */
        if (chooser->show_path_bar && my < PATH_BAR_HEIGHT && mx < 30) {
            venom_file_chooser_go_up(chooser);
            return VENOM_TRUE;
        }
        
        /* Select item */
        if (chooser->hover_index >= 0) {
            VenomFileEntry* e = &chooser->entries[chooser->hover_index];
            
            if (e->is_directory) {
                /* Navigate into directory */
                if (chooser->current_path) venom_free(chooser->current_path, strlen(chooser->current_path) + 1);
                chooser->current_path = strdup_alloc(e->path);
                chooser->scroll_offset = 0;
                load_directory(chooser);
            } else {
                /* Select file */
                e->selected = !e->selected;
                if (chooser->selected_path) venom_free(chooser->selected_path, strlen(chooser->selected_path) + 1);
                chooser->selected_path = strdup_alloc(e->path);
                
                if (chooser->on_select) {
                    chooser->on_select(chooser, e->path, chooser->callback_data);
                }
            }
            widget->needs_redraw = VENOM_TRUE;
            return VENOM_TRUE;
        }
    }
    
    if (event->type == VENOM_EVENT_MOUSE_BUTTON_DOWN && event->mouse.button == VENOM_MOUSE_BUTTON_LEFT) {
        /* Double-click to confirm */
        VenomFileEntry* e = chooser->hover_index >= 0 ? &chooser->entries[chooser->hover_index] : NULL;
        if (e && !e->is_directory && chooser->on_confirm) {
            chooser->on_confirm(chooser, e->path, chooser->callback_data);
        }
    }
    
    if (event->type == VENOM_EVENT_MOUSE_SCROLL) {
        chooser->scroll_offset -= (VenomI32)(event->scroll.y * chooser->item_height * 3);
        if (chooser->scroll_offset < 0) chooser->scroll_offset = 0;
        VenomI32 max = (VenomI32)(chooser->entry_count * chooser->item_height - widget->bounds.height);
        if (max < 0) max = 0;
        if (chooser->scroll_offset > max) chooser->scroll_offset = max;
        widget->needs_redraw = VENOM_TRUE;
        return VENOM_TRUE;
    }
    
    return VENOM_FALSE;
}

const VenomWidgetClass venom_file_chooser_class = {
    .class_name = "VenomFileChooser",
    .instance_size = sizeof(VenomFileChooser),
    .parent_class = &venom_widget_class,
    .init = file_chooser_init,
    .destroy = file_chooser_destroy,
    .measure = file_chooser_measure,
    .draw = file_chooser_draw,
    .on_event = file_chooser_on_event,
};

VenomResultPtr venom_file_chooser_create(void) { return venom_widget_create(&venom_file_chooser_class); }

void venom_file_chooser_set_mode(VenomFileChooser* c, VenomFileChooserMode m) { if (c) c->mode = m; }

void venom_file_chooser_set_path(VenomFileChooser* chooser, const char* path) {
    if (!chooser) return;
    if (chooser->current_path) venom_free(chooser->current_path, strlen(chooser->current_path) + 1);
    chooser->current_path = strdup_alloc(path);
    chooser->scroll_offset = 0;
    load_directory(chooser);
    venom_widget_invalidate((VenomWidget*)chooser);
}

const char* venom_file_chooser_get_path(const VenomFileChooser* c) { return c ? c->current_path : NULL; }
const char* venom_file_chooser_get_selected(const VenomFileChooser* c) { return c ? c->selected_path : NULL; }

void venom_file_chooser_set_filter(VenomFileChooser* chooser, const char* pattern) {
    if (!chooser) return;
    if (chooser->filter_pattern) venom_free(chooser->filter_pattern, strlen(chooser->filter_pattern) + 1);
    chooser->filter_pattern = strdup_alloc(pattern);
    load_directory(chooser);
    venom_widget_invalidate((VenomWidget*)chooser);
}

void venom_file_chooser_set_show_hidden(VenomFileChooser* c, VenomBool s) {
    if (c) { c->show_hidden = s; load_directory(c); venom_widget_invalidate((VenomWidget*)c); }
}

void venom_file_chooser_refresh(VenomFileChooser* c) {
    if (c) { load_directory(c); venom_widget_invalidate((VenomWidget*)c); }
}

void venom_file_chooser_go_up(VenomFileChooser* chooser) {
    if (!chooser || !chooser->current_path) return;
    char* last_slash = strrchr(chooser->current_path, '/');
    if (last_slash && last_slash != chooser->current_path) {
        *last_slash = '\0';
    } else if (last_slash == chooser->current_path) {
        chooser->current_path[1] = '\0';
    }
    chooser->scroll_offset = 0;
    load_directory(chooser);
    venom_widget_invalidate((VenomWidget*)chooser);
}

void venom_file_chooser_set_on_select(VenomFileChooser* c, VenomFileCallback cb, void* data) {
    if (c) { c->on_select = cb; c->callback_data = data; }
}

void venom_file_chooser_set_on_confirm(VenomFileChooser* c, VenomFileCallback cb, void* data) {
    if (c) { c->on_confirm = cb; c->callback_data = data; }
}

VenomWidget* _venom_file_chooser_build(const VenomFileChooserConfig* config) {
    VenomResultPtr r = venom_file_chooser_create();
    if (!r.ok) return NULL;
    VenomFileChooser* c = (VenomFileChooser*)r.value;
    c->mode = config->mode;
    if (config->initial_path) venom_file_chooser_set_path(c, config->initial_path);
    if (config->filter) venom_file_chooser_set_filter(c, config->filter);
    c->on_confirm = config->on_confirm;
    c->callback_data = config->data;
    return (VenomWidget*)c;
}
