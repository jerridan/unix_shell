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
    char **history = calloc(KEEP_IN_HISTORY, sizeof(char*));
    int history_size = 0; // Current size of history buffer

    while(1) {
      // Print terminal prompt
      printf("%s> ", username);

      // Read input from terminal
      char *input = read_input();
      // Did the user just hit 'Enter'?
      if(0 == strcmp(input, "\n")) {
        continue;
      }

      // Add command to history
      if(KEEP_IN_HISTORY == history_size) {
        free(history[0]);
        for(int i = 0; i < KEEP_IN_HISTORY; i++) {
          history[i] = history[i+1];
        }
        history[KEEP_IN_HISTORY] = NULL;
        history_size--;
      }
      history[history_size] = calloc(strlen(input) + 1, sizeof(char));
      strcpy(history[history_size], input);
      history_size++;

      // Process the input into arguments
      char **args = process_input(input);

      // Execute command arguments
      handle_commands(args, &history);
    }

    free(username);
    exit(EXIT_SUCCESS);
  }
}

// Executes a command that contains IO-redirection
int handle_redirected_commands(char **args) {

   int position = 0;

   char **command1, **command2;
   int num_args = 0;

  for(int i = 0; 0 != strcmp(">", args[i]); i++ {
    command1[num_args] = args[i];
    position++;
  }

  position++;

  FILE

  num_args = 0;
  while(NULL != args[position]) {
    command2[num_args] = args[position];
    num_args++;
    position++;
  }


}

// Identifies and executes command arguments
// Returns -1 on error
int handle_commands(char **args, char ***history) {

  // Check for exit command
  if(0 == strcmp("exit", args[0])) {
    exit(EXIT_SUCCESS);
  }

  // Check for history command
  if(0 == strcmp("history", args[0])) {
    print_history(*history);
    return 0;
  }

  // Count number of pipes
  int num_pipes = 0;
  for (int i = 0; NULL != args[i]; i++) {
    if(0 == strcmp("|", args[i])) {
      num_pipes++;
    }
  }

  if(num_pipes > 0) {
    // Executing a command with piping
    return handle_piped_commands(num_pipes, args);
  } else {            
    // Check if there is IO-redirection
    bool io_redirection = false;
    for(int i = 0; !io_redirection && NULL != args[i]; i++) {
      if((0 == strcmp("<", args[i])) || (0 == strcmp(">", args[i]))) {
        io_redirection = true;
      }
    }
    if(io_redirection) {
      // Executing a command with IO-redirection
      return handle_redirected_commands(args);
    } else {
      // Just a regular command to execute
      return execute_command(args, STDIN_FILENO, STDOUT_FILENO);
    }
  }
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

// Prints the command history to stdout
void print_history(char **history) {
  syslog(LOG_INFO, "Command history");
  for(int i = 0; NULL != history[i + 1]; i++) {
    syslog(LOG_INFO, "%s", history[i]);
  }
}











