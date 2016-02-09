/****************************************************************************
 * client.c
 *
 * Computer Science 3305b - Spring 2016
 * Author: Jerridan Quiring
 *
 * Implements a Unix-like terminal
****************************************************************************/

#include "client.h"

int main() {
  //Open call to syslog
  openlog("shell_client", LOG_PERROR | LOG_PID | LOG_NDELAY, LOG_USER);
  setlogmask(LOG_UPTO(LOG_DEBUG));

  // Create first child process - parent of all other processes
  pid_t pid_first_child = fork();
  if(pid_first_child < 0) {
    handle_fork_error();
    closelog();
    exit(EXIT_FAILURE);

  } else if(pid_first_child > 0) { // Parent process
    wait_for_first_child();
    closelog();
    exit(EXIT_SUCCESS);

  } else { // Child process

    char *username = get_username();
    while(1) {
      // Print terminal prompt
      printf("%s> ", username);

      // Read input from terminal
      char *input = read_input();

      // Process the input into arguments
      char **args = process_input(input);

      // Execute command arguments
      execute_command(args);
    }

    free(username);
    exit(EXIT_SUCCESS);
  }
}

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
int handle_pipe_commands(int num_pipes, char **args) {
  // Get array of commands to pipe
  char ***cmds = get_piping_commands(num_pipes, args);

  char **command1 = cmds[0];
  char **command2 = cmds[1];

  int fd[2]; // Holds file descriptors of pipe ends
  if(pipe(fd) < 0) {
    syslog(LOG_ERR, "Pipe error. Unable to execute command.");
    return -1;
  }

  int pid_child1, pid_child2;
  pid_child1 = fork();

  if(pid_child1 < 0) {
    handle_fork_error();
    return -1;
  } 

  if(pid_child1 > 0) {
    pid_child2 = fork();
    if(pid_child2 < 0) {
      handle_fork_error();
      return -1;
    } 
  }

  if(pid_child1 > 0 && pid_child2 > 0) { // Parent
    // Close pipe at both ends, since parent doesn't need them
    close(fd[0]);
    close(fd[1]);
    
    // Wait for both children
    for(int i = 0; i < 2; i++) {
      int wpid = wait(NULL);
      if(wpid < 0) {
        handle_wait_error();
        return -1;
      }
    }
  }

  if(0 == pid_child1) { // Child 1
    close(fd[0]);
    if(dup2(fd[1], STDOUT_FILENO) < 0) {
      syslog(LOG_ERR, "Pipe error");
    }
    execvp(command1[0], command1);
  }

  if(pid_child1 > 0 && pid_child2 == 0) { // Child 2
    close(fd[1]);
    dup2(fd[0], STDIN_FILENO);
    execvp(command2[0], command2);
  }

  return 0;
}

// Identifies and executes command arguments
// Returns -1 on error
int execute_command(char** args) {

  // Check for exit command
  if(0 == strcmp("exit", args[0])) {
    exit(EXIT_SUCCESS);
  }

  // Count number of pipes
  int num_pipes = 0;
  for (int i = 0; NULL != args[i]; i++) {
    if(0 == strcmp("|", args[i])) {
      num_pipes++;
    }
  }

  if(num_pipes > 0) { // Executing a command with piping
    return handle_pipe_commands(num_pipes, args);

  } else { // Just a regular command to execute
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
      int result = execvp(args[0], args);
      if(result < 0) {
        syslog(LOG_ERR, "%s", strerror(errno));
        errno = 0; // Reset errno
        exit(EXIT_FAILURE);
      }
    }
  }

  
  return 0;
}

// Processes command-line input into separate arguments
char** process_input(char *input) {
  // Begin with small buffer, will realloc if needed
  const int buffer_increment = 10;
  int buffer_size = buffer_increment;

  // Array of string token arguments
  char **args = calloc(buffer_size, sizeof(char*));

  // Input will be separated by whitespace, newlines and returns
  const char separator[4] = " \n\r";
  char *token = NULL; // Current string token
  int num_args = 0;   // Number of arguments read
  
  // Split input into arguments
  token = strtok(input, separator);
  while(NULL != token) {
    // Increase buffer if necessary
    if(num_args == buffer_size) {
      buffer_size += buffer_increment;
      printf("buffer increased to %d\n", buffer_size);
      args = realloc(args, buffer_size * sizeof(char*));
    }
    args[num_args] = calloc(strlen(token)+1, sizeof(char));
    strcpy(args[num_args], token);
    num_args++;
    token = strtok(NULL, separator);
  }
  free(input);
  return args;
}

// Returns the name of the user executing the terminal
char* get_username() {
  char *login;    // Login read from system
  char *username; // Pointer to copied login name
  if(NULL == (login = getlogin())) {
    username = "user";
    syslog(LOG_DEBUG, "Unable to determine username. Defaulting to 'user'");
    errno = 0; // Reset errno
    return username;
  }

  // Copy login to username as memory may be overwritten later
  username = malloc(strlen(login) + 1);
  strcpy(username, login);
  return username;
}

// Returns a line of input from the terminal
char* read_input() {

  // Buffer will be allocated by getline
  char *buffer = NULL;
  size_t buffer_size = 0;

  // Read a line from standard input
  getline(&buffer, &buffer_size, stdin);

  return buffer;
}

// Sets parent to wait for first child process to execute
void wait_for_first_child() {
  int status; // Status of first child process
  pid_t wpid; // PID returned by wait

  // Ignore SIGINT for parent process
  struct sigaction new_action;
  memset(&new_action, 0, sizeof(new_action));
  new_action.sa_handler = SIG_IGN;
  sigaction(SIGINT, &new_action, NULL);

  do {
    // Wait for child to finish
    wpid = wait(&status);

    if(-1 == wpid) {
      handle_wait_error();
      closelog();
      exit(EXIT_FAILURE);
    }

    // Did child exit on its own?
    if(WIFEXITED(status)) {
      syslog(LOG_DEBUG, "Child 1 exited, status=%d", WEXITSTATUS(status));
    }
    
    // Was child killed?
    if(WIFSIGNALED(status)) {
      syslog(LOG_DEBUG, "Child 1 killed, status=%d", WTERMSIG(status));
    }

    // Was child stopped?
    if(WIFSTOPPED(status)) {
      syslog(LOG_DEBUG, "Child 1 stopped, status=%d", WSTOPSIG(status));
    }

    // Was child continued?
    if(WIFCONTINUED(status)) {
      syslog(LOG_DEBUG, "Child 1 continued");
    }

  } while(!WIFEXITED(status) && !WIFSIGNALED(status));
}

// Set up termination signal for first child
void setup_child_term_signal() {
  // Actions for connection termination handler
  struct sigaction new_action;
  memset(&new_action, 0, sizeof(new_action));
  new_action.sa_handler = &handle_termination;
  sigaction(SIGINT, &new_action, NULL);
}

// Termination handler for interrupt signal (Ctrl+C)
void handle_termination(int signal) {
  syslog(LOG_INFO, "Termination requested with signal: %d", signal);
  abort();
}












