/* vim: expandtab sw=4 sts=4 ts=8
 **********************************************************
 * interprt.c
 *
 * Copyright 2004, Stefan Siegl <ssiegl@gmx.de>, Germany
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Publice License,
 * version 2 or any later. The license is contained in the COPYING
 * file that comes with the wsdebug distribution.
 *
 * wsdebug's whitespace interpreter and debugger
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "fileio.h"
#include "interprt.h"

/* gcc complains, that strchr is for signed chars actually, but unsigned 
 * ones shouldn't hurt -- especially since we hunt for '\0' here and there.
 */
#define u_strchr(a,b) ((unsigned char *)strchr((char *)a, (signed char) b))
#define u_strcmp(a,b) ((unsigned char *)strcmp((char *)a, (char *)b))



/* define interpreter stacks first */
STACK_DEF(signed int, exec_stack, exec_stack_len, exec_stack_alloc)
STACK_DEF(signed int, exec_heap, exec_heap_len, exec_heap_alloc)
STACK_DEF(unsigned int, exec_bt, exec_bt_len, exec_bt_alloc)

int interprt_running = 0;


/* prototypes of interprt_do_... worker functions */
static interprt_do_stat interprt_do_stack_manip(const unsigned char *ip);
static interprt_do_stat interprt_do_arithmetic(const unsigned char *ip);
static interprt_do_stat interprt_do_heap_access(const unsigned char *ip);
static interprt_do_stat interprt_do_io_command(const unsigned char *ip); 
static interprt_do_stat interprt_do_flow_control(const unsigned char *ip); 

static interprt_do_stat interprt_search_label(const unsigned char *label); 
static signed int interprt_number(const unsigned char *ptr);



/* when reading in chars (in interprt_do_io_command) try to reset terminal's
 * canonical flag so we can read character by character. otherwise we
 * would have to require the user to enter a whole line (terminated by \n);
 * this however is probably not what we want to have. 
 *
 * setting the terminal into canonical mode however requires POSIX commands,
 * where wsdebug tries to stick to the ISO C89 / ANSI standards ...
 */
#ifdef __USE_POSIX
#  include <termio.h>
#endif



/* in order to increase lookup-speed of labels we cache those, therefore
 * we scan the whole file for labels, if the first lookup is requested.
 *
 * Define SMALL_HASH, to request creation a hash with 16 buckets, else
 * 256 buckets would be reserved. Typically a small hash is enough, i.e.
 * if you don't intend to run really large (say programs >50 labels)
 * programs.
 */
#define SMALL_CACHE 1
/* #define DEBUG_CACHE_STATISTICS 1 */
/* define DEBUG_CACHE if you want some statistics to be printed, every
 * time the cache gets created/updated. This is particularly useful when
 * updating/refining the hashing function
 */

typedef struct label_cache_t_ label_cache_t;
struct label_cache_t_ {
    unsigned int ws_ptr;
    label_cache_t *next;
};

#ifdef SMALL_CACHE
label_cache_t *label_cache[16] = {0};
#else
label_cache_t *label_cache[256] = {0};
#endif

/* static variable, telling whether our cache is in sane state. we need
 * to mark it dirty if wsdata is changed (=> setting breakpoints)
 */
int label_cache_ready;

static int label_cache_hash_func(const unsigned char *);
static void label_cache_create(void);



/* void interprt_init(void)
 *
 * initialize (aka start or restart) whitespace interpreter
 */
void interprt_init(void)
{
    interprt_reset(); 

    /* set instruction pointer to beginning-of-file => IP=0 */
    exec_bt_push(0);
    interprt_running = 1;

    /* disable buffering of standard output */
    setvbuf(stdout, NULL, _IONBF, 0);

    /* label cache is setup on first jump/call, perhaps we got a program
     * without any jump request, well, even that's somewhat unlikely ..
     */
    label_cache_ready = 0;
}



/* void interprt_reset(void)
 *
 * reset (aka kill) whitespace interpreter
 */
void interprt_reset(void)
{
    /* reset stacks */
    exec_stack_reset();
    exec_heap_reset();
    exec_bt_reset(); 

    interprt_running = 0;
}



/* interprt_do_stat interprt_step(void)
 * 
 * execute exactly one instruction
 */
interprt_do_stat interprt_step(void)
{
    interprt_do_stat stat = DO_SYNTAX_ERROR;
    const unsigned char *ip = &wsdata[exec_bt_get()];

    if(ip >= wsdata + wsdata_len)
         /* we've reached end of programm, but there was no \n\n\n */
        return DO_END_NOT_EXPECTED;
   
    switch(ip[0]) {
        case ' ': stat = interprt_do_stack_manip(&ip[1]); break;
                  
        case '\t':
            switch(ip[1]) {
                case ' ': stat = interprt_do_arithmetic(&ip[2]); break;
                case '\t': stat = interprt_do_heap_access(&ip[2]); break;
                case '\n': stat = interprt_do_io_command(&ip[2]); break;
                default: return DO_SYNTAX_ERROR;
            }
            
            break;
            
        case '\n': stat = interprt_do_flow_control(&ip[1]); break;

        case 0xCF:
            exec_bt_replace((ip - wsdata) + 2); /* inc to beg of next insn */
            return DO_REACHED_BREAKPOINT;

        default:
            return DO_SYNTAX_ERROR;
    }

    switch(stat) {
        case DO_SYNTAX_ERROR:
        case DO_LABEL_NOT_FOUND:
        case DO_EXIT:
        case DO_END_NOT_EXPECTED:
        case DO_STACK_UNDERFLOW:
            interprt_running = 0;

        case DO_REACHED_BREAKPOINT: 
            return stat;

        case DO_OKAY:
            /* okay, called interprt_do_... didn't update the stack but
             * everything went perfectly well. 
             * therefore forward the instruction pointer to the next insn
             */
            ip = u_strchr(ip, '\0');
            assert(ip);
            exec_bt_replace((ip - wsdata) + 1); /* inc to beg of next line */
            break;

        case DO_OKAY_IP_MANIP:
            /* DO_OKAY_IP_MANIP is returned if the command either was an
             * unconditional jump, a call or an executed conditional jump.
             *
             * the interprt_do_flow_control() function already manipulated the
             * stack, so we don't have to!
             */
             break;
    }

    return DO_OKAY;
}



/* interprt_do_stat interprt_next(void)
 * 
 * execute to next instruction (on same stack level)
 */
interprt_do_stat interprt_next(void)
{
    interprt_do_stat stat;
    unsigned int stack_level = exec_bt_len;

    do
        if((stat = interprt_step()) != DO_OKAY) 
            return stat; /* error occured */
    while(exec_bt_len > stack_level);

    return stat;
}



/* interprt_do_stat interprt_cont(void)
 * 
 * continue executing instruction till error (or break point)
 */
interprt_do_stat interprt_cont(void)
{
    for(;;) {
        interprt_do_stat stat = interprt_step();
        if(stat != DO_OKAY) return stat; /* error occured */
    }
}




/* interprt_do_stack_manip
 *
 * interpret a stack manipulation command. where ip points to the first
 * command bit (right after imp)
 */
static interprt_do_stat interprt_do_stack_manip(const unsigned char *ip)
{
    switch(ip[0]) {
        case '\n':
            switch(ip[1]) {
                case ' ': /* duplicate top item */
                    {
                        unsigned int value;

                        if(! exec_stack_len) return DO_STACK_UNDERFLOW;

                        value = exec_stack_get(); 
                        exec_stack_push(value);
                        return DO_OKAY;
                    }
                    
                case '\t': /* swap top two items */
                    {
                        unsigned int first, second;
                        
                        if(exec_stack_len < 2) return DO_STACK_UNDERFLOW;

                        first = exec_stack_pop();
                        second = exec_stack_pop();

                        /* okay, now push first element (former top) first */
                        exec_stack_push(first);
                        exec_stack_push(second);
                        return DO_OKAY;
                    }
                    
                case '\n': /* discard item at the top of the stack */
                    exec_stack_len --;
                    return DO_OKAY;
                
                default: return DO_SYNTAX_ERROR;
            }

        case '\t':
            switch(ip[1]) {
                case ' ':
                    if(ip[2] == '\n') return DO_SYNTAX_ERROR; 
                    {
                        int value;
                        int stack_pos = interprt_number(&ip[2]);
                        if(stack_pos < 0) return DO_SYNTAX_ERROR;
                        
                        if((unsigned int)stack_pos >= exec_stack_len) 
                            return DO_STACK_UNDERFLOW;

                        value = exec_stack[exec_stack_len - 1 - stack_pos];
                        exec_stack_push(value);
                        return DO_OKAY;
                    }
                    
                case '\n':
                    if(ip[2] == '\n') return DO_SYNTAX_ERROR; 
                    {
                        int stack_save;
                        int discard_items = interprt_number(&ip[2]);
                        if(discard_items < 0) return DO_SYNTAX_ERROR;

                        if((unsigned int)discard_items + 1 >= exec_stack_len)
                            return DO_STACK_UNDERFLOW;

                        stack_save = exec_stack_pop();
                        exec_stack_len -= (unsigned int)discard_items;
                        exec_stack_push(stack_save);
                        return DO_OKAY;
                    }
                    
                case '\t':
                default:
                    return DO_SYNTAX_ERROR;
            }

        case ' ':
            /* sign must be either \t or ' '! */
            if(ip[1] == '\n') return DO_SYNTAX_ERROR; 
            exec_stack_push(interprt_number(&ip[1]));
            return DO_OKAY;
    }

    return DO_SYNTAX_ERROR;
}




/* signed int interprt_number(const unsigned char *ptr)
 *
 * reinterpret ws-style-number to common signed int value
 */
static signed int interprt_number(const unsigned char *ptr) {
    unsigned int value = 0;
    const unsigned char *sign = ptr ++;

    if(*ptr != '\n') do {
        value <<= 1;
        if(*ptr == '\t') value |= 1;
    } while(*(++ ptr) != '\n');

    return *sign == ' ' ? value : -value;
}




/* interprt_do_arithmetic
 *
 * interpret an arithmetic command. where ip points to the first
 * command bit (right after imp)
 */
static interprt_do_stat interprt_do_arithmetic(const unsigned char *ip)
{
    unsigned int result = 0;

    /* the first _pushed_ argument is the first argument of the operation! */
    unsigned int first, second;
    
    if(exec_stack_len < 2) return DO_STACK_UNDERFLOW;
    second = exec_stack_pop();
    first = exec_stack_pop();

    switch(ip[0]) {
        case ' ':
            switch(ip[1]) {
                case ' ': result = first + second; break;
                case '\t': result = first - second; break;
                case '\n': result = first * second; break;
            }
            break;

        case '\t':
            switch(ip[1]) {
                case ' ': result = first / second; break;
                case '\t': result = first % second; break;
                default: return DO_SYNTAX_ERROR;
            }
            break;

        default:
            return DO_SYNTAX_ERROR;
    }

    exec_stack_push(result);
    return DO_OKAY;
}




/* interprt_do_heap_access
 *
 * interpret a heap access command. where ip points to the first
 * command bit (right after imp)
 */
static interprt_do_stat interprt_do_heap_access(const unsigned char *ip)
{
    signed int value;
    unsigned int address;

    switch(*ip) {
        case ' ': /* store */
            if(exec_stack_len < 2) return DO_STACK_UNDERFLOW;

            /* return DO_STACK_UNDERFLOW if address is negative! */
            if(exec_stack[exec_stack_len - 2] < 0) return DO_STACK_UNDERFLOW;

            value = exec_stack_pop();
            address = exec_stack_pop(); 

            exec_heap_allocate(address);
            exec_heap_write(address,value);
            break;

        case '\t': /* retrieve */
            if(! exec_stack_len) return DO_STACK_UNDERFLOW;

            /* return DO_STACK_UNDERFLOW if address is negative! */
            if(exec_stack_get() < 0) return DO_STACK_UNDERFLOW;

            address = exec_stack_pop();

            exec_heap_allocate(address);
            exec_stack_push(exec_heap_read(address));
            break;

        default:
            return DO_SYNTAX_ERROR;
    }

    return DO_OKAY;
}




/* interprt_do_io_command
 *
 * interpret a I/O command. where ip points to the first
 * command bit (right after imp)
 */
static interprt_do_stat interprt_do_io_command(const unsigned char *ip)
{
    signed int value = 0;

    if(*ip == ' ') {
        /* we got an I/O write */
        if(! exec_stack_len) return DO_STACK_UNDERFLOW;
        value = exec_stack_pop(); 

        switch(ip[1]) {
            case ' ': /* output character */
                printf("%c", value);
                break;

            case '\t': /* output number */
                printf("%d", value);
                break;
                
            default:
                return DO_SYNTAX_ERROR;
        }

        return DO_OKAY;
    }

    if(*ip != '\t') return DO_SYNTAX_ERROR;

#ifdef __USE_POSIX
    /* turn off canonical mode, if it isn't turn off yet! */
{
    struct termio backup, termmode;
    int termio_restore_backup = 0;

    if(! ioctl(0, TCGETA, &termmode)) {
        memmove(&backup, &termmode, sizeof(struct termio));

        termmode.c_lflag &= ~ICANON;
        termmode.c_cc[VMIN] = 1;
        termmode.c_cc[VTIME] = 0;

        if(! ioctl(0, TCSETA, &termmode))
            termio_restore_backup = 1;
    }
#endif

    /* we got an I/O read insn */
    switch(ip[1]) {
        case ' ': /* read character */
            value = getchar();
            break;

        case '\t': /* read number */
            scanf("%d", &value);
            break;

        default:
            return DO_SYNTAX_ERROR;
    }

#ifdef __USE_POSIX
    /* restore previous mode of canonical flag, if necessary */
    if(termio_restore_backup)
        ioctl(0, TCSETA, &backup);
}
#endif

    /* okay, now store that value to the heap */
    {
        unsigned int address;

        if(! exec_stack_len) return DO_STACK_UNDERFLOW;

        /* return DO_STACK_UNDERFLOW if address is negative! */
        if(exec_stack_get() < 0) return DO_STACK_UNDERFLOW;

        address = exec_stack_pop();

        exec_heap_allocate(address);
        exec_heap_write(address,value);
    }

    return DO_OKAY;
}




/* interprt_do_flow_control
 *
 * interpret a flow control command. where ip points to the first
 * command bit (right after imp)
 */
static interprt_do_stat interprt_do_flow_control(const unsigned char *ip)
{
    switch(ip[0]) {
        case ' ':
            switch(ip[1]) {
                case ' ': /* label mark, ignore */
                    break;

                case '\t': /* call subroutine */
                    /* interprt_search_label pushes the correct instruction
                     * pointer onto the stack. the old one (aka this call here
                     * is left untouched and must be corrected when RETURN
                     * is called below!
                     */
                    return interprt_search_label(&ip[2]);

                case '\n': /* unconditional jump */
                    exec_bt_len --; /* drop old ip */
                    return interprt_search_label(&ip[2]);

                default:
                    return DO_SYNTAX_ERROR;
            }
            break;

        case '\t':
            switch(ip[1]) {
                case '\n': /* subroutine return */
                    {
                        unsigned int ip;
                        
                        exec_bt_len --; /* discard our instruction pointer */
                        ip = exec_bt_pop(); /* ip of caller */

                        /* now advance the former ip */
                        for(;wsdata[ip] != 0; ip ++);
                        exec_bt_push(ip + 1);
                        return DO_OKAY_IP_MANIP;
                    }

                case ' ':
                case '\t':
                    /* space => jump if stack value is zero
                     * tab => jump if stack value is negative
                     */
                    {
                        signed int value;

                        if(! exec_stack_len) return DO_STACK_UNDERFLOW;
                        value = exec_stack_pop();

                        if((ip[1] == ' ' && value == 0) || 
                           (ip[1] == '\t' && value < 0)) {
                            /* okay, we've got to jump ... */
                            exec_bt_len --; /* drop old insn ptr */
                            return interprt_search_label(&ip[2]);
                        }
                    }
                    /* nope, don't jump since conditions not met */
                    break;

                default:
                    return DO_SYNTAX_ERROR;
            }
                
            break;

        case '\n':
            return ip[1] == '\n' ? DO_EXIT : DO_SYNTAX_ERROR;

        default:
            return DO_SYNTAX_ERROR;
    }

    return DO_OKAY;
}




/* interprt_search_label
 *
 * scan the wsdata array for the label, pointed to by label argument
 */
static interprt_do_stat interprt_search_label(const unsigned char *label)
{
    label_cache_t *bucket_ptr;
    int bucket = label_cache_hash_func(label);

    if(! label_cache_ready) label_cache_create();

    for(bucket_ptr = label_cache[bucket]; bucket_ptr != NULL;
        bucket_ptr = bucket_ptr->next)
        /* scan through the whole bucket step by step, scanning for the
         * label we need
         */
        if(! u_strcmp(&wsdata[bucket_ptr->ws_ptr], label)) {
            /* hey, got the label, push pointer to the next insn to stack */
            unsigned char *ptr = u_strchr(&wsdata[bucket_ptr->ws_ptr], '\0');
            assert(ptr);
            exec_bt_push((ptr - wsdata) + 1); /* begin of next line */
            return DO_OKAY_IP_MANIP;
        }

    return DO_LABEL_NOT_FOUND;
}




/* void interprt_err_handler(FILE *target interprt_do_stat)
 *
 * Write out an error message to the user, if the interprt_do_... worker
 * function ran into an error. Furthermore write out the current line,
 * the bt-stack pointer points to.
 */
interprt_do_stat interprt_err_handler(FILE *target, interprt_do_stat status)
{
    switch(status) {
        case DO_SYNTAX_ERROR:
            fprintf(target,"Syntax Error, cannot continue.\n");
            break;
        
        case DO_STACK_UNDERFLOW:
            fprintf(target,"Stack Underflown, unable to continue.\n");
            break;

        case DO_EXIT:
            fprintf(target, "Program exited normally.\n");
            return status; /* -> don't write stack dump */

        case DO_REACHED_BREAKPOINT:
            fprintf(target, 
                "Breakpoint at 0x%04x reached.\n", exec_bt_get() - 2);
            break;

        case DO_OKAY:
        case DO_OKAY_IP_MANIP:
            break; /* simple write stack dump, everythin's right */

        case DO_LABEL_NOT_FOUND:
            fprintf(target,
                "Requested label couldn't be found, cannot continue.\n");
            break;

        case DO_END_NOT_EXPECTED:
            fprintf(target, "Program didn't end on \\n\\n\\n, "
                            "but no more bits to execute, stop.\n");
            return status; /* -> don't write stack dump */
    }

    /* okay, now tell what the instruction, the IP points to, is */
    if(exec_bt_len)
        interprt_output_list(target, &wsdata[exec_bt_get()], 1);
    else
        fprintf(target, "exec_bt stack empty, is the program running?\n");

    /* okay, now write the first bits of the stack out ... */
    {
        int dump_to = exec_stack_len > 5 ? exec_stack_len - 5 : 0;
        int dump;
        
        fprintf(target, "[stack=0x%04x]: ", exec_stack_len);
        for(dump = exec_stack_len - 1; dump >= dump_to; dump --)
            if(exec_stack[dump] >= ' ' && exec_stack[dump] < 'z')
                fprintf(target, "0x%02x(%c)  ", exec_stack[dump], exec_stack[dump]);

            else if(exec_stack[dump] >= 0) {
                if(exec_stack[dump] <= 0xFF)
                    fprintf(target, "0x%02x  ", exec_stack[dump]);

                else if(exec_stack[dump] <= 0xFFFF)
                    fprintf(target, "0x%04x  ", exec_stack[dump]);

                else 
                    fprintf(target, "0x%08x  ", exec_stack[dump]);
            }
            else 
                fprintf(target, "0x%08x  ", exec_stack[dump]);

        fprintf(target, "\n");
    }

    return status;
}




/* void interprt_output_list(FILE *target, const char *wsdata_ptr, int lines)
 *
 * write out a given number of lines around the specified position
 */
void interprt_output_list(FILE *target, const unsigned char *wsdata_ptr, int lines) 
{
    if(lines > 1 && wsdata_ptr > wsdata) {
        /* okay, scroll back half of amount of lines back, to start
         * output from there ...
         */
        int scroll = lines >> 1;
        for(; wsdata_ptr > wsdata; wsdata_ptr --)
            if(wsdata_ptr[-1] == 0 && (-- scroll))
                break;
    }

    while(lines -- && wsdata_ptr < wsdata + wsdata_len) {
        int count = 8; /* write out no more but 8 elements per line */

        fprintf(target, "[ip=0x%04x]: ", wsdata_ptr - wsdata);

        for(; *wsdata_ptr && -- count; wsdata_ptr ++)
            switch(wsdata_ptr[0]) {
                case '\t': fprintf(target, "[TAB]"); break;
                case '\n': fprintf(target, "[LF]"); break;
                case ' ': fprintf(target, "[SPACE]"); break;
                case 0xCF: fprintf(target, "<break-point>"); break;
            }

        if(! count) {
            fprintf(target, "...");

            /* roll wsdata_ptr forward to beginning of next line, if we
             * need to write out any more line ...
             */
            if(lines)
                for(; *wsdata_ptr; wsdata_ptr ++);
        }

        fprintf(target, "\n");
        wsdata_ptr ++;
    }
}



/* int label_cache_hash_func(const unsigned char *)
 *
 * calculate the corresponding hash bucket number for the label
 */
static int label_cache_hash_func(const unsigned char *ptr) 
{
    int hash = 0;
    int shift = 1;

    for(; *ptr != '\n'; shift = shift == 128 ? 1 : shift << 1, ptr ++)
        if(*ptr == '\t')
            hash ^= shift;

#ifdef SMALL_CACHE
    /* user wants just a small hash, merge (xor) it down to 4 bits then
     */
    hash = (hash & 0x0F) ^ ((hash & 0xF0) >> 4);
#endif

    return hash;
}



/* void label_cache_clear(void)
 *
 * clear a previously generated label cache (especially free() allocated
 * memory on heap
 */
static void label_cache_clear(void)
{
    label_cache_t *ptr, *next;
    int bucket;
#ifdef SMALL_CACHE
    int buckets = 16;
#else
    int buckets = 256;
#endif

    for(bucket = 0; bucket < buckets; bucket ++) {
        ptr = label_cache[bucket];

        if(ptr && (ptr = ptr->next))
            do {
                next = ptr->next;
                free(ptr);
            } while((ptr = next));
    }

    memset(label_cache, 0, sizeof(label_cache));
}



/* void label_cache_create(void)
 *
 * scan the whole wsdata buffer for label marks to generate our label cache
 */
static void label_cache_create(void) 
{
    unsigned int pos;
#ifdef DEBUG_CACHE_STATISTICS
    int bucket, buckets = sizeof(label_cache) / sizeof(label_cache[0]);
    unsigned int cache_entries = 0;
#endif

    label_cache_clear();

    for(pos = 0; pos < wsdata_len; pos ++) {
        if(wsdata[pos] == '\n' && wsdata[pos + 1] == ' '
           && wsdata[pos + 2] == ' ') {
            /* okay, we've found a label marker */
            int bucket = label_cache_hash_func(&wsdata[pos + 3]);
            label_cache_t *entry = malloc(sizeof(label_cache_t));

            entry->next = label_cache[bucket];
            entry->ws_ptr = pos + 3;
            
            label_cache[bucket] = entry;

#ifdef DEBUG_CACHE_STATISTICS
            cache_entries ++;
#endif
        }
    }

    /* okay, our cache should be sane now ... */
    label_cache_ready = 1;

#ifdef DEBUG_CACHE_STATISTICS
    fprintf(stderr, 
            "=== LABEL CACHE STATISTICS ===\n\n"
            "cache has %d entries, and %d buckets\n"
            "therefore perfect bucket-depth would be: %f\n\n"
            "actual bucket depth of each bucket is however as follows:\n",
            cache_entries, buckets, (float)cache_entries / buckets);

    for(bucket = 0; bucket < buckets; bucket ++) {
        label_cache_t *ptr = label_cache[bucket];
        int entries = 0;

        for(; ptr != NULL; entries ++, ptr = ptr->next);
        fprintf(stderr, "bucket 0x%02x has %d entries.\n", bucket, entries);
    }
#endif
}



/* void label_cache_update(unsigned int start, unsigned int offset);
 *
 * Update the label_cache offsets into wsdata. Adjust every pointer pointing
 * to wsdata[start] or behind by offset
 */
void label_cache_update(unsigned int start, unsigned int offset) 
{
    label_cache_t *ptr;
    unsigned int bucket;
    unsigned int buckets = sizeof(label_cache) / sizeof(label_cache[0]);

    if(! label_cache_ready) return;

    for(bucket = 0; bucket < buckets; bucket ++)
        for(ptr = label_cache[bucket]; ptr; ptr = ptr->next)
            if(ptr->ws_ptr >= start)
                ptr->ws_ptr += offset;
}


/***** -*- emacs is great -*-
Local Variables:
mode: C
c-basic-offset: 4
indent-tabs-mode: nil
end: 
****************************/
