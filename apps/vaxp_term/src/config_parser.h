#ifndef VAXP_TERM_CONFIG_H
#define VAXP_TERM_CONFIG_H

#include <vaxp/vaxpui.h>

typedef struct {
    VaxpColor default_fg;
    VaxpColor default_bg;
    float font_size;
} VaxpTermConfig;

/* Load config from a file path. Returns 1 on success, 0 on failure. */
int load_vaxp_term_config(const char* filepath, VaxpTermConfig* config);

/* Initialize with default values */
void init_default_config(VaxpTermConfig* config);

#endif
