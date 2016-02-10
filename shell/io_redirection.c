/****************************************************************************
 * io_redirection.h
 *
 * Computer Science 3305b - Spring 2016
 * Author: Jerridan Quiring
 *
 * Provides support for executing commands with IO redirection
****************************************************************************/

#include "io_redirection.h"

// Executes a command that contains IO-redirection
int handle_redirected_commands(char **args) {
  const int buffer_increment = 10;
  int buffer_size = buffer_increment;
  int position = 0;

  char **command1 = calloc(buffer_size, sizeof(char*));
  int num_args = 0;

  // Get the command to the left of the redirect
  for(int i = 0; (0 != strcmp(">", args[i])) && (0 != strcmp("<", args[i])); i++) {
    // Reallocate more space if necessary
    if(num_args == buffer_size) {
      buffer_size += buffer_increment;
      command1 = realloc(command1, buffer_size * sizeof(char*));
    }
    command1[num_args] = args[i];
    position++;
    num_args++;
  }

  // Is this an input redirect?
  bool input_redirect = (0 == strcmp("<", args[position]));
  position++;

  int result; // Result of command execution
  if(!input_redirect) {
    // Create the output file or truncate to 0 size
    int output_fd = open(args[position], O_WRONLY | O_CREAT | O_TRUNC, 0755);

    // Execute command and put output in file
    result = execute_command(command1, STDIN_FILENO, output_fd);
    close(output_fd);
  } else {

    // Input file
    int input_fd = open(args[position], O_RDONLY, 0755);
    int output_fd;

    if(NULL == args[++position]) {
      // Input redirection
      result = execute_command(command1, input_fd, STDOUT_FILENO);
    } else {
      // Input and output redirection
      output_fd = open(args[++position], O_WRONLY | O_CREAT | O_TRUNC, 0755);
      result = execute_command(command1, input_fd, output_fd);
    }
  }

  return result;
}