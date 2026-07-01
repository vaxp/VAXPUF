#ifndef PTY_HANDLER_H
#define PTY_HANDLER_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Starts a new shell session using forkpty.
 * 
 * @param master_fd Pointer to store the master file descriptor of the PTY.
 * @param pid Pointer to store the process ID of the child shell.
 * @param cols Initial columns of the terminal.
 * @param rows Initial rows of the terminal.
 * @return 0 on success, -1 on failure.
 */
int pty_start(int* master_fd, pid_t* pid, int cols, int rows);

/**
 * @brief Resizes the PTY window size.
 * 
 * @param master_fd The master file descriptor of the PTY.
 * @param cols New columns.
 * @param rows New rows.
 * @return 0 on success, -1 on failure.
 */
int pty_resize(int master_fd, int cols, int rows);

#ifdef __cplusplus
}
#endif

#endif /* PTY_HANDLER_H */
