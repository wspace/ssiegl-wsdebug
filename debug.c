/* vim: expandtab sw=4 sts=4 ts=8
 **********************************************************
 * debug.c
 *
 * Copyright 2004, Stefan Siegl <ssiegl@gmx.de>, Germany
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Publice License,
 * version 2 or any later. The license is contained in the COPYING
 * file that comes with the wsdebug distribution.
 *
 * whitespace debugger (console based)
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "fileio.h"
#include "interprt.h"



/* add cruft, related to libreadline and it's history thingy
 */
#ifdef HAVE_LIBREADLINE
#  if defined(HAVE_READLINE_READLINE_H)
#    include <readline/readline.h>
#  elif defined(HAVE_READLINE_H)
#    include <readline.h>
#  else
extern char *readline ();
#  endif
#endif

#ifdef HAVE_READLINE_HISTORY
#  if defined(HAVE_READLINE_HISTORY_H)
#    include <readline/history.h>
#  elif defined(HAVE_HISTORY_H)
#    include <history.h>
#  else
extern void add_history ();
extern int write_history ();
extern int read_history ();
#  endif
#endif



static int debug_eval(char *cmd_line);
static unsigned int debug_search_line_begin(unsigned int address);

unsigned char breakpoint = 0xCF;
#define debug_set_breakpoint(pos) \
    do { \
        if(wsdata[pos] == 0xCF) \
            printf("There already is a breakpoint at 0x%04x.\n", pos); \
        else { \
            printf("Breakpoint set at 0x%04x.\n", pos); \
            wsdata_merge_into(pos,&breakpoint,1); \
            label_cache_update(pos,2); \
        } \
    } while(0)




void debug_launch(void) 
{
    /* now start to read and evaluate commands */
    for(;;) {
#ifdef HAVE_LIBREADLINE
        char *buf;

        buf = readline("\n(wsdebug) ");
        if(! buf) break; /* we're at eof, get outta here */

#ifdef HAVE_READLINE_HISTORY
        if(*buf) add_history(buf);
#endif

#else
        /* we do not have readline library available, work around, using
         * standard fgets
         */
        char buf[256];

        /* write out wsdebug prompot */
        printf("\n(wsdebug) ");
        if(! fgets(buf, sizeof(buf), stdin)) break;
#endif

        if(debug_eval(buf))
            break;

#ifdef HAVE_LIBREADLINE
        /* get rid of memory allocated by readline() */
        free(buf);
#endif
    }

    putchar('\n');
    return;
}




/* int debug_eval(char *cmd_line)
 *
 * interpret the given wsdebug commandline.
 *
 * RETURN: 1 if debugger exit requested
 */
static int debug_eval(char *cmd_line) 
{
    char *command = NULL;
    char *argument = NULL;

    /* okay, first we've got to split up the command line */
    for(; *cmd_line != 0; cmd_line ++)
        if(isalpha(*cmd_line)) {
            command = cmd_line;
            break;
        }

    if(! command) return 0; /* no command specified */

    for(;; cmd_line ++)
        if(! isalpha(*cmd_line)) {
            /* okay, got end of the command */
            if(*cmd_line != 0) argument = cmd_line + 1;
            *cmd_line = 0; /* terminate */
            break;
        }

    if(argument) {
        /* okay, we've got an argument, try to trim whitespace etc. */
        for(; *argument != 0; argument ++)
            if(! isspace(*argument))
                break;
                
        for(cmd_line = argument + strlen(argument) - 1;;cmd_line --)
            if(isspace(*cmd_line))
                *cmd_line = 0;
            else
                break;

        if(!*argument) argument = NULL;
    }
        
    /* fprintf(stderr, "WSDBG: command='%s', argument='%s'\n", command, argument); */

    /* okay, now try to evaluate the commands */
    if(! strcmp(command, "help")) {
        printf("list of wsdebug-commands:\n\n"
               "break -- set breakpoint at (or shortly before) address\n"
               "continue -- continue execution\n"
               "exit -- leave debugger\n"
               "file -- use FILE as whitespace program to be debugged\n"
               "help -- display this screen\n"
               "kill -- kill execution of program being debugged\n"
               "list -- list lines around specified address\n"
               "next -- execute a whole instruction\n"
               "run -- start debugged program\n"
               "step -- execute exactly one whitespace instruction\n"
               "\n");
    }
    else if(!strcmp(command, "break")) {
        if(argument) {
            unsigned int value = 
                debug_search_line_begin(strtoul(argument, NULL, 0));

            if(value < wsdata_len)
                debug_set_breakpoint(value);
            else
                printf("cannot set breakpoint behind end of file.\n");
        }
        else
            printf("break takes an address argument, try specifying one.\n");
    }
    else if(!strcmp(command, "cont") || !strcmp(command, "continue")) {
        if(interprt_running)
            interprt_err_handler(stdout, interprt_cont());
        else 
            printf("The program is not being run, try 'run'.\n");
    }
    else if(!strcmp(command, "exit"))
        return 1;

    else if(!strcmp(command, "file")) {
        if(argument) {
            if(load_file(argument))
                printf("%s: unable to open file\n", argument);
            else
                printf("%s: file successfully loaded.\n", argument);
        }
        else
            printf("file takes a filename argument, try specifying one.\n");
    }
    else if(!strcmp(command, "kill")) {
        interprt_reset(); 
    }
    else if(!strcmp(command, "list")) {
        if(argument) {
            unsigned long int value = strtoul(argument, NULL, 0);
            interprt_output_list(stdout, 
                                 &wsdata[debug_search_line_begin(value)], 10); 
        }
        else
            printf("list takes an address argument, try specifying one.\n");
    }
    else if(!strcmp(command, "next")) {
        if(interprt_running)
            interprt_err_handler(stdout, interprt_next());
        else 
            printf("The program is not being run, try 'run'.\n");
    }
    else if(!strcmp(command, "run")) {
        interprt_init();
        interprt_err_handler(stdout, interprt_cont());
    }
    else if(!strcmp(command, "step")) {
        if(interprt_running)
            interprt_err_handler(stdout, interprt_step());
        else 
            printf("The program is not being run, try 'run'.\n");
    }
    else {
        printf("Undefined command: \"%s\".  Try \"help\".\n", command);
    }

    return 0;
}




/* unsigned int debug_search_line_begin(int address)
 *
 * map the specified address (= offset in wsdata) to the beginning of the
 * pointed line. e.g. if a ws-command starts at 0xC4 and last 17 chars,
 * map addr. 0xC9 back to 0xC4.
 *
 * RETURN: pointer to wsdata
 */
static unsigned int debug_search_line_begin(unsigned int address) 
{
    if(address > wsdata_len) address = wsdata_len ? (wsdata_len - 1) : 0;
    if(address < 1) return 0; 

    for(; address > 0 && wsdata[address - 1] != 0; address --);
    return address;
}



/***** -*- emacs is great -*-
Local Variables:
mode: C
c-basic-offset: 4
indent-tabs-mode: nil
end: 
****************************/
