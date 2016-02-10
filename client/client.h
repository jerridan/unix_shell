/****************************************************************************
 * client.h
 *
 * Computer Science 3305b - Spring 2016
 * Author: Jerridan Quiring
 *
 * Implements a Unix-like terminal
****************************************************************************/

#ifndef CLIENT_H
#define CLIENT_H

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "error_handling.h"
#include "piping.h"
#include "execution.h"

#define KEEP_IN_HISTORY 11 // 10 + 1, incl history cmd on printout

int main();

// Identifies and executes command arguments
int handle_commands(char **args, char ***history);

// Executes a command that contains IO-redirection
int handle_redirected_commands(char **args);

// Processes command-line input into separate arguments
char** process_input(char *input);

// Returns the name of the user executing the terminal
char* get_username();

// Returns a line of input from the terminal
char* read_input();

// Sets parent to wait for first child process to execute
void wait_for_first_child();

// Set up termination signal for first child
void setup_child_term_signal();

// Termination handler for interrupt signal (Ctrl+C)
void handle_termination(int signal);

// Prints the command history to stdout
void print_history(char **history);

#endif