/*
 * VENOMUI - FileChooser widget header
 */

#ifndef VENOM_FILE_CHOOSER_H
#define VENOM_FILE_CHOOSER_H

#include "venom/widgets/venom_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VenomFileChooser VenomFileChooser;

typedef enum VenomFileChooserMode {
    VENOM_FILE_CHOOSER_OPEN,
    VENOM_FILE_CHOOSER_SAVE,
    VENOM_FILE_CHOOSER_SELECT_FOLDER,
    VENOM_FILE_CHOOSER_OPEN_MULTIPLE,
} VenomFileChooserMode;

typedef struct VenomFileEntry {
    char* name;
    char* path;
    VenomBool is_directory;
    VenomSize size;
    VenomBool selected;
} VenomFileEntry;

typedef void (*VenomFileCallback)(VenomFileChooser* chooser, const char* path, void* data);
typedef void (*VenomMultiFileCallback)(VenomFileChooser* chooser, const char** paths, VenomU32 count, void* data);
typedef VenomBool (*VenomFileFilter)(const char* filename, void* data);

struct VenomFileChooser {
    VenomWidget base;
    
    VenomFileChooserMode mode;
    char* current_path;
    char* selected_path;
    char* filename;
    
    VenomFileEntry* entries;
    VenomU32 entry_count;
    VenomU32 entry_capacity;
    
    char** selected_paths;
    VenomU32 selected_count;
    
    VenomFileFilter filter;
    void* filter_data;
    char* filter_pattern;     /* e.g. "*.txt;*.md" */
    
    VenomBool show_hidden;
    VenomBool show_path_bar;
    
    VenomI32 hover_index;
    VenomI32 scroll_offset;
    VenomF32 item_height;
    
    VenomColor background_color;
    VenomColor item_color;
    VenomColor selected_color;
    VenomColor folder_color;
    VenomColor file_color;
    
    VenomFileCallback on_select;
    VenomMultiFileCallback on_multi_select;
    VenomFileCallback on_confirm;
    void* callback_data;
};

VenomResultPtr venom_file_chooser_create(void);
void venom_file_chooser_set_mode(VenomFileChooser* chooser, VenomFileChooserMode mode);
void venom_file_chooser_set_path(VenomFileChooser* chooser, const char* path);
const char* venom_file_chooser_get_path(const VenomFileChooser* chooser);
const char* venom_file_chooser_get_selected(const VenomFileChooser* chooser);
void venom_file_chooser_set_filter(VenomFileChooser* chooser, const char* pattern);
void venom_file_chooser_set_show_hidden(VenomFileChooser* chooser, VenomBool show);
void venom_file_chooser_refresh(VenomFileChooser* chooser);
void venom_file_chooser_go_up(VenomFileChooser* chooser);
void venom_file_chooser_set_on_select(VenomFileChooser* chooser, VenomFileCallback cb, void* data);
void venom_file_chooser_set_on_confirm(VenomFileChooser* chooser, VenomFileCallback cb, void* data);

extern const VenomWidgetClass venom_file_chooser_class;

#define venom_file_chooser(...) _venom_file_chooser_build(&(VenomFileChooserConfig){ __VA_ARGS__ })

typedef struct VenomFileChooserConfig {
    VenomFileChooserMode mode;
    const char* initial_path;
    const char* filter;
    VenomFileCallback on_confirm;
    void* data;
} VenomFileChooserConfig;

VenomWidget* _venom_file_chooser_build(const VenomFileChooserConfig* config);

#ifdef __cplusplus
}
#endif

#endif
