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

#ifdef HAVE_CONFIG_H
#  ifndef HAVE_LIBGMP
#    include <config.h>
#  endif
#endif

#include "storage.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

#ifdef HAVE_LIBGMP
/* we have support for GNU's MP library aboard, then let's make use
 * of it. The original whitespace interpreter written in Haskell
 * makes use of it as well
 */
#  include <gmp.h>
#  define WSVAR_TYPE mpz_t

/* memory management */
#  define WSVAR_INIT(v) mpz_init(v)
#  define WSVAR_INIT_DECL(s,a) unsigned int i = (a)
#  define WSVAR_INIT_EXEC(s,a) for(;i < (a); i++) WSVAR_INIT((s)[i])
#  define WSVAR_CLEAR(v) mpz_clear(v);
#  define WSVAR_CLEAR_STACK(s,a) \
    do { \
        unsigned int i; \
        for(i = 0; i < (a); i++) \
            WSVAR_CLEAR((s)[i]); \
    } while(0)
#  define WSVAR_GET_UI(v) mpz_get_ui(v)
#  define WSVAR_PRINTF(v) gmp_printf("%Zd", (v))
#  define WSVAR_SET_SI(dest,v) mpz_set_si((dest),(v))
#  define WSVAR_INPUT(dest) mpz_inp_str((dest),stdin,0)
#  define WSVAR_CMP_ZERO(v) mpz_cmp_si((v), 0)

#  define WSVAR_DUMP_(h,v) \
    if(mpz_cmp_si((v), ' ') >= 0 && mpz_cmp_si((v), 'z') <= 0) \
        printf(" '%c' ", (int)mpz_get_ui(v) & 0xFF); \
    else \
        gmp_printf(h"%Z04x ", (v));

/* arithmetic stuff */
#  define WSVAR_ASSIGN(dest,src) mpz_set((dest), (src))
#  define WSVAR_ADD(dest,s1,s2) mpz_add((dest), (s1), (s2))
#  define WSVAR_ADD_UI(dest,s1,s2) mpz_add_ui((dest), (s1), (s2))
#  define WSVAR_SUB(dest,s1,s2) mpz_sub((dest), (s1), (s2))
#  define WSVAR_MUL(dest,s1,s2) mpz_mul((dest), (s1), (s2))
#  define WSVAR_MUL_UI(dest,s1,s2) mpz_mul_ui((dest), (s1), (s2))
#  define WSVAR_DIV(dest,s1,s2) mpz_tdiv_q((dest), (s1), (s2))
#  define WSVAR_MOD(dest,s1,s2) mpz_tdiv_r((dest), (s1), (s2))
#else

/* don't have GNU's MP library available, so let's use signed int */
#  define WSVAR_TYPE signed int

/* memory management */
#  define WSVAR_INIT(v)
#  define WSVAR_INIT_DECL(s,a)
#  define WSVAR_INIT_EXEC(s,a)
#  define WSVAR_CLEAR(v)
#  define WSVAR_CLEAR_STACK(s,a)
#  define WSVAR_GET_UI(v) ((unsigned int) v)
#  define WSVAR_PRINTF(v) printf("%d", (v))
#  define WSVAR_SET_SI(dest,v) (dest) = (v)
#  define WSVAR_INPUT(dest) scanf("%d", &(dest))
#  define WSVAR_CMP_ZERO(v) (v)

#  define WSVAR_DUMP_(h,v) \
    if((v) >= ' ' && (v) <= 'z') \
        printf(" '%c' ", (v)); \
    else \
        printf(h"%4x ", (v));

/* arithmetic stuff */
#  define WSVAR_ASSIGN(dest,src) dest = src
#  define WSVAR_ADD(dest,s1,s2) (dest) = (s1) + (s2)
#  define WSVAR_ADD_UI(dest,s1,s2) (dest) = (s1) + (s2)
#  define WSVAR_SUB(dest,s1,s2) (dest) = (s1) - (s2)
#  define WSVAR_MUL(dest,s1,s2) (dest) = (s1) * (s2)
#  define WSVAR_MUL_UI(dest,s1,s2) (dest) = (s1) * (s2)
#  define WSVAR_DIV(dest,s1,s2) (dest) = (s1) / (s2)
#  define WSVAR_MOD(dest,s1,s2) (dest) = (s1) % (s2)
#endif




/* abstraction layer, wrapping exec_stack and exec_heap!! */
#define WSVAR_STACK_REQUIRE(s,l,a,r) \
    do { \
        WSVAR_INIT_DECL(s,a); \
        STACK_REQUIRE(s,l,a,r); \
        WSVAR_INIT_EXEC(s,a); \
    } while(0)

#define WSVAR_STACK_ALLOCATE(s,l,a,r) \
    do { \
        WSVAR_INIT_DECL(s,a); \
        STACK_ALLOCATE(s,a,r); \
        WSVAR_INIT_EXEC(s,a); \
    } while(0)

#define WSVAR_STACK_RESET(s,l,a) \
    do { \
        WSVAR_CLEAR_STACK(s,a); \
        (l) = 0; \
    } while(0)

#define WSVAR_STACK_PUSH(s,l,a,v) \
    do { \
        WSVAR_STACK_REQUIRE(s,l,a,1); \
        WSVAR_ASSIGN((s)[(l) ++], (v)); \
    } while(0)

#define WSVAR_STACK_GET(s,l,d) WSVAR_ASSIGN((d), (s)[(l) - 1])
#define WSVAR_STACK_POP(s,l,d) WSVAR_ASSIGN((d), (s)[-- (l)])

#define WSVAR_STACK_WRITE(s,a,v) WSVAR_ASSIGN((s)[a], v)
#define WSVAR_STACK_READ(s,a,d)  WSVAR_ASSIGN(d, (s)[a])



#  define WSVAR_DUMP_HEXPREFIX(v) WSVAR_DUMP_("0x",v)
#  define WSVAR_DUMP(v) WSVAR_DUMP_(,v)





/* exec_stack stack ***********************************************************/
STACK_DEF_EXT(WSVAR_TYPE, exec_stack, exec_stack_len, exec_stack_alloc)
#define exec_stack_reset()      WSVAR_STACK_RESET(exec_stack,exec_stack_len,exec_stack_alloc)
#define exec_stack_require(r)   WSVAR_STACK_REQUIRE(exec_stack,exec_stack_len,exec_stack_alloc,(r))
#define exec_stack_push(v)      WSVAR_STACK_PUSH(exec_stack,exec_stack_len,exec_stack_alloc,(v))
#define exec_stack_pop(d)       WSVAR_STACK_POP(exec_stack,exec_stack_len,(d))
#define exec_stack_get(d)       WSVAR_STACK_GET(exec_stack,exec_stack_len,(d))
    
/* this is the stack, where the stack operations of the whitespace programming
 * language are performed.
 */




/* exec_heap stack ************************************************************/
STACK_DEF_EXT(WSVAR_TYPE, exec_heap, exec_heap_len, exec_heap_alloc)
#define exec_heap_reset()       WSVAR_STACK_RESET(exec_heap,exec_heap_len,exec_heap_alloc)
#define exec_heap_allocate(a)   WSVAR_STACK_ALLOCATE(exec_heap,exec_heap_len,exec_heap_alloc,a)
#define exec_heap_write(a,v)    WSVAR_STACK_WRITE(exec_heap, a, v)
#define exec_heap_read(a,d)     WSVAR_STACK_READ(exec_heap, a, d)

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



/* interpreter toggles, changing behaviour ************************************/
extern struct toggle_t {
    char *name;
    char *help;
    unsigned state :1;
} toggles[];

enum {
    TOGGLE_NOCANON,
    TOGGLE_LAST,
};




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
