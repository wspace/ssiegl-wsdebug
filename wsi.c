/* vim: expandtab sw=4 sts=4 ts=8
 **********************************************************
 * wsi.c
 *
 * Copyright 2004, Stefan Siegl <ssiegl@gmx.de>, Germany
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Publice License,
 * version 2 or any later. The license is contained in the COPYING
 * file that comes with the wsdebug distribution.
 *
 * whitespace interpreter
 */

#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>

#include "fileio.h"
#include "interprt.h"

#define VERSION "0.0.1"




int main(int argc, char **argv) 
{
    if(argc != 2) {
        printf("WhiteSpace Interpreter " VERSION "\n"
               "Copyright 2004, Stefan Siegl <ssiegl@gmx.de>, Germany\n"
               "WSI is free software, covered by the GNU General Public License, any you are\n"
               "welcome to change it and/or distribute copies of it under certain conditions.\n"
               "There is absolutely no warranty for WSDBG. See COPYING file for more info.\n"
               "\n"
               "Usage:\n"
               "    %s [executable-file]\n"
               "    %s --help\n"
               "\n"
               "Options:\n"
               "    --help          Print this message.\n"
               "\n", argv[0], argv[0]);
        return 2;
    }

    if(load_file(argv[1])) {
        fprintf(stderr, "%s: unable to load file.\n", argv[1]); 
        return 2;
    }

    interprt_init();
    return interprt_err_handler(stderr, interprt_cont()) != DO_EXIT;
}



/***** -*- emacs is great -*-
Local Variables:
mode: C
c-basic-offset: 4
indent-tabs-mode: nil
end: 
****************************/
