/* vim: expandtab sw=4 sts=4 ts=8
 **********************************************************
 * interprt.h
 *
 * Copyright 2004, Stefan Siegl <ssiegl@gmx.de>, Germany
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Publice License,
 * version 2 or any later. The license is contained in the COPYING
 * file that comes with the wsdebug distribution.
 *
 * wsdebug's whitespace interpreter
 */

#ifndef _INTERPRT_H
#define _INTERPRT_H

#include "storage.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>


/* exec_stack stack ***********************************************************/
STACK_DEF_EXT(signed int, exec_stack, exec_stack_len, exec_stack_alloc)
#define exec_stack_reset()      exec_stack_len = 0;
#define exec_stack_require(r)   STACK_REQUIRE(exec_stack,exec_stack_len,exec_stack_alloc,r)
#define exec_stack_push(v)      do{exec_stack_require(1); STACK_PUSH(exec_stack,exec_stack_len,v)}while(0)
#define exec_stack_pop()        (exec_stack[-- exec_stack_len])
#define exec_stack_get()        (exec_stack[exec_stack_len - 1])
    
/* this is the stack, where the stack operations of the whitespace programming
 * language are performed.
 */




/* exec_heap stack ************************************************************/
STACK_DEF_EXT(signed int, exec_heap, exec_heap_len, exec_heap_alloc)
#define exec_heap_reset()       exec_heap_len = 0;
#define exec_heap_allocate(a)   STACK_ALLOCATE(exec_heap,exec_heap_alloc,a);
#define exec_heap_write(a,v)    (exec_heap[a] = (v))
#define exec_heap_read(a)       (exec_heap[a])

/* all heap access operations use are performed in this piece of memory 
 *
 * BE CAREFUL, exec_heap yet is only able to serve _positive_ heap addresses.
 * negative addresses will fail and abort the program!
 * 
 * make sure to exec_heap_allocate(a) before calling exec_heap_read(a)
 * or exec_heap_write(a,v)!!
 */




/* exec_backtrace stack *******************************************************/
STACK_DEF_EXT(unsigned int, exec_bt, exec_bt_len, exec_bt_alloc)
#define exec_bt_reset()     exec_bt_len = 0;
#define exec_bt_require(r)  STACK_REQUIRE(exec_bt,exec_bt_len,exec_bt_alloc,r)
#define exec_bt_push(ip)    do{exec_bt_require(1); STACK_PUSH(exec_bt,exec_bt_len,ip)}while(0)
#define exec_bt_pop()       STACK_POP(exec_bt,exec_bt_len)
#define exec_bt_get()       (exec_bt[exec_bt_len - 1])
#define exec_bt_replace(a)  (exec_bt[exec_bt_len - 1] = (a))

/* instruction pointer is pushed onto this stack, if programmer makes use of
 * the CALL operation (lf, space, tab)
 */




/* return value type of interpreter / debugger functions **********************/
typedef enum {
    DO_SYNTAX_ERROR,
    DO_EXIT,
    DO_OKAY,
    DO_OKAY_IP_MANIP, /* do-function manipulated BT stack, do not modify ip */
    DO_LABEL_NOT_FOUND,
    DO_END_NOT_EXPECTED,
    DO_REACHED_BREAKPOINT,
    DO_STACK_UNDERFLOW
} interprt_do_stat;




/* prototypes for interpreter / debugger couple *******************************/
void interprt_init(void);
void interprt_reset(void);

interprt_do_stat interprt_step(void);
interprt_do_stat interprt_next(void);
interprt_do_stat interprt_cont(void);
interprt_do_stat interprt_err_handler(FILE *target, interprt_do_stat status);
void interprt_output_list(FILE *target, const unsigned char *wsdata_ptr, int lines);

extern int interprt_running;

/* label_cache store's offsets into wsdata, therefore we need to update it,
 * if we e.g. add a breakpoint into it.
 */
void label_cache_update(unsigned int start, unsigned int offset);

#endif



/***** -*- emacs is great -*-
Local Variables:
mode: C
c-basic-offset: 4
indent-tabs-mode: nil
end: 
****************************/
