/* vim: expandtab sw=4 sts=4 ts=8 cin
 **********************************************************
 * fileio.c
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

#include <string.h>

#include "fileio.h"
#include "storage.h"

#ifdef __USE_POSIX
#  include <fcntl.h>
#  include <sys/stat.h>

   /* permissions to assign to new files */
#  define PERM (S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR)
#endif

/* check whether char (a) is a whitespace command character */
#define iswschar(a) (((a) == '\t') || ((a) == '\n') || ((a) == ' '))

static int compose_cmd_is_complete(void);

#ifdef __USE_POSIX
/* int parse_file(const int fd)
 *
 * parse the file's content into the wsdata stack (unix-like systems only)
 *
 * RETURN: -1 on failure.
 */
int parse_file(const int fd) 
{
    unsigned char buf[512];
    int len, pos;
    
    if(fd < 0) return -1; /* file descriptor not valid */

    wsdata_reset();
    compose_reset(); 

    while((len = read(fd, buf, sizeof(buf))) > 0) {
        for(pos = 0; pos < len; pos ++) {
            if(! iswschar(buf[pos])) continue; /* ignore non-ws characters */

            compose_require(1);
            compose_push(buf[pos]);
        
            if(compose_cmd_is_complete())
                compose_append_to_wsdata();
        }
    }

    return -(len<0); /* return -1 if read() failed, 0 otherwise */
}
#endif



/* int load_file(const char *fname)
 *
 * read the file's content into the wsdata stack.
 *
 * RETURN: -1 on failure.
 */
int load_file(const char *fname)
{
#ifdef __USE_POSIX
    int status;
    
    int fd = open(fname, O_RDONLY);
    if(fd < 0) return -1;

    /* call our parse_file function, reading data from unix filedescriptor */
    status = parse_file(fd);
    return close(fd) < 0 ? -1 : status;

#else 

    unsigned char buf[512];
    int len, pos;
    
    FILE *hdl = fopen(fname, "rb");
    if(! hdl) return -1;
    
    wsdata_reset();
    compose_reset(); 

    while((len = fread(buf, 1, sizeof(buf), hdl)) > 0) {
        for(pos = 0; pos < len; pos ++) {
            if(! iswschar(buf[pos])) continue; /* ignore non-ws characters */

            compose_require(1);
            compose_push(buf[pos]);
        
            if(compose_cmd_is_complete())
                compose_append_to_wsdata();
        }
    }

    fclose(hdl);
    return -(len<0); /* return -1 if fread() failed, 0 otherwise */
#endif
}


/* int write_file(const char *fname)
 *
 * write whole wsdata stack to file (with provided name)
 *
 * RETURN: -1 on failure.
 */
int write_file(const char *fname)
{
#ifdef __USE_POSIX
    int status;

    int fd = open(fname, O_WRONLY | O_CREAT | O_TRUNC, PERM);
    if(fd < 0) return -1;

    status = write_file_fd(fd);
    return close(fd) < 0 ? -1 : status;

#else
    unsigned char *start, *stop;

    FILE *hdl = fopen(fname, "wb");
    if(! hdl) return -1;

    for(start = stop = wsdata; stop < wsdata + wsdata_len; stop ++)
        if(*stop == 0) {
            /* jupp, reached end of command, write it out */
            size_t len = stop - start;
            if(fwrite(start, 1, len, hdl) != len)
                return -1; /* something terrible happend */
            
            start = stop + 1;
        }

    fclose(hdl);
    return 0;

#endif
}



#ifdef __USE_POSIX
/* int write_file_fd(const int fd)
 * 
 * write whole wsdata stack to the given filedescriptor
 *
 * RETURN: -1 on failure.
 */
int write_file_fd(const int fd) 
{
    unsigned char *start, *stop;

    if(fd<0) return -1;

    for(start = stop = wsdata; stop < wsdata + wsdata_len; stop ++)
        if(*stop == 0) {
            /* jupp, reached end of command, write it out */
            int len = stop - start;
            if(write(fd, start, len) != len)
                return -1; /* something terrible happend */
            
            start = stop + 1;
        }

    return 0;
}
#endif



/* int compose_cmd_is_complete(void)
 *
 * check whether we've got a complete command in compose buffer, or whether
 * there are still some chars missing.
 *
 * RETURN: 1 if command is complete, 0 otherwise
 */
static int compose_cmd_is_complete(void) {
    if(compose_len < 3) return 0; /* every command has at least 3 chars */
    
    switch(compose[0]) {
        case ' ': /* stack manipulation */
            switch(compose[1]) {
                case ' ': 
                    /* push to stack
                     * we need to have a number, which has at least 1 byte,
                     * namely sign, there need to be no digits present ...
                     */
                    return (compose_len > 3 && compose[compose_len - 1] == '\n') ? 1 : 0;
                    
                case '\n':
                    /* all these commands don't have parameters, so if we got
                     * three chars, it's enough ...
                     */
                    return compose_len == 3 ? 1 : 0;

                case '\t':
                    /* 1x imp, 2x command + at least two for number */
                    return (compose_len > 5 && compose[compose_len - 1] == '\n') ? 1 : 0;
            }
            break;

        case '\t':
            switch(compose[1]) {
                case ' ': 
                    /* arithmetic, these commands do not take a parameter
                     * therefore, if we've got 2 + 2 chars, it's okay
                     */
                    return compose_len == 4 ? 1 : 0;

                case '\t': 
                    /* heap access, no parameters as well, but only one
                     * command byte after 2 imp bytes
                     */
                    return compose_len == 3 ? 1 : 0;

                case '\n': /* I/O */
                    /* all i/o commands are 4 chars long (2x imp, 2x cmd)
                     */
                    return compose_len == 4 ? 1 : 0;
            }
            break;

        case '\n': /* flow control */
            switch(compose[1]) {
                case ' ':
                    /* mark as label, call subroutine, uncond. jump,
                     * all commands take a label as arg. so we've got
                     * 1x imp, 2x command + label
                     */
                    return (compose_len > 3 && compose[compose_len - 1] == '\n') ? 1 : 0;

                case '\t':
                    if(compose[2] == '\n')
                        return 1; /* end of subroutine */

                    /* conditional jumps, we require at least 1 char for the
                     * imp, two for the command and one for the label 
                     */
                    return (compose_len > 4 && compose[compose_len - 1] == '\n') ? 1 : 0;

                case '\n':
                    /* command \n\n\n is end of program, no others with \n\n */
                    return compose_len == 3 ? 1 : 0;
            }
            break;
    }

    /* FIXME this shouldn't be reached, return 1 anyhow */
    return 1;
}



/***** -*- emacs is great -*-
Local Variables:
mode: C
c-basic-offset: 4
indent-tabs-mode: nil
end: 
****************************/
