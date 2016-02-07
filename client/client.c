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

  // Print terminal prompt
  char *username = get_username();
  printf("%s> ", username);

  // Read input from terminal
  char *input = read_input();
  syslog(LOG_DEBUG, "User input: %s", input);
  exit(EXIT_SUCCESS);
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
  char current; // Current char being read from input

  do {
    current = getchar();
    if(bytes_read == buffer_size) {
      // If buffer is full, reallocate more space
      buffer = realloc(buffer, bytes_read + buffer_increment);

      if(NULL == buffer) {
        syslog(LOG_ERR, "Memory allocation error. Aborting.");
        exit(EXIT_FAILURE);
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




























