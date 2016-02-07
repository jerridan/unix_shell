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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

int main();

// Returns the name of the user executing the terminal
char* get_username();

// Returns a line of input from the terminal
char* read_input();

#endif