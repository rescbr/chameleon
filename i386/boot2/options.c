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

#include "boot.h"
#include "bootstruct.h"
#include "graphics.h"
#include "fdisk.h"
#include "pci.h"
#include "options.h"
#include "modules.h"

#ifndef EMBED_HELP
#define EMBED_HELP 0
#endif

#if EMBED_HELP
#include "embedded.h" //+ 1376 bytes
#endif


bool shouldboot = false;

#if UNUSED
extern int multiboot_timeout;
extern int multiboot_timeout_set;
#endif

extern BVRef    bvChain;
//extern int		menucount;

extern int		gDeviceCount;

int			selectIndex = 0;
MenuItem *  menuItems = NULL;

static int countdown( const char * msg, int row, int timeout );
static void showBootPrompt(int row, bool visible);
static void updateBootArgs( int key );
static void showMenu( const MenuItem * items, int count,
					 int selection, int row, int height );
static int updateMenu( int key, void ** paramPtr );
static void skipblanks( const char ** cpp );
static const char * extractKernelName( char ** cpp );

//==========================================================================

void changeCursor( int col, int row, int type, CursorState * cs )
{
    if (cs) getCursorPositionAndType( &cs->x, &cs->y, &cs->type );
    setCursorType( type );
    setCursorPosition( col, row, 0 );
}

void moveCursor( int col, int row )
{
    setCursorPosition( col, row, 0 );
}

void restoreCursor( const CursorState * cs )
{
    setCursorPosition( cs->x, cs->y, 0 );
    setCursorType( cs->type );
}

//==========================================================================

/* Flush keyboard buffer; returns TRUE if any of the flushed
 * characters was F8.
 */

bool flushKeyboardBuffer(void)
{
    bool status = false;
	
    while ( readKeyboardStatus() ) {
        if (bgetc() == 0x4200) status = true;
    }
    return status;
}

//==========================================================================

static int countdown( const char * msg, int row, int timeout )
{
    unsigned long time;
    int ch  = 0;
    int col = strlen(msg) + 1;
	
    flushKeyboardBuffer();
	
	moveCursor( 0, row );
	printf(msg);
	/*
	int multi_buff = 18 * (timeout);
    int multi = ++multi_buff;
	*/
    //unsigned int lasttime=0;
	
    for ( time = time18(), timeout++; timeout > 0; )
    {
		/*
		if( time18() > lasttime)
		{
			multi--; 
			lasttime=time18();
		}		
		*/
        if ((ch = readKeyboardStatus()))
            break;
		
        // Count can be interrupted by holding down shift,
        // control or alt key
        if ( ( readKeyboardShiftFlags() & 0x0F ) != 0 )
		{
            ch = 1;
            break;
        }
		
        if ( time18() >= time )
        {
            time += 18;
            timeout--;
			
			moveCursor( col, row );
			printf("(%d) ", timeout);
        }
    }
	
    flushKeyboardBuffer();
	
    return ch;
}

//==========================================================================

 char   gBootArgs[BOOT_STRING_LEN];
 char * gBootArgsPtr = gBootArgs;
 char * gBootArgsEnd = gBootArgs + BOOT_STRING_LEN - 1;
 char   booterCommand[BOOT_STRING_LEN];
 char   booterParam[BOOT_STRING_LEN];

void clearBootArgs(void)
{
	gBootArgsPtr = gBootArgs;
	memset(gBootArgs, '\0', BOOT_STRING_LEN);
}

void addBootArg(const char * argStr)
{
	if ( (gBootArgsPtr + strlen(argStr) + 1) < gBootArgsEnd)
	{
		*gBootArgsPtr++ = ' ';
		strcat(gBootArgs, argStr);
		gBootArgsPtr += strlen(argStr);
	}
}

//==========================================================================

static void showBootPrompt(int row, bool visible)
{
	extern char bootPrompt[];
#ifndef OPTION_ROM
	extern char bootRescanPrompt[];
#endif
	
	changeCursor( 0, row, kCursorTypeUnderline, 0 );    
	clearScreenRows( row, kScreenLastRow );
	
	clearBootArgs();
	
	if (visible) {
#ifndef OPTION_ROM
		if (gEnableCDROMRescan)
		{
			printf( bootRescanPrompt );
		}
		else
#endif
		{
			printf( bootPrompt );
		}
	} else {
		printf("Press Enter to start up the foreign OS. ");
	}
}

//==========================================================================

static void updateBootArgs( int key )
{
    key &= kASCIIKeyMask;
	
    switch ( key )
    {
        case kBackspaceKey:
            if ( gBootArgsPtr > gBootArgs )
            {
                int x, y, t;
                getCursorPositionAndType( &x, &y, &t );
                if ( x == 0 && y )
                {
                    x = 80; y--;
                }
                if (x)
				{
					x--;
				}
				
				setCursorPosition( x, y, 0 );
				putca(' ', 0x07, 1);
				
				*gBootArgsPtr-- = '\0';
			}
            
			break;
			
        default:
            if ( key >= ' ' && gBootArgsPtr < gBootArgsEnd)
            {
				putchar(key);  // echo to screen
				*gBootArgsPtr++ = key;
			}
            
			break;
    }
}

//==========================================================================

 const MenuItem * gMenuItems = NULL;

 int   gMenuItemCount;
 int   gMenuRow;
 int   gMenuHeight;
 int   gMenuTop;
 int   gMenuBottom;
 int   gMenuSelection;

 int	 gMenuStart;
 int	 gMenuEnd;

void printMenuItem( const MenuItem * item, int highlight )
{
    printf("  ");
	
    if ( highlight )
        putca(' ', 0x70, strlen(item->name) + 4);
    else
        putca(' ', 0x07, 40);
	
    printf("  %40s\n", item->name);
}

//==========================================================================

static void showMenu( const MenuItem * items, int count,
					 int selection, int row, int height )
{
    int         i;
    CursorState cursorState;
	
    if ( items == NULL || count == 0 ) 
		return;
	
    // head and tail points to the start and the end of the list.
    // top and bottom points to the first and last visible items
    // in the menu window.
	
    gMenuItems		= items;
    gMenuRow		= row;
    gMenuHeight		= height;
    gMenuItemCount	= count;
    gMenuTop		= 0;
    gMenuBottom		= min( count, height ) - 1;
    gMenuSelection	= selection;
	
    gMenuStart		= 0;
    gMenuEnd	    = count; //min( count, gui.maxdevices ) - 1;
	
	// If the selected item is not visible, shift the list down.
	
    if ( gMenuSelection > gMenuBottom )
    {
        gMenuTop += ( gMenuSelection - gMenuBottom );
        gMenuBottom = gMenuSelection;
    }
	
	if ( gMenuSelection > gMenuEnd )
    {
		gMenuStart += ( gMenuSelection - gMenuEnd );
        gMenuEnd = gMenuSelection;
    }
	
	// Draw the visible items.
	
	changeCursor( 0, row, kCursorTypeHidden, &cursorState );
	
	for ( i = gMenuTop; i <= gMenuBottom; i++ )
	{
		printMenuItem( &items[i], (i == gMenuSelection) );
	}
	
	restoreCursor( &cursorState );
}

//==========================================================================

static int updateMenu( int key, void ** paramPtr )
{
    int moved = 0;
	
    union {
        struct {
            unsigned int
			selectionUp   : 1,
			selectionDown : 1,
			scrollUp      : 1,
			scrollDown    : 1;
        } f;
        unsigned int w;
    } draw = {{0}};
	
	if ( gMenuItems == NULL )
		return 0;
	
	switch ( key )
	{
		case 0x4800:  // Up Arrow
			if ( gMenuSelection != gMenuTop )
				draw.f.selectionUp = 1;
			else if ( gMenuTop > 0 )
				draw.f.scrollDown = 1;
			break;
			
		case 0x5000:  // Down Arrow
			if ( gMenuSelection != gMenuBottom )
				draw.f.selectionDown = 1;
			else if ( gMenuBottom < (gMenuItemCount - 1) ) 
				draw.f.scrollUp = 1;
			break;
		default:
			break;
	}
	
    if ( draw.w )
    {
        if ( draw.f.scrollUp )
        {
            scollPage(0, gMenuRow, 40, gMenuRow + gMenuHeight - 1, 0x07, 1, 1);
            gMenuTop++; gMenuBottom++;
			gMenuStart++; gMenuEnd++;
            draw.f.selectionDown = 1;
        }
		
        if ( draw.f.scrollDown )
        {
            scollPage(0, gMenuRow, 40, gMenuRow + gMenuHeight - 1, 0x07, 1, -1);
            gMenuTop--; gMenuBottom--;
            gMenuStart--; gMenuEnd--;
            draw.f.selectionUp = 1;
        }
		
        if ( draw.f.selectionUp || draw.f.selectionDown )
        {
			
			CursorState cursorState;
			
			// Set cursor at current position, and clear inverse video.
			changeCursor( 0, gMenuRow + gMenuSelection - gMenuTop, kCursorTypeHidden, &cursorState );
			printMenuItem( &gMenuItems[gMenuSelection], 0 );
			
			if ( draw.f.selectionUp )
			{
				gMenuSelection--;
				if(( gMenuSelection - gMenuStart) == -1 )
				{
					gMenuStart--;
					gMenuEnd--;
				}
				
			} else {
				gMenuSelection++;
				if(( gMenuSelection - ( gMenuEnd - 1) - gMenuStart) > 0 )
				{
					gMenuStart++;
					gMenuEnd++;
				}
			}
			
			moveCursor( 0, gMenuRow + gMenuSelection - gMenuTop );
			printMenuItem( &gMenuItems[gMenuSelection], 1 );
			restoreCursor( &cursorState );
			
		}
		
        *paramPtr = gMenuItems[gMenuSelection].param;        
        moved = 1;
    }
	
	return moved;
}

//==========================================================================

static void skipblanks( const char ** cpp ) 
{
    while ( **(cpp) == ' ' || **(cpp) == '\t' ) ++(*cpp);
}

//==========================================================================

static const char * extractKernelName( char ** cpp )
{
    char * kn = *cpp;
    char * cp = *cpp;
    char   c;
	
    // Convert char to lower case.
	
    c = *cp | 0x20;
	
    // Must start with a letter or a '/'.
	
    if ( (c < 'a' || c > 'z') && ( c != '/' ) ) return 0;
	
    // Keep consuming characters until we hit a separator.
	
    while ( *cp && (*cp != '=') && (*cp != ' ') && (*cp != '\t') )
	{
        cp++;
	}
    // Only SPACE or TAB separator is accepted.
    // Reject everything else.
	
    if (*cp == '=') return 0;
	
    // Overwrite the separator, and move the pointer past
    // the kernel name.
	
    if (*cp != '\0') *cp++ = '\0';
    *cpp = cp;
	
    return kn;
}

#ifndef OPTION_ROM
//==========================================================================

void printMemoryInfo(void)
{
    int line;
    unsigned long i;
    MemoryRange *mp = bootInfo->memoryMap;
	
    // Activate and clear page 1
    setActiveDisplayPage(1);
    clearScreenRows(0, 24);
    setCursorPosition( 0, 0, 1 );
	
    printf("BIOS reported memory ranges:\n");
    line = 1;
    for (i=0; i<bootInfo->memoryMapCount; i++) {
        printf("Base 0x%08x%08x, ",
               (unsigned long)(mp->base >> 32),
               (unsigned long)(mp->base));
        printf("length 0x%08x%08x, type %d\n",
               (unsigned long)(mp->length >> 32),
               (unsigned long)(mp->length),
               mp->type);
        if (line++ > 20) {
            pause();
            line = 0;
        }
        mp++;
    }
    if (line > 0) {
        pause();
    }
    
    setActiveDisplayPage(0);
}

char *getMemoryInfoString()
{
    unsigned long i;
    MemoryRange *mp = bootInfo->memoryMap;
	char *buff = malloc(sizeof(char)*1024);
	if(!buff) return 0;
	
	char info[] = "BIOS reported memory ranges:\n";
	sprintf(buff, "%s", info);
    for (i=0; i<bootInfo->memoryMapCount; i++) {
        sprintf( buff+strlen(buff), "Base 0x%08x%08x, ",
				(unsigned long)(mp->base >> 32),
				(unsigned long)(mp->base));
        sprintf( buff+strlen(buff), "length 0x%08x%08x, type %d\n",
				(unsigned long)(mp->length >> 32),
				(unsigned long)(mp->length),
				mp->type);
        mp++;
    }
	return buff;
}
//==========================================================================

void lspci(void)
{
	if (bootArgs->Video.v_display == VGA_TEXT_MODE) { 
		setActiveDisplayPage(1);
		clearScreenRows(0, 24);
		setCursorPosition(0, 0, 1);
	}
	
	dump_pci_dt(root_pci_dev->children);
	
	pause();
	
	if (bootArgs->Video.v_display == VGA_TEXT_MODE) {
		setActiveDisplayPage(0);
	}
}
#endif

//==========================================================================

int getBootOptions(bool firstRun)
{
	int     i;
	int     key;
	int     nextRow;
	int     timeout;
#if UNUSED
	int     bvCount;
#endif
	BVRef   bvr;
	BVRef   menuBVR;
	bool    showPrompt, newShowPrompt, isCDROM;
	
	// Initialize default menu selection entry.
	gBootVolume = menuBVR = selectBootVolume(bvChain);
	
	if (biosDevIsCDROM(gBIOSDev)) {
		isCDROM = true;
	} else {
		isCDROM = false;
	}
	
	// Clear command line boot arguments
	clearBootArgs();
	
	// Allow user to override default timeout.
#if UNUSED
	if (multiboot_timeout_set) {
		timeout = multiboot_timeout;
	} else 
#endif
	if (!getIntForKey(kTimeoutKey, &timeout, &bootInfo->bootConfig)) {
		/*  If there is no timeout key in the file use the default timeout
		 which is different for CDs vs. hard disks.  However, if not booting
		 a CD and no config file could be loaded set the timeout
		 to zero which causes the menu to display immediately.
		 This way, if no partitions can be found, that is the disk is unpartitioned
		 or simply cannot be read) then an empty menu is displayed.
		 If some partitions are found, for example a Windows partition, then
		 these will be displayed in the menu as foreign partitions.
		 */
		if (isCDROM) {
			timeout = kCDBootTimeout;
		} else {
			timeout = sysConfigValid ? kBootTimeout : 0;
		}
	}
	
	if (timeout < 0) {
		gBootMode |= kBootModeQuiet;
	}
	
	// If the user is holding down a modifier key, enter safe mode.
	if ((readKeyboardShiftFlags() & 0x0F) != 0) {
		
		//gBootMode |= kBootModeSafe;
	}
	
	// Checking user pressed keys
	bool f8press = false, spress = false, vpress = false;
	while (readKeyboardStatus()) {
		key = bgetc ();
		if (key == 0x4200) f8press = true;
		if ((key & 0xff) == 's' || (key & 0xff) == 'S') spress = true;
		if ((key & 0xff) == 'v' || (key & 0xff) == 'V') vpress = true;
	}
	// If user typed F8, abort quiet mode, and display the menu.
	if (f8press) {
		gBootMode &= ~kBootModeQuiet;
		timeout = 0;
	}
	// If user typed 'v' or 'V', boot in verbose mode.
	if ((gBootMode & kBootModeQuiet) && firstRun && vpress) {
		addBootArg(kVerboseModeFlag);
	}
	// If user typed 's' or 'S', boot in single user mode.
	if ((gBootMode & kBootModeQuiet) && firstRun && spress) {
		addBootArg(kSingleUserModeFlag);
	}
	
	setCursorPosition(0, 0, 0);
	clearScreenRows(0, kScreenLastRow);
	if (!(gBootMode & kBootModeQuiet)) {
		// Display banner and show hardware info.
		printf(bootBanner, (bootInfo->convmem + bootInfo->extmem) / 1024);
	}
	changeCursor(0, kMenuTopRow, kCursorTypeUnderline, 0);
	verbose("Scanning device %x...", gBIOSDev);
	
	// When booting from CD, default to hard drive boot when possible. 
	if (isCDROM && firstRun) {
		const char *val;
		char *prompt = NULL;
		char *name = NULL;
		int cnt;
		int optionKey;
		
		if (getValueForKey(kCDROMPromptKey, &val, &cnt, &bootInfo->bootConfig)) {
			prompt = malloc(cnt + 1);
			strncat(prompt, val, cnt);
		} else {
			name = malloc(80);
			getBootVolumeDescription(gBootVolume, name, 79, false);
			prompt = malloc(256);
			sprintf(prompt, "Press any key to start up from %s, or press F8 to enter startup options.", name);
			free(name);
		}
		
		if (getIntForKey( kCDROMOptionKey, &optionKey, &bootInfo->bootConfig )) {
			// The key specified is a special key.
		} else {
			// Default to F8.
			optionKey = 0x4200;
		}
		
		// If the timeout is zero then it must have been set above due to the
		// early catch of F8 which means the user wants to set boot options
		// which we ought to interpret as meaning he wants to boot the CD.
		if (timeout != 0) {
			key = countdown(prompt, kMenuTopRow, timeout);
		} else {
			key = optionKey;
		}
		
		if (prompt != NULL) {
			free(prompt);
		}
		
		clearScreenRows( kMenuTopRow, kMenuTopRow + 2 );
		
		// Hit the option key ?
		if (key == optionKey) {
			gBootMode &= ~kBootModeQuiet;
			timeout = 0;
		} else {
			key = key & 0xFF;
			
			// Try booting hard disk if user pressed 'h'
			if (biosDevIsCDROM(gBIOSDev) && key == 'h') {
				BVRef bvr;
				
				// Look at partitions hosting OS X other than the CD-ROM
				for (bvr = bvChain; bvr; bvr=bvr->next) {
					if ((bvr->flags & kBVFlagSystemVolume) && bvr->biosdev != gBIOSDev) {
						gBootVolume = bvr;
					}
				}
			}
			goto done;
		}
	}
	
	if (gBootMode & kBootModeQuiet) {
		// No input allowed from user.
		goto done;
	}
	
	if (firstRun && timeout > 0 && countdown("Press any key to enter startup options.", kMenuTopRow, timeout) == 0) {
		// If the user is holding down a modifier key,
		// enter safe mode.
		if ((readKeyboardShiftFlags() & 0x0F) != 0) {
			gBootMode |= kBootModeSafe;
		}
		goto done;
	}
	
	if (gDeviceCount) {
		// Allocate memory for an array of menu items.
		menuItems = malloc(sizeof(MenuItem) * gDeviceCount);
		if (menuItems == NULL) {
			goto done;
		}
		
		// Associate a menu item for each BVRef.
		for (bvr=bvChain, i=gDeviceCount-1, selectIndex=0; bvr; bvr=bvr->next) {
			if (bvr->visible) {
				getBootVolumeDescription(bvr, menuItems[i].name, sizeof(menuItems[i].name) - 1, true);
				menuItems[i].param = (void *) bvr;
				if (bvr == menuBVR) {
					selectIndex = i;
				}
				i--;
			}
		}
	}
	
	// Clear screen and hide the blinking cursor.
	clearScreenRows(kMenuTopRow, kMenuTopRow + 2);
	changeCursor(0, kMenuTopRow, kCursorTypeHidden, 0);
	
	nextRow = kMenuTopRow;
	showPrompt = true;
	
	if (gDeviceCount) {
		printf("Use \30\31 keys to select the startup volume.");
		showMenu( menuItems, gDeviceCount, selectIndex, kMenuTopRow + 2, kMenuMaxItems );
		nextRow += min( gDeviceCount, kMenuMaxItems ) + 3;
	}
	
	// Show the boot prompt.
	showPrompt = (gDeviceCount == 0) || (menuBVR->flags & kBVFlagNativeBoot);
	showBootPrompt( nextRow, showPrompt );
	
	do {
		key = getc();
		updateMenu( key, (void **) &menuBVR );
		newShowPrompt = (gDeviceCount == 0) || (menuBVR->flags & kBVFlagNativeBoot);
		
		if (newShowPrompt != showPrompt)
		{
			showPrompt = newShowPrompt;
			showBootPrompt( nextRow, showPrompt );
		}
		
		if (showPrompt)
		{
			updateBootArgs(key);
		}
		
		switch (key) {
			case kReturnKey:
				if (*gBootArgs == '?') {
					char * argPtr = gBootArgs;
					
					// Skip the leading "?" character.
					argPtr++;
					getNextArg(&argPtr, booterCommand);
					getNextArg(&argPtr, booterParam);
					
					/*
					 * TODO: this needs to be refactored.
					 */
#ifndef OPTION_ROM
#if UNUSED
					if (strcmp( booterCommand, "video" ) == 0)
					{
						printVBEModeInfo();
					}
					else
#endif
					if ( strcmp( booterCommand, "memory" ) == 0) 
					{
						printMemoryInfo();
					}
					else if (strcmp(booterCommand, "lspci") == 0) 
					{
						lspci();
					} 
					else if (strcmp(booterCommand, "more") == 0) 
					{
						showTextFile(booterParam);
					}
					else if (strcmp(booterCommand, "rd") == 0) 
					{
						if (execute_hook("processRAMDiskCommand", (void*)argPtr, &booterParam, NULL, NULL, NULL, NULL) != EFI_SUCCESS)
						showMessage("ramdisk module not found, please install RamdiskLoader.dylib in /Extra/modules/");
					} 
					else if (strcmp(booterCommand, "norescan") == 0)
					{
						if (gEnableCDROMRescan)
						{
							gEnableCDROMRescan = false;
							break;
						}
					}
					else
					{
						showHelp();
					}
#endif
					key = 0;
					showBootPrompt(nextRow, showPrompt);
					break;
				}
				gBootVolume = menuBVR;
				setRootVolume(menuBVR);
				gBIOSDev = menuBVR->biosdev;
				break;
				
			case kEscapeKey:
				clearBootArgs();
				break;
#ifndef OPTION_ROM
			case kF5Key:
				// New behavior:
				// Clear gBootVolume to restart the loop
				// if the user enabled rescanning the optical drive.
				// Otherwise boot the default boot volume.
				if (gEnableCDROMRescan) {
					gBootVolume = NULL;
					clearBootArgs();
				}
				break;
				
			case kF10Key:
				gScanSingleDrive = false;
#if UNUSED
                scanDisks(gBIOSDev, &bvCount);
#else
                scanDisks();
#endif
				gBootVolume = NULL;
				clearBootArgs();
				break;
#endif
			default:
				key = 0;
				break;
		}
	} while (0 == key);
	
done:
	if (bootArgs->Video.v_display == VGA_TEXT_MODE) {
		clearScreenRows(kMenuTopRow, kScreenLastRow);
		changeCursor(0, kMenuTopRow, kCursorTypeUnderline, 0);
	}
	shouldboot = false;
	if (menuItems) {
		free(menuItems);
		menuItems = NULL;
	}
	return 0;
}

//==========================================================================

extern unsigned char chainbootdev;
extern unsigned char chainbootflag;

bool copyArgument(const char *argName, const char *val, int cnt, char **argP, int *cntRemainingP)
{
    int argLen = argName ? strlen(argName) : 0;
    int len = argLen + cnt + 1;  // +1 to account for space
	
    if (len > *cntRemainingP) {
        error("Warning: boot arguments too long, truncating\n");
        return false;
    }
	
    if (argName) {
        strncpy( *argP, argName, argLen );
        *argP += argLen;
        *argP[0] = '=';
        (*argP)++;
        len++; // +1 to account for '='
    }
    strncpy( *argP, val, cnt );
    *argP += cnt;
    *argP[0] = ' ';
    (*argP)++;
	
    *cntRemainingP -= len;
    return true;
}

int              ArgCntRemaining;

int
processBootOptions()
{
    const char *     cp  = gBootArgs;
    const char *     val = 0;
    const char *     kernel;
    int              cnt;
    int		     userCnt;
    char *           argP;
    char *           configKernelFlags;
        
    skipblanks( &cp );
	
    // Update the unit and partition number.
	
    if ( gBootVolume )
    {
        if (!( gBootVolume->flags & kBVFlagNativeBoot ))
        {
            readBootSector( gBootVolume->biosdev, gBootVolume->part_boff,
						   (void *) 0x7c00 );
			
            //
            // Setup edx, and signal intention to chain load the
            // foreign booter.
            //
			
            chainbootdev  = gBootVolume->biosdev;
            chainbootflag = 1;
			
            return 1;
        }
		
        setRootVolume(gBootVolume);
		
    }
    // If no boot volume fail immediately because we're just going to fail
    // trying to load the config file anyway.
    else
		return -1;
	
    // Load config table specified by the user, or use the default.
	
    if (!getValueForBootKey(cp, "config", &val, &cnt)) {
		val = 0;
		cnt = 0;
    }
	
    // Load com.apple.Boot.plist from the selected volume
    // and use its contents to override default bootConfig.
    // This is not a mandatory opeartion anymore.
	
    loadOverrideConfig(&bootInfo->overrideConfig);
	
    // Use the kernel name specified by the user, or fetch the name
    // in the config table, or use the default if not specified.
    // Specifying a kernel name on the command line, or specifying
    // a non-default kernel name in the config file counts as
    // overriding the kernel, which causes the kernelcache not
    // to be used.
	
    gOverrideKernel = false;
    if (( kernel = extractKernelName((char **)&cp) )) {        
		strlcpy( bootInfo->bootFile, kernel, sizeof(bootInfo->bootFile)+1 );
        gOverrideKernel = true;
    } else {
        if ( getValueForKey( kKernelNameKey, &val, &cnt, &bootInfo->bootConfig ) ) {
            strlcpy( bootInfo->bootFile, val, cnt+1 );
            if (strcmp( bootInfo->bootFile, kDefaultKernel ) != 0) {
                gOverrideKernel = true;
            }
        } else {
			strlcpy( bootInfo->bootFile, kDefaultKernel, sizeof(bootInfo->bootFile)+1 );
        }
    }
	
    ArgCntRemaining = BOOT_STRING_LEN - 2;  // save 1 for NULL, 1 for space
    argP = bootArgs->CommandLine;
	
    // Get config table kernel flags, if not ignored.
    if (getValueForBootKey(cp, kIgnoreBootFileFlag, &val, &cnt) ||
		!getValueForKey( kKernelFlagsKey, &val, &cnt, &bootInfo->bootConfig )) {
        val = "";
        cnt = 0;
    }
	
    configKernelFlags = malloc(cnt + 1);
    strlcpy(configKernelFlags, val, cnt + 1);
	
    	
    if (!getValueForBootKey(cp, kSafeModeFlag, &val, &cnt) &&
        !getValueForBootKey(configKernelFlags, kSafeModeFlag, &val, &cnt)) {
        if (gBootMode & kBootModeSafe) {
            copyArgument(0, kSafeModeFlag, strlen(kSafeModeFlag), &argP, &ArgCntRemaining);
        }
    }
	
    // Store the merged kernel flags and boot args.
	
    cnt = strlen(configKernelFlags);
    if (cnt) {
        if (cnt > ArgCntRemaining) {
            error("Warning: boot arguments too long, truncating\n");
            cnt = ArgCntRemaining;
        }
        strncpy(argP, configKernelFlags, cnt);
        argP[cnt++] = ' ';
        ArgCntRemaining -= cnt;
    }
    userCnt = strlen(cp);
    if (userCnt > ArgCntRemaining) {
		error("Warning: boot arguments too long, truncating\n");
		userCnt = ArgCntRemaining;
    }
    strncpy(&argP[cnt], cp, userCnt);
    argP[cnt+userCnt] = '\0';
	
	if(!shouldboot)
	{
		gVerboseMode = getValueForKey( kVerboseModeFlag, &val, &cnt, &bootInfo->bootConfig ) ||
		getValueForKey( kSingleUserModeFlag, &val, &cnt, &bootInfo->bootConfig );
		
		gBootMode = ( getValueForKey( kSafeModeFlag, &val, &cnt, &bootInfo->bootConfig ) ) ?
		kBootModeSafe : kBootModeNormal;
		
        if ( getValueForKey( kIgnoreCachesFlag, &val, &cnt, &bootInfo->bootConfig ) ) {
            gBootMode = kBootModeSafe;
		}
	}
	
	if ( getValueForKey( kMKextCacheKey, &val, &cnt, &bootInfo->bootConfig ) )
	{
		strlcpy(gMKextName, val, cnt + 1);
	}
	
    free(configKernelFlags);
	
    return 0;
}

#ifndef OPTION_ROM

//==========================================================================
// Load the help file and display the file contents on the screen.

void showTextBuffer(char *buf, int size)
{
	char	*bp;
	int	line;
	int	line_offset;
	int	c;
	
	
	bp = buf;
	while (size-- > 0) {
		if (*bp == '\n') {
			*bp = '\0';
		}
		bp++;
	}
	*bp = '\1';
	line_offset = 0;
	
	setActiveDisplayPage(1);
	
	while (1) {
		clearScreenRows(0, 24);
		setCursorPosition(0, 0, 1);
		bp = buf;
		for (line = 0; *bp != '\1' && line < line_offset; line++) {
			while (*bp != '\0') {
				bp++;
			}
			bp++;
		}
		for (line = 0; *bp != '\1' && line < 23; line++) {
			setCursorPosition(0, line, 1);
			printf("%s\n", bp);
			while (*bp != '\0') {
				bp++;
			}
			bp++;
		}
		
		setCursorPosition(0, 23, 1);
		if (*bp == '\1') {
			printf("[Type %sq or space to quit viewer]", (line_offset > 0) ? "p for previous page, " : "");
		} else {
			printf("[Type %s%sq to quit viewer]", (line_offset > 0) ? "p for previous page, " : "", (*bp != '\1') ? "space for next page, " : "");
		}
		
		c = getc();
		if (c == 'q' || c == 'Q') {
			break;
		}
		if ((c == 'p' || c == 'P') && line_offset > 0) {
			line_offset -= 23;
		}
		if (c == ' ') {
			if (*bp == '\1') {
				break;
			} else {
				line_offset += 23;
			}
		}
	}
	setActiveDisplayPage(0);
}

void showHelp(void)
{
#if EMBED_HELP==0
	int fd = -1;
	char dirspec[512];
	char filename[512];
	sprintf(filename, "BootHelp.txt");
	
		// Check Extra on booting partition
		sprintf(dirspec,"/Extra/%s",filename);
		fd=open (dirspec);
		if (fd<0)
		{	// Fall back to booter partition
			sprintf(dirspec,"bt(0,0)/Extra/%s",filename);
			fd=open (dirspec);
			if (fd<0)
			{
				printf("BootHelp not found: %s\n", filename);
				return;
			}
		}
	int BootHelp_txt_len = file_size (fd);
	void *BootHelp_txt=malloc(BootHelp_txt_len);
	if (BootHelp_txt)
	{
		if (read (fd, BootHelp_txt, BootHelp_txt_len)!=BootHelp_txt_len)
		{
			printf("Couldn't read BootHelp %s\n",dirspec);
			free (BootHelp_txt);
			close (fd);
			return;
		}
				
		close (fd);		
	}
#endif
	showTextBuffer((char *)BootHelp_txt, BootHelp_txt_len);
}

void showMessage(char * message)
{
	showTextBuffer(message, strlen(message));
}

void showTextFile(const char * filename)
{
#define MAX_TEXT_FILE_SIZE 65536
	char	*buf;
	int	fd;
	int	size;
	
	if ((fd = open_bvdev("bt(0,0)", filename)) < 0) {
		printf("\nFile not found: %s\n", filename);
		sleep(2);
		return;
	}
	
	size = file_size(fd);
	if (size > MAX_TEXT_FILE_SIZE) {
		size = MAX_TEXT_FILE_SIZE;
	}
	buf = malloc(size);
	read(fd, buf, size);
	close(fd);
	showTextBuffer(buf, size);
	free(buf);
}
#endif

#ifndef OPTION_ROM
bool promptForRescanOption(void)
{
	printf("\nWould you like to enable media rescan option?\nPress ENTER to enable or any key to skip.\n");
	if (getc() == kReturnKey) {
		return true;
	} else {
		return false;
	}
}
#endif
