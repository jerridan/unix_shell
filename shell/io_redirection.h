/****************************************************************************
 * io_redirection.h
 *
 * Computer Science 3305b - Spring 2016
 * Author: Jerridan Quiring
 *
 * Provides support for executing commands with IO redirection
****************************************************************************/

#ifndef IO_REDIRECTION_H
#define IO_REDIRECTION_H

#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <sys/stat.h>

#include "execution.h"

// Executes a command that contains IO-redirection
int handle_redirected_commands(char **args);

#endif
