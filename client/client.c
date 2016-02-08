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

  } else if(pid_first_child > 0) { // Parent process

    wait_for_first_child();  

    syslog(LOG_DEBUG, "Exiting client");
    closelog();
    exit(EXIT_SUCCESS);

  } else { // Child process

    // Print terminal prompt
    char *username = get_username();
    printf("%s> ", username);

    // Read input from terminal
    char *input = read_input();
    syslog(LOG_DEBUG, "User input: %s", input);
    exit(EXIT_SUCCESS);
  }
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
  const int buffer_increment = 15;
  int buffer_size = buffer_increment;               // Buffer size
  int bytes_read = 0; // Number of bytes read from input
  char *buffer = calloc(buffer_size, sizeof(char)); // Input buffer
  if(NULL == buffer) {
    handle_memory_error();
  }
  char current; // Current char being read from input

  do {
    current = getchar();
    if(bytes_read == buffer_size) {
      // If buffer is full, reallocate more space
      buffer = realloc(buffer, bytes_read + buffer_increment);

      if(NULL == buffer) {
        handle_memory_error();
      }

      buffer_size += buffer_increment; // Update buffer size
    }
    buffer[bytes_read] = current;
    bytes_read++;

  } while (EOF != current && '\n' != current);

  // Replace EOF or newline with string terminator
  buffer[--bytes_read] = '\0';

  // Remove any empty space on the buffer
  buffer = realloc(buffer, bytes_read);

  return buffer;
}

// Sets parent to wait for first child process to execute
void wait_for_first_child() {
  int status; // Status of first child process
  pid_t wpid; // PID returned by wait

  // Block SIGINT for parent process
  struct sigaction new_action;
  memset(&new_action, 0, sizeof(new_action));
  new_action.sa_handler = SIG_IGN;
  sigaction(SIGINT, &new_action, NULL);

  do {
    // Wait for child to finish
    wpid = wait(&status);

    if(wpid == -1) {
      handle_wait_error();
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












