/* vim: expandtab sw=4 sts=4 ts=8
 **********************************************************
 * wsdebug.c
 *
 * Copyright 2004, Stefan Siegl <ssiegl@gmx.de>, Germany
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Publice License,
 * version 2 or any later. The license is contained in the COPYING
 * file that comes with the wsdebug distribution.
 *
 * whitespace debugger
 */

#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>

#include "fileio.h"
#include "interprt.h"
#include "debug.h"

#define VERSION "0.0.1"



int main(int argc, char **argv) 
{
    printf("WhiteSpace Debugger " VERSION "\n"
           "Copyright 2004, Stefan Siegl <ssiegl@gmx.de>, Germany\n"
           "WSDEBUG is free software, covered by the GNU General Public License, and you are\n"
           "welcome to change it and/or distribute copies of it under certain conditions.\n"
           "There is absolutely no warranty for WSDEBUG. See COPYING file for more info.\n");

    if(argc > 1) {
        if(argc > 2 || !strcmp(argv[1], "--help")) {
            printf("\n"
                   "Usage:\n"
                   "    %s [executable-file]\n"
                   "    %s --help\n"
                   "\n"
                   "Options:\n"
                   "    --help          Print this message.\n"
                   "\n", argv[0], argv[0]);
            return 1;
        }

        if(load_file(argv[1]))
            printf("%s: unable to open file\n", argv[1]);
        else
            printf("%s: file successfully loaded.\n", argv[1]);
    }
    
    debug_launch(); 
    return 0;
}



/***** -*- emacs is great -*-
Local Variables:
mode: C
c-basic-offset: 4
indent-tabs-mode: nil
end: 
****************************/
