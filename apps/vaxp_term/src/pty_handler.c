#include "pty_handler.h"
#include <pty.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <string.h>

int pty_start(int* master_fd, pid_t* pid, int cols, int rows) {
    struct winsize win = {0};
    win.ws_col = cols;
    win.ws_row = rows;

    /* Create PTY and fork */
    pid_t child = forkpty(master_fd, NULL, NULL, &win);
    if (child < 0) {
        perror("forkpty");
        return -1;
    }

    if (child == 0) {
        /* Child process (Shell) */
        
        /* Set TERM environment variable */
        setenv("TERM", "xterm-256color", 1);
        
        /* Execute bash */
        char* args[] = {"/bin/bash", NULL};
        execv(args[0], args);
        
        /* Should not reach here */
        perror("execv");
        exit(1);
    }

    /* Parent process */
    *pid = child;
    
    /* Make master_fd non-blocking if needed, but we'll use poll/select in a thread */
    return 0;
}

int pty_resize(int master_fd, int cols, int rows) {
    struct winsize win = {0};
    win.ws_col = cols;
    win.ws_row = rows;
    if (ioctl(master_fd, TIOCSWINSZ, &win) < 0) {
        return -1;
    }
    return 0;
}
