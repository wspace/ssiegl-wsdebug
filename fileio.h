/* vim: expandtab sw=4 sts=4 ts=8
 **********************************************************
 * fileio.h
 *
 * Copyright 2004, Stefan Siegl <ssiegl@gmx.de>, Germany
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Publice License,
 * version 2 or any later. The license is contained in the COPYING
 * file that comes with the wsdebug distribution.
 *
 * wsdebug file input/output
 */

#ifndef _FILEIO_H
#define _FILEIO_H

#include <stdio.h>

#ifdef __USE_POSIX
#include <unistd.h>
/* functions to write out wsdata to unix filedescriptors (and load) */
int parse_file(const int fd);
int write_file_fd(const int fd);
#endif

/* functions to write out wsdata directly into files (and load) */
int load_file(const char *fname);
int write_file(const char *fname);

#endif



/***** -*- emacs is great -*-
Local Variables:
mode: C
c-basic-offset: 4
indent-tabs-mode: nil
end: 
****************************/
