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

  if(num_pipes > 0) {
    const int buffer_increment = 10;
    for (int i = 0; i < num_pipes; i++) {
      
      int buffer_size_1 = buffer_increment;
      int buffer_size_2 = buffer_increment;
      int num_args_1 = 0;
      int num_args_2 = 0;

      char **command1 = calloc(buffer_size_1, sizeof(char*));
      char **command2 = calloc(buffer_size_2, sizeof(char*));

      int position = 0;

      while(0 != strcmp("|", args[position])) {
        // Reallocate more space if necessary
        if(num_args_1 == buffer_size_1) {
          buffer_size_1 += buffer_increment;
          command1 = realloc(command1, buffer_size_1);
        }
        command1[position] = args[position];
        num_args_1++;
        position++;
      }

      position++; // Move past pipe argument
      while(NULL != args[position]) {
        // Reallocate more space if necessary
        if(num_args_2 == buffer_size_2) {
          buffer_size_2 += buffer_increment;
          command2 = realloc(command2, buffer_size_2);
        }
        command2[num_args_2] = args[position];
        num_args_2++;
        position++;
      }

      // Print command 1
      position = 0;
      printf("Command 1: \n");
      while(NULL != command1[position]) {
        printf("%s ", command1[position++]);
      }
      printf("\n");

      // Print command 2
      position = 0;
      printf("Command 2: \n");
      while(NULL != command2[position]) {
        printf("%s ", command2[position++]);
      }
      printf("\n");
    }

  } else {
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












