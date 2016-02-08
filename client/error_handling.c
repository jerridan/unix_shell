/****************************************************************************
 * error_handling.c
 *
 * Computer Science 3305b - Spring 2016
 * Author: Jerridan Quiring
 *
 * Defines actions to be taken in case of various errors
****************************************************************************/

#include "error_handling.h"

// Logs a memory allocation error message and terminates the program
void handle_memory_error() {
  syslog(LOG_ERR, "Memory allocation error. Aborting.");
  closelog();
  exit(EXIT_FAILURE);
}

// Logs a fork error message and terminates the program
void handle_fork_error() {
  syslog(LOG_ERR, "Shell fork unsuccessful. Aborting.");
  closelog();
  exit(EXIT_FAILURE);
}

// Logs a wait error message and terminates the program
void handle_wait_error() {
  syslog(LOG_ERR, "Error waiting for child process. Aborting.");
  closelog();
  exit(EXIT_FAILURE);
}