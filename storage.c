/* vim: expandtab sw=4 sts=4 ts=8
 **********************************************************
 * storage.c
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

#include "storage.h"

#ifndef NULL
#define NULL ((void*)0)
#endif

/* this file will probably stay rather empty, since it's only necessary
 * to define the wsdata and compose stack somewhere.
 */

STACK_DEF(unsigned char, wsdata, wsdata_len, wsdata_alloc)
STACK_DEF(unsigned char, compose, compose_len, compose_alloc)




/***** -*- emacs is great -*-
Local Variables:
mode: C
c-basic-offset: 4
indent-tabs-mode: nil
end: 
****************************/
