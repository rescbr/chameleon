/*
 * Copyright (c) 1999-2004 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Portions Copyright (c) 1999-2004 Apple Computer, Inc.  All Rights
 * Reserved.  This file contains Original Code and/or Modifications of
 * Original Code as defined in and that are subject to the Apple Public
 * Source License Version 2.0 (the "License").  You may not use this file
 * except in compliance with the License.  Please obtain a copy of the
 * License at http://www.apple.com/publicsource and read it before using
 * this file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON- INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */



#ifndef __BOOT2_OPTIONS_H
#define __BOOT2_OPTIONS_H

#include "bootstruct.h"
#include "graphics.h"

/*
 * options.c
 */
extern int getBootOptions(bool firstRun);
extern int processBootOptions(void);
extern bool promptForRescanOption(void);
extern bool copyArgument(const char *argName, const char *val, int cnt, char **argP, int *cntRemainingP);


void showHelp(void);
void showTextFile(const char * filename);
char *getMemoryInfoString(void);
void showMessage(char * message);
void showTextBuffer(char *buf, int size);

void clearBootArgs(void);
void addBootArg(const char * argStr);
void printMemoryInfo(void);
void lspci(void);
char *getMemoryInfoString(void);

#ifdef UNUSED
extern int multiboot_timeout;
extern int multiboot_timeout_set;
#endif


// Maximum config table value size
#define VALUE_SIZE 2048

#endif /* __BOOT2_OPTIONS_H */