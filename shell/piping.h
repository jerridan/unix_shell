/****************************************************************************
 * piping.h
 *
 * Computer Science 3305b - Spring 2016
 * Author: Jerridan Quiring
 *
 * Provides support for executing commands with pipes
****************************************************************************/

#ifndef PIPING_H
#define PIPING_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

#include "execution.h"

// Returns an array of commands, each to be piped into the next
char*** get_piping_commands(int num_pipes, char** args);

// Executes a command that contains pipes
int handle_piped_commands(int num_pipes, char **args);

#endif