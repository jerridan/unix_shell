/****************************************************************************
 * error_handling.h
 *
 * Computer Science 3305b - Spring 2016
 * Author: Jerridan Quiring
 *
 * Defines actions to be taken in case of various errors
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>

 // Logs a memory allocation error message and terminates the program
void handle_memory_error();

// Logs a fork error message and terminates the program
void handle_fork_error();

// Logs a wait error message and terminates the program
void handle_wait_error();