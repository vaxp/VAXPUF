/*
 * VAXPUI - FileChooser widget header
 */

#ifndef VAXP_FILE_CHOOSER_H
#define VAXP_FILE_CHOOSER_H

#include "vaxp/widgets/vaxp_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VaxpFileChooser VaxpFileChooser;

typedef enum VaxpFileChooserMode {
    VAXP_FILE_CHOOSER_OPEN,
    VAXP_FILE_CHOOSER_SAVE,
    VAXP_FILE_CHOOSER_SELECT_FOLDER,
    VAXP_FILE_CHOOSER_OPEN_MULTIPLE,
} VaxpFileChooserMode;

typedef struct VaxpFileEntry {
    char* name;
    char* path;
    VaxpBool is_directory;
    VaxpSize size;
    VaxpBool selected;
} VaxpFileEntry;

typedef void (*VaxpFileCallback)(VaxpFileChooser* chooser, const char* path, void* data);
typedef void (*VaxpMultiFileCallback)(VaxpFileChooser* chooser, const char** paths, VaxpU32 count, void* data);
typedef VaxpBool (*VaxpFileFilter)(const char* filename, void* data);

struct VaxpFileChooser {
    VaxpWidget base;
    
    VaxpFileChooserMode mode;
    char* current_path;
    char* selected_path;
    char* filename;
    
    VaxpFileEntry* entries;
    VaxpU32 entry_count;
    VaxpU32 entry_capacity;
    
    char** selected_paths;
    VaxpU32 selected_count;
    
    VaxpFileFilter filter;
    void* filter_data;
    char* filter_pattern;     /* e.g. "*.txt;*.md" */
    
    VaxpBool show_hidden;
    VaxpBool show_path_bar;
    
    VaxpI32 hover_index;
    VaxpI32 scroll_offset;
    VaxpF32 item_height;
    
    VaxpColor background_color;
    VaxpColor item_color;
    VaxpColor selected_color;
    VaxpColor folder_color;
    VaxpColor file_color;
    
    VaxpFileCallback on_select;
    VaxpMultiFileCallback on_multi_select;
    VaxpFileCallback on_confirm;
    void* callback_data;
};

VaxpResultPtr vaxp_file_chooser_create(void);
void vaxp_file_chooser_set_mode(VaxpFileChooser* chooser, VaxpFileChooserMode mode);
void vaxp_file_chooser_set_path(VaxpFileChooser* chooser, const char* path);
const char* vaxp_file_chooser_get_path(const VaxpFileChooser* chooser);
const char* vaxp_file_chooser_get_selected(const VaxpFileChooser* chooser);
void vaxp_file_chooser_set_filter(VaxpFileChooser* chooser, const char* pattern);
void vaxp_file_chooser_set_show_hidden(VaxpFileChooser* chooser, VaxpBool show);
void vaxp_file_chooser_refresh(VaxpFileChooser* chooser);
void vaxp_file_chooser_go_up(VaxpFileChooser* chooser);
void vaxp_file_chooser_set_on_select(VaxpFileChooser* chooser, VaxpFileCallback cb, void* data);
void vaxp_file_chooser_set_on_confirm(VaxpFileChooser* chooser, VaxpFileCallback cb, void* data);

extern const VaxpWidgetClass vaxp_file_chooser_class;

#define vaxp_file_chooser(...) _vaxp_file_chooser_build(&(VaxpFileChooserConfig){ __VA_ARGS__ })

typedef struct VaxpFileChooserConfig {
    VaxpFileChooserMode mode;
    const char* initial_path;
    const char* filter;
    VaxpFileCallback on_confirm;
    void* data;
} VaxpFileChooserConfig;

VaxpWidget* _vaxp_file_chooser_build(const VaxpFileChooserConfig* config);

#ifdef __cplusplus
}
#endif

#endif
