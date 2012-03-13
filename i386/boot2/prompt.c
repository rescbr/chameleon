/*
 * Copyright (c) 1999-2003 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Portions Copyright (c) 1999-2003 Apple Computer, Inc.  All Rights
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
/*
 * Copyright 1993 NeXT, Inc.
 * All rights reserved.
 */

#include "vers.h"
#include "boot.h"
#include "platform.h"

static char bootBanner[] = "\nDarwin/x86 boot v" I386BOOT_VERSION " - Chameleon v" I386BOOT_CHAMELEONVERSION /*" r" I386BOOT_CHAMELEONREVISION*/ "\n"
                    "Build date: " I386BOOT_BUILDDATE "\n"
                    "%dMB memory\n";

static char bootPrompt[] =
    "Press Enter to start up Darwin/x86 with no options, or you can:\n"
    "  Type -v and press Enter to start up with diagnostic messages\n"
#ifndef OPTION_ROM
    "  Type ? and press Enter to learn about advanced startup options\n\n"
#endif
    "boot: ";

#ifndef OPTION_ROM
static char bootRescanPrompt[] =
    "Press Enter to start up Darwin/x86 with no options, or you can:\n"
    "  Press F5 after you swapped the media. The drive will be rescanned.\n"
    "  Type -v and press Enter to start up with diagnostic messages\n"
    "  Type ? and press Enter to learn about advanced startup options\n\n"
    "boot: ";
#endif

void InitBootPrompt(void)
{
    set_env(envBootBanner,(uint32_t)bootBanner);
    set_env(envBootPrompt,(uint32_t)bootPrompt);
    set_env(envBootRescanPrompt,(uint32_t)bootRescanPrompt);
}
