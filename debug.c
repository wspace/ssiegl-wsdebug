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


static int debug_exec_break(const char *arg);
static int debug_exec_continue(const char *arg);
static int debug_exec_exit(const char *arg);
static int debug_exec_file(const char *arg);
static int debug_exec_help(const char *arg);
static int debug_exec_kill(const char *arg);
static int debug_exec_list(const char *arg);
static int debug_exec_next(const char *arg);
static int debug_exec_run(const char *arg); 
static int debug_exec_step(const char *arg);

typedef int (* debug_exec_func)(const char *arg);

struct {
    char *name;
    char *helptext;
    debug_exec_func func;
    unsigned int takes_arg:1;
    unsigned int need_running_prog:1;
} debug_commands[] = {
    { "break", "set breakpoint at (or shortly before) address", debug_exec_break, 1, 0 },
    { "continue", "continue execution", debug_exec_continue, 0, 1 },
    { "cont", NULL, debug_exec_continue, 0, 1 },
    { "exit", "leave debugger", debug_exec_exit, 0, 0 },
    { "file", "use FILE as whitespace program to be debugged", debug_exec_file, 1, 0 },
    { "help", "display this screen", debug_exec_help, 0, 0 },
    { "kill", "kill execution of program being debugged", debug_exec_kill, 0, 1 },
    { "list", "list lines around specified address", debug_exec_list, 1, 0 },
    { "next", "execute a whole instruction", debug_exec_next, 0, 1 },
    { "run", "start debugged program", debug_exec_run, 0, 0 },
    { "step", "execute exactly one whitespace instruction", debug_exec_step, 0, 1 },
    { "quit", "leave, just like exit.", debug_exec_exit, 0, 0 },
};




void debug_launch(void) 
{
    /* now start to read and evaluate commands */
    for(;;) {
#ifdef HAVE_LIBREADLINE
        char *buf;
        static char *last = NULL;

        buf = readline("\n(wsdebug) ");
        if(! buf) break; /* we're at eof, get outta here */

#ifdef HAVE_READLINE_HISTORY
        if(*buf && (!last || !strcmp(last, buf))) add_history(buf);
#endif
        if(!*buf) {
            /* okay, no arg supplied, just execute last command once more */
            free(buf);
            buf = last;
        }

#else
        /* we do not have readline library available, work around, using
         * standard fgets
         */
        char buf[256];

        /* write out wsdebug prompt */
        printf("\n(wsdebug) ");
        if(! fgets(buf, sizeof(buf), stdin)) break;
#endif

        if(debug_eval(buf))
            break;

#ifdef HAVE_LIBREADLINE
        /* get rid of memory allocated by readline() */
        if(last != buf && last) free(last);

        /* store the last command, the user's entered */
        last = buf;
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
    int i;
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

    for(i = 0; i < sizeof(debug_commands) / sizeof(debug_commands[0]); i ++) {
        int p;
        for(p = 0; ; p++) {
            if(debug_commands[i].name[p] != command[p]) break;
            if(debug_commands[i].name[p] == 0) {
                /* okay, found correct command structure */
                if(debug_commands[i].takes_arg && !argument) {
                    printf("%s takes an argument, however you "
                           "didn't supply one.\n", debug_commands[i].name);
                    return 0; 
                }

                if(debug_commands[i].need_running_prog && !interprt_running) {
                    printf("The program is not being run, try 'run'.\n");
                    return 0;
                }

                return debug_commands[i].func(argument);
            }
        }
    }

    printf("Undefined command: \"%s\".  Try \"help\".\n", command);
    return 0;
}



static int debug_exec_help(const char *arg) 
{
    int i;

    printf("list of wsdebug-commands:\n\n");

    for(i = 0; i < sizeof(debug_commands) / sizeof(*debug_commands); i ++)
        if(debug_commands[i].helptext)
            printf("%s -- %s\n", debug_commands[i].name,
                   debug_commands[i].helptext);

    return 0; /* don't exit program */
}



static int debug_exec_break(const char *arg)
{
    unsigned int value = debug_search_line_begin(strtoul(arg, NULL, 0));

    if(value < wsdata_len)
        debug_set_breakpoint(value);
    else
        printf("cannot set breakpoint behind end of file.\n");
    
    return 0; /* continue executing wsdebug */
}



static int debug_exec_continue(const char *arg)
{
    interprt_err_handler(stdout, interprt_cont());
    return 0; /* don't stop wsdebug here */
}



static int debug_exec_exit(const char *arg)
{
    return 1; /* exit wsdebug */
}



static int debug_exec_file(const char *argument)
{
    if(load_file(argument))
        printf("%s: unable to open file\n", argument);
    else
        printf("%s: file successfully loaded.\n", argument);

    return 0; /* request not to leave */
}



static int debug_exec_kill(const char *arg) 
{
    interprt_reset(); 
    return 0;
}



static int debug_exec_list(const char *arg)
{
    unsigned long int value = strtoul(arg, NULL, 0);
    interprt_output_list(stdout, &wsdata[debug_search_line_begin(value)], 10); 

    return 0; /* don't exit yet */
}



static int debug_exec_next(const char *arg)
{
    interprt_err_handler(stdout, interprt_next());
    return 0; /* continue executing wsdebug */
}



static int debug_exec_run(const char *arg)
{
    interprt_init();
    interprt_err_handler(stdout, interprt_cont());
    return 0;
}



static int debug_exec_step(const char *arg)
{
    interprt_err_handler(stdout, interprt_step());
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
