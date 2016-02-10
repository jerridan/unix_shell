/****************************************************************************
 * execution.c
 *
 * Computer Science 3305b - Spring 2016
 * Author: Jerridan Quiring
 *
 * Contains functionality necessary for single command execution
****************************************************************************/

#include "execution.h"

// Executes a command, given a fd to read from and fd to write to
// Return -1 on error
int execute_command(char **cmd, int read_fd, int write_fd) {

  int child_pid = fork();
  if(child_pid < 0) { // Fork error
    syslog(LOG_ERR, "Fork error. Unable to execute command.");
    errno = 0; // Reset errno
    return -1;

  } else if(child_pid > 0) { // Parent
    int status; // Status of child process
    pid_t wpid; // PID returned by wait

    // Wait for child to execute command
    wpid = wait(&status);
    if(-1 == wpid) {
      handle_wait_error();
      return -1;
    }

  } else { // Child
    if(STDIN_FILENO != read_fd) {
      dup2(read_fd, STDIN_FILENO);
    }
    if(STDOUT_FILENO != write_fd) {
      dup2(write_fd, STDOUT_FILENO);
    }
    int result = execvp(cmd[0], cmd);
    if(result < 0) {
      syslog(LOG_ERR, "%s", strerror(errno));
      errno = 0; // Reset errno
      return -1;
    }
  }
  
  return 0;
}