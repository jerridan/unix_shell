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
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "error_handling.h"

int main();

// Identifies and executes command arguments
int execute_command(char** args);

// Returns an array of commands, each to be piped into the next
char*** get_piping_commands(int num_pipes, char** args);

// Executes a command that contains pipes
int handle_pipe_commands(int num_pipes, char **args);

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

#endif