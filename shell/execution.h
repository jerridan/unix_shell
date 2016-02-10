/****************************************************************************
 * execution.h
 *
 * Computer Science 3305b - Spring 2016
 * Author: Jerridan Quiring
 *
 * Contains functionality necessary for single command execution
****************************************************************************/

#ifndef EXECUTION_H
#define EXECUTION_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "error_handling.h"

// Executes a command, given a fd to read from and fd to write to
// Return -1 on error
int execute_command(char **cmd, int read_fd, int write_fd);

#endif