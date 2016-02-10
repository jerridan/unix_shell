/****************************************************************************
 * piping.c
 *
 * Computer Science 3305b - Spring 2016
 * Author: Jerridan Quiring
 *
 * Provides support for executing commands with pipes
****************************************************************************/

#include "piping.h"

// Returns an array of commands, each to be piped into the next
char*** get_piping_commands(int num_pipes, char** args) {
  const int buffer_increment = 10;
  int buffer_size;

  // Initialize commands array
  char ***cmds = calloc(num_pipes + 1, sizeof(char**));

  int position = 0;
  int num_args;

  // Add every command preceding a pipe to the commands array
  for (int i = 0; i < num_pipes; i++) {
    buffer_size = buffer_increment;
    cmds[i] = calloc(buffer_size, sizeof(char*));

    num_args = 0;

    while (0 != strcmp("|", args[position])) {
      // Reallocate more space if necessary
      if(num_args == buffer_size) {
        buffer_size += buffer_increment;
        cmds[i] = realloc(cmds[i], buffer_size * sizeof(char*));
      }
      cmds[i][num_args] = args[position];
      num_args++;
      position++;
    }

    position++; // Skip pipe command, we don't want it
  }

  // Get the last command
  buffer_size = buffer_increment;
  num_args = 0;
  cmds[num_pipes] = calloc(buffer_size, sizeof(char*));
  while(NULL != args[position]) {
    // Reallocate more space if necessary
    if(num_args == buffer_size) {
      buffer_size += buffer_increment;
      cmds[num_pipes] = realloc(cmds[num_pipes], buffer_size * sizeof(char*));
    }
    cmds[num_pipes][num_args] = args[position];
    num_args++;
    position++;
  }

  return cmds;
}

// Executes a command that contains pipes
int handle_piped_commands(int num_pipes, char **args) {
  // Get array of commands to pipe
  char ***cmds = get_piping_commands(num_pipes, args);

  int fd[2]; // Holds file descriptors of pipe ends
  if(pipe(fd) < 0) {
    syslog(LOG_ERR, "Pipe error. Unable to execute command.");
    return -1;
  }

  // Execute first command - reads from STDIN
  int result = execute_command(cmds[0], STDIN_FILENO, fd[1]);
  if(-1 == result) {
    return -1;
  }

  close(fd[1]);         // Close write end

  // Execute any commands in-between pipes
  for (int i = 1; i < num_pipes; i++) {
    int temp_fd[2];     // Holds file descriptors of pipe ends
    if(pipe(temp_fd) < 0) {
      syslog(LOG_ERR, "Pipe error. Unable to execute command.");
      return -1;
    }
    fd[1] = temp_fd[1]; // Point to write end of new pipe

    result = execute_command(cmds[i], fd[0], fd[1]);
    if(-1 == result) {
      return -1;
    }

    close(fd[1]);       // Close write end
    close(fd[0]);       // Close read end
    fd[0] = temp_fd[0]; // Point to read end of new pipe
  }

  // Execute last command - writes to STDOUT
  result = execute_command(cmds[num_pipes], fd[0], STDOUT_FILENO);
  if(-1 == result) {
    return -1;
  }

  close(fd[0]);

  return 0;
}