/* vim: expandtab sw=4 sts=4 ts=8
 **********************************************************
 * storage.h
 *
 * Copyright 2004, Stefan Siegl <ssiegl@gmx.de>, Germany
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Publice License,
 * version 2 or any later. The license is contained in the COPYING
 * file that comes with the wsdebug distribution.
 *
 * wsdebug data storage (in memory representation)
 */

#ifndef _STORAGE_H
#define _STORAGE_H

#include <malloc.h>
/* #include <memory.h> */
#include <assert.h>


/* okay, generic stack handling macros first **********************************/

/* these macros are thought for internal (so storage.h) use only.
 * they provide low-level support for defining and working with
 * stacks.
 *
 * use these to create your own stacks, in wsdebug for example
 * the wsdata and compose stacks, see below
 */
#define STACK_DEF_EXT(elm,st,st_len,st_alloc) \
    extern elm *st; \
    extern unsigned int st_len; \
    extern unsigned int st_alloc;
	
#define STACK_DEF(elm,st,st_len,st_alloc) \
    elm *st = NULL; \
    unsigned int st_len = 0; \
    unsigned int st_alloc = 0;
	
#define STACK_REQUIRE(st,st_len,st_alloc,elements) \
    while((st_len) + (elements) > (st_alloc)) \
        st = realloc(st, ((st_alloc) = (st_alloc) ? (st_alloc) << 1 : 512) * sizeof((st)[0]))

#define STACK_ALLOCATE(st,st_alloc,request) \
    if((request) >= (st_alloc)) \
        st = realloc(st, (st_alloc = (request) + 1) * sizeof((st)[0]))

#define STACK_PUSH(st,st_len,value) \
    st[(st_len) ++] = (value);
	
#define STACK_POP(st,st_len) \
    (assert((st_len) > 0), st[-- (st_len)])

#define STACK_DELETE(st,st_len,first,len) \
    do { \
        memmove(&st[first], &st[(first) + (len)], ((st_len) - (first) - (len)) * sizeof(st[0])); \
        st_len -= len; \
    } while(0)




/* wsdata stack ***************************************************************/
STACK_DEF_EXT(unsigned char, wsdata, wsdata_len, wsdata_alloc)
#define wsdata_require(bytes) STACK_REQUIRE(wsdata, wsdata_len, wsdata_alloc, bytes)
#define wsdata_delete(start,len) STACK_DELETE(wsdata, wsdata_len, start, len)
#define wsdata_reset() (wsdata_len = 0)
#define wsdata_merge_into(pos,src,src_len) \
    do { \
        wsdata_require(src_len + 1); \
        if((pos) < wsdata_len) \
            memmove(&wsdata[pos + src_len + 1], &wsdata[pos], wsdata_len - pos); \
        memcpy(&wsdata[pos], src, src_len); \
        wsdata[(pos) + src_len] = 0; /* null byte, to tell command is over here */ \
        wsdata_len += src_len + 1; \
    } while(0)
 
/* wsdata stack, stack to hold the whitespace data of the currently coded 
 * programm in memory.
 *
 * in order to append or insert an new instruction, use the compose stack
 * as described below
 *
 * use wsdata_delete(s,l) to delete some data from the stack. therefore specify
 * the first byte you'd like to delete and how many bytes to delete.
 *
 * use wsdata_reset() if you'd like to get rid of the whole stack, aka start
 * a new file.
 *
 * use wsdata_merge_into(p,s,l) if you'd like to put a string(s) of len(l)
 * right into the wsdata array (at pos 'p'). it takes care of moving the
 * data behind automatically.
 */




/* compose stack **************************************************************/
STACK_DEF_EXT(unsigned char, compose, compose_len, compose_alloc)
#define compose_require(bytes) STACK_REQUIRE(compose, compose_len, compose_alloc, bytes)
#define compose_push(byte) STACK_PUSH(compose, compose_len, byte)
#define compose_reset() (compose_len = 0)

#define compose_merge_into_wsdata(pos) \
    do { \
        wsdata_merge_into(pos,compose,compose_len); \
        compose_len = 0; /* clear compose buffer */ \
    } while(0)

#define compose_append_to_wsdata() \
    compose_merge_into_wsdata(wsdata_len)

/* compose stack, the stack thought to be used to put together a new
 * whitespace instruction.
 *
 * in order to do so, do the following:
 *   1. compose_reset();
 *   2. compose_require(16);  -- if you think, you'll need 16 bytes
 *   3. compose_push('\t');   -- to push a [tab] to the stack
 *   4. compose_push('\n');   -- to push a [lf] to the stack
 *   5. compose_append_to_wsdata();  -- to put the cmd at the end of wsdata
 *
 * if you want to insert the compsed command into wsdata stack, use
 * compose_merge_into_wsdata(p) and specify where to put the information.
 */

#endif




/***** -*- emacs is great -*-
Local Variables:
mode: C
c-basic-offset: 4
indent-tabs-mode: nil
end: 
****************************/
