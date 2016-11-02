#ifndef HEADER_H
#define HEADER_H

/*
Authors: Dustin Lapierre, Albert Sebastian
Class: CSI-385-02
Assignment: FAT12 Filesystem
Created: 10.14.2016
Helper Functions
functions used throughout project

Certification of Authenticity:
I certify that this assignment is entirely my own work.
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

char **parsePath(char * pathname);

#endif
