#include "config_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void init_default_config(VaxpTermConfig* config) {
    config->default_fg = vaxp_color_rgb(220, 220, 220);
    config->default_bg = vaxp_color_rgba(0, 0, 0, 100);
    config->font_size = 14.0f;
}

/* Helper to parse hex color like #RRGGBB or #RRGGBBAA */
static VaxpColor parse_hex_color(const char* hex) {
    if (hex[0] == '#') hex++;
    int len = strlen(hex);
    
    unsigned int r=0, g=0, b=0, a=255;
    if (len >= 6) {
        sscanf(hex, "%02x%02x%02x", &r, &g, &b);
        if (len >= 8) {
            sscanf(hex + 6, "%02x", &a);
        }
    }
    return vaxp_color_rgba(r, g, b, a);
}

/* Trim whitespace from string */
static char* trim_whitespace(char* str) {
    while(isspace((unsigned char)*str)) str++;
    if(*str == 0) return str;
    
    char* end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return str;
}

int load_vaxp_term_config(const char* filepath, VaxpTermConfig* config) {
    init_default_config(config);
    
    FILE* file = fopen(filepath, "r");
    if (!file) return 0;
    
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char* trimmed = trim_whitespace(line);
        if (trimmed[0] == '\0' || trimmed[0] == '#' || trimmed[0] == ';') {
            continue; /* Skip comments and empty lines */
        }
        
        char* eq = strchr(trimmed, '=');
        if (eq) {
            *eq = '\0';
            char* key = trim_whitespace(trimmed);
            char* val = trim_whitespace(eq + 1);
            
            if (strcmp(key, "foreground") == 0) {
                config->default_fg = parse_hex_color(val);
            } else if (strcmp(key, "background") == 0) {
                config->default_bg = parse_hex_color(val);
            } else if (strcmp(key, "font_size") == 0) {
                config->font_size = atof(val);
            }
        }
    }
    
    fclose(file);
    return 1;
}
