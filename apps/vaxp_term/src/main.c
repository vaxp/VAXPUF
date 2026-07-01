#include <vaxp/vaxpui.h>
#include "pty_handler.h"
#include "vaxp_terminal.h"
#include "config_parser.h"
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

static int g_pty_fd = -1;
static pid_t g_shell_pid = -1;
static VaxpWidget* g_terminal = NULL;
static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;
static VaxpTermConfig g_config;

/* Background thread to read from PTY */
static void* pty_reader_thread(void* arg) {
    (void)arg;
    char buffer[1024];
    while (1) {
        ssize_t n = read(g_pty_fd, buffer, sizeof(buffer));
        if (n <= 0) break; /* Shell exited or error */
        
        pthread_mutex_lock(&g_mutex);
        if (g_terminal) {
            vaxp_terminal_write((VaxpTerminal*)g_terminal, buffer, n);
        }
        pthread_mutex_unlock(&g_mutex);
        
        /* Request UI redraw */
        if (g_terminal) {
            vaxp_widget_invalidate(g_terminal);
        }
    }
    return NULL;
}

static VaxpWidget* build_ui(void) {
    pthread_mutex_lock(&g_mutex);
    g_terminal = vaxp_term_widget(
        .cols = 80,
        .rows = 24,
        .font_size = g_config.font_size,
        .pty_fd = g_pty_fd
    );
    
    /* Apply config colors */
    VaxpTerminal* t = (VaxpTerminal*)g_terminal;
    t->default_fg = g_config.default_fg;
    t->default_bg = g_config.default_bg;
    t->current_fg = g_config.default_fg;
    t->current_bg = g_config.default_bg;
    
    /* VaxpTerminal handles stretching itself */
    VaxpWidget* root = vaxp_col(
        .align = VAXP_ALIGN_STRETCH,
        .children = VAXP_CHILDREN(g_terminal)
    );
    
    /* Set initial focus to terminal so user doesn't have to click */
    vaxp_focus_set(g_terminal);
    
    pthread_mutex_unlock(&g_mutex);
    
    return root;
}

int main(int argc, char** argv) {
    (void)argc; (void)argv;
    
    /* Load config */
    char config_path[256];
    const char* home_dir = getenv("HOME");
    if (!home_dir) home_dir = ".";
    snprintf(config_path, sizeof(config_path), "%s/.config/vaxp_term.conf", home_dir);
    load_vaxp_term_config(config_path, &g_config);
    
    /* Start Shell */
    if (pty_start(&g_pty_fd, &g_shell_pid, 80, 24) < 0) {
        fprintf(stderr, "Failed to start PTY\n");
        return 1;
    }
    
    /* Start reader thread */
    pthread_t thread;
    pthread_create(&thread, NULL, pty_reader_thread, NULL);
    
    /* Run App */
    int ret = VAXP_APP(
        .title = "VAXPUF Terminal Emulator",
        .width = 800,
        .height = 600,
        .decoration = VAXP_DECORATION_DARK,
        .build = build_ui
    );
    
    pthread_mutex_lock(&g_mutex);
    g_terminal = NULL;
    pthread_mutex_unlock(&g_mutex);
    
    if (g_pty_fd >= 0) {
        close(g_pty_fd);
    }
    
    return ret;
}
