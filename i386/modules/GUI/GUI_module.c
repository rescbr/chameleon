/*
 * Copyright (c) 2009 Evan Lojewski. All rights reserved.
 *
 */

#include "libsaio.h"
#include "options.h"
#include "graphic_utils.h"
#include "ramdisk.h"
#include "embedded.h"

#include "picopng.h"
#include "gui.h"

#include "modules.h"

/* Kabyl: BooterLog */
#define BOOTER_LOG_SIZE	(64 * 1024)
#define SAFE_LOG_SIZE	80


bool useGUI;

void GUI_Kernel_Start_hook(void* kernelEntry, void* arg2, void* arg3, void* arg4);
void GUI_PreBoot_hook(void* arg1, void* arg2, void* arg3, void* arg4);

int GUI_getBootOptions(bool firstRun);

static void GUI_updateBootArgs( int key );
void GUI_clearBootArgs(void);
static void GUI_showBootPrompt(int row, bool visible);
static void GUI_showMenu( const MenuItem * items, int count, int selection, int row, int height );
static int GUI_updateMenu( int key, void ** paramPtr );
static void GUI_showHelp(void);

int GUI_printf(const char * fmt, ...);
int GUI_verbose(const char * fmt, ...);
int GUI_error(const char * fmt, ...);
void GUI_stop(const char * fmt, ...);


/* console.c */
struct putc_info {
    char * str;
    char * last_str;
};
void sputc(int c, struct putc_info * pi);
extern char *msgbuf;
extern char *cursor;




char GUI_bootRescanPrompt[] =
"Press Enter to start up Darwin/x86 with no options, or you can:\n"
"  Press F5 after you swapped the media. The drive will be rescanned.\n"
"  Type -v and press Enter to start up with diagnostic messages\n"
"  Type ? and press Enter to learn about advanced startup options\n\n"
"boot: ";



/**
 ** The kernel is about to start, draw the boot graphics if we are not in
 ** verbose mode.
 **/
void GUI_ExecKernel_hook(void* kernelEntry, void* arg2, void* arg3, void* arg4)
{
	if(!gVerboseMode)
	{
		// Note: shouldn't be needed, but just in case
		drawBootGraphics();
	}
	else
	{
		setVideoMode( GRAPHICS_MODE, 0 );
		
	}
}

/**
 ** A boot option has been selected, disable the graphical elements on screen.
 **/
void GUI_PreBoot_hook(void* arg1, void* arg2, void* arg3, void* arg4)
{
	// Turn off any GUI elements
	if( bootArgs->Video.v_display == GRAPHICS_MODE )
	{
		gui.devicelist.draw = false;
		gui.bootprompt.draw = false;
		gui.menu.draw = false;
		gui.infobox.draw = false;
		gui.logo.draw = false;
		drawBackground();
		updateVRAM();

		if(!gVerboseMode)
		{
			// Disable outputs, they will still show in the boot log.
			replace_function("_printf", &GUI_verbose);
			drawBootGraphics();
		}
		
	}
}

/**
 ** Module startup code. Replace console only print functions as well as
 ** replace various menu functions. Finaly, initialize the gui and hook
 ** into important events.
 **/
void GUI_start()
{
	
	// Start the gui
	
	useGUI = true;
	// Override useGUI default
	getBoolForKey(kGUIKey, &useGUI, &bootInfo->bootConfig);
	if (useGUI && initGUI())
	{
		// initGUI() returned with an error, disabling GUI.
		useGUI = false;
	}
	else
	{
		replace_function("_initGraphicsMode", &GUI_initGraphicsMode);
		replace_function("_getBootOptions", &GUI_getBootOptions);
		replace_function("_clearBootArgs", &GUI_clearBootArgs);
		replace_function("_showHelp", &GUI_showHelp);
		
		replace_function("_printf", &GUI_printf);
		replace_function("_verbose", &GUI_verbose);
		replace_function("_error", &GUI_error);
		replace_function("_stop", &GUI_stop);		
	}
	
	// Hoot for the boot screen 
	//ExecKernel register_hook_callback("Kernel Start", &GUI_Kernel_Start_hook);
	register_hook_callback("ExecKernel", &GUI_ExecKernel_hook);
	register_hook_callback("PreBoot", &GUI_PreBoot_hook);		
	
}

/**
 ** Overriden chameleon function. Draws the updated menu.
 **/
static int GUI_updateMenu( int key, void ** paramPtr )
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
	
	if( bootArgs->Video.v_display == GRAPHICS_MODE )
	{
		int res;
		
		// set navigation keys for horizontal layout as defaults
		int previous	= 0x4B00;		// left arrow
		int subsequent	= 0x4D00;		// right arrow
		int menu		= 0x5000;		// down arrow
		
		if ( gui.layout == VerticalLayout )
		{
			// set navigation keys for vertical layout
			previous	= 0x4800;		// up arrow
			subsequent	= 0x5000;		// down arrow
			menu		= 0x4B00;		// right arrow
		} 
		
		if ( key == previous )
		{
			if ( gMenuSelection > gMenuTop )
				draw.f.selectionUp = 1;
			else if ( gMenuTop > 0 )
				draw.f.scrollDown = 1;
			
		}
		
		else if ( key ==  subsequent )
		{
			if ( gMenuSelection != gMenuBottom)
				draw.f.selectionDown = 1;
			else if ( gMenuBottom < ( gMenuItemCount - 1 ) )
				draw.f.scrollUp = 1;
		}
		
		else if ( key == menu )
		{
			if ( gui.menu.draw )
				updateInfoMenu(key);
			else
				drawInfoMenu();
		}
		
		else if ( gui.menu.draw )
		{
			res = updateInfoMenu(key);
			
			if ( res == CLOSE_INFO_MENU )
				gui.menu.draw = false;
			else
			{
				shouldboot = ( res != DO_NOT_BOOT );
				
				if ( shouldboot )
					gui.menu.draw = false;
				
				switch (res)
				{
					case BOOT_NORMAL:
						gVerboseMode = false;
						gBootMode = kBootModeNormal;
						break;
						
					case BOOT_VERBOSE:
						gVerboseMode = true;
						gBootMode = kBootModeNormal;
						addBootArg(kVerboseModeFlag);
						break;
						
					case BOOT_IGNORECACHE:
						gVerboseMode = false;
						gBootMode = kBootModeNormal;
						addBootArg(kIgnoreCachesFlag);
						break;
						
					case BOOT_SINGLEUSER:
						gVerboseMode = true;
						gBootMode = kBootModeNormal;
						addBootArg(kSingleUserModeFlag);
						break;
				}
				
			}
			
		}	
		
	} else {
		switch ( key )
		{
        	case 0x4800:  // Up Arrow
				if ( gMenuSelection != gMenuTop )
				{
					draw.f.selectionUp = 1;
				}
				else if ( gMenuTop > 0 )
				{
					draw.f.scrollDown = 1;
				}
				break;
				
			case 0x5000:  // Down Arrow
				if ( gMenuSelection != gMenuBottom )
				{
					draw.f.selectionDown = 1;
				}
				else if ( gMenuBottom < (gMenuItemCount - 1) ) 
				{
					draw.f.scrollUp = 1;
				}
				break;
		}
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
			
			if( bootArgs->Video.v_display == VGA_TEXT_MODE )
			{
				changeCursor( 0, (gMenuRow + gMenuSelection - gMenuTop), kCursorTypeHidden, &cursorState );
				printMenuItem( &gMenuItems[gMenuSelection], 0 );
			}
			
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
				if(( gMenuSelection - ( gui.maxdevices - 1) - gMenuStart) > 0 )
				{
					gMenuStart++;
					gMenuEnd++;
				}
			}
			
			if( bootArgs->Video.v_display == VGA_TEXT_MODE )
			{
				moveCursor( 0, gMenuRow + gMenuSelection - gMenuTop );
				printMenuItem( &gMenuItems[gMenuSelection], 1 );
				restoreCursor( &cursorState );
				
			}
			else
			{
				drawDeviceList (gMenuStart, gMenuEnd, gMenuSelection);
			}
			
		}
		
        *paramPtr = gMenuItems[gMenuSelection].param;        
        moved = 1;
    }
	
	return moved;
}


static void GUI_showMenu( const MenuItem * items, int count,
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
    gMenuEnd	    = min( count, gui.maxdevices ) - 1;
	
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
	
	if( bootArgs->Video.v_display == GRAPHICS_MODE )
	{
		drawDeviceList(gMenuStart, gMenuEnd, gMenuSelection);
	}
	else 
	{
		
		changeCursor( 0, row, kCursorTypeHidden, &cursorState );
		
		for ( i = gMenuTop; i <= gMenuBottom; i++ )
		{
			printMenuItem( &items[i], (i == gMenuSelection) );
		}
		
		restoreCursor( &cursorState );
    }
}


static void GUI_updateBootArgs( int key )
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
					x--;
				if( bootArgs->Video.v_display == VGA_TEXT_MODE )
				{
					setCursorPosition( x, y, 0 );
					putca(' ', 0x07, 1);
				} else
				{
					updateGraphicBootPrompt(kBackspaceKey);
				}
				
				*gBootArgsPtr-- = '\0';
			}
            
			break;
			
        default:
            if ( key >= ' ' && gBootArgsPtr < gBootArgsEnd)
            {
				if( bootArgs->Video.v_display == VGA_TEXT_MODE )
				{
					putchar(key);  // echo to screen
				}
				else
				{
					updateGraphicBootPrompt(key);
				}
				*gBootArgsPtr++ = key;
			}
            
			break;
    }
}


static void GUI_showBootPrompt(int row, bool visible)
{
	extern char bootPrompt[];
	
	if( bootArgs->Video.v_display == VGA_TEXT_MODE )
	{
		changeCursor( 0, row, kCursorTypeUnderline, 0 );    
		clearScreenRows( row, kScreenLastRow );
	}
	
	clearBootArgs();
	
	if (visible)
	{
		if (bootArgs->Video.v_display == VGA_TEXT_MODE) 
		{
			if (gEnableCDROMRescan)
			{
				printf( GUI_bootRescanPrompt );
			} 
			else
			{
				printf( bootPrompt );
			}
		}
	} 
	else
	{
		if (bootArgs->Video.v_display == GRAPHICS_MODE)
		{
			clearGraphicBootPrompt();
		} 
		else
		{
			printf("Press Enter to start up the foreign OS. ");
		}
	}
}


void GUI_clearBootArgs(void)
{
	gBootArgsPtr = gBootArgs;
	memset(gBootArgs, '\0', BOOT_STRING_LEN);
	
	if (bootArgs->Video.v_display == GRAPHICS_MODE) 
	{
		clearGraphicBootPrompt();
	}
}


int GUI_getBootOptions(bool firstRun)
{
	int     i;
	int     key;
	int     nextRow;
	int     timeout;
	int     bvCount;
	BVRef   bvr;
	BVRef   menuBVR;
	bool    showPrompt, newShowPrompt, isCDROM;
	
	// Initialize default menu selection entry.
	gBootVolume = menuBVR = selectBootVolume(bvChain);
	
	if (biosDevIsCDROM(gBIOSDev))
	{
		isCDROM = true;
	} 
	else
	{
		isCDROM = false;
	}
	
	// ensure we're in graphics mode if gui is setup
	if (gui.initialised && bootArgs->Video.v_display == VGA_TEXT_MODE)
	{
		setVideoMode(GRAPHICS_MODE, 0);
	}
	
	// Clear command line boot arguments
	clearBootArgs();
	
	// Allow user to override default timeout.
	if (!getIntForKey(kTimeoutKey, &timeout, &bootInfo->bootConfig))
	{
		/*  If there is no timeout key in the file use the default timeout
		 which is different for CDs vs. hard disks.  However, if not booting
		 a CD and no config file could be loaded set the timeout
		 to zero which causes the menu to display immediately.
		 This way, if no partitions can be found, that is the disk is unpartitioned
		 or simply cannot be read) then an empty menu is displayed.
		 If some partitions are found, for example a Windows partition, then
		 these will be displayed in the menu as foreign partitions.
		 */
		if (isCDROM)
		{
			timeout = kCDBootTimeout;
		}
		else 
		{
			timeout = sysConfigValid ? kBootTimeout : 0;
		}
	}
	
	if (timeout < 0) 
	{
		gBootMode |= kBootModeQuiet;
	}
	
	// If the user is holding down a modifier key, enter safe mode.
	if ((readKeyboardShiftFlags() & 0x0F) != 0) 
	{
		
		//gBootMode |= kBootModeSafe;
	}
	
	// Checking user pressed keys
	bool f8press = false, spress = false, vpress = false;
	while (readKeyboardStatus())
	{
		key = bgetc ();
		if (key == 0x4200) f8press = true;
		if ((key & 0xff) == 's' || (key & 0xff) == 'S') spress = true;
		if ((key & 0xff) == 'v' || (key & 0xff) == 'V') vpress = true;
	}
	// If user typed F8, abort quiet mode, and display the menu.
	if (f8press)
	{
		gBootMode &= ~kBootModeQuiet;
		timeout = 0;
	}
	// If user typed 'v' or 'V', boot in verbose mode.
	if ((gBootMode & kBootModeQuiet) && firstRun && vpress)
	{
		addBootArg(kVerboseModeFlag);
	}
	// If user typed 's' or 'S', boot in single user mode.
	if ((gBootMode & kBootModeQuiet) && firstRun && spress)
	{
		addBootArg(kSingleUserModeFlag);
	}
	
	if (bootArgs->Video.v_display == VGA_TEXT_MODE)
	{
		setCursorPosition(0, 0, 0);
		clearScreenRows(0, kScreenLastRow);
		if (!(gBootMode & kBootModeQuiet)) 
		{
			// Display banner and show hardware info.
			printf(bootBanner, (bootInfo->convmem + bootInfo->extmem) / 1024);
			printf(getVBEInfoString());
		}
		changeCursor(0, kMenuTopRow, kCursorTypeUnderline, 0);
		verbose("Scanning device %x...", gBIOSDev);
	}
	
	// When booting from CD, default to hard drive boot when possible. 
	if (isCDROM && firstRun)
	{
		const char *val;
		char *prompt = NULL;
		char *name = NULL;
		int cnt;
		int optionKey;
		
		if (getValueForKey(kCDROMPromptKey, &val, &cnt, &bootInfo->bootConfig))
		{
			prompt = malloc(cnt + 1);
			strncat(prompt, val, cnt);
		}
		else 
		{
			name = malloc(80);
			getBootVolumeDescription(gBootVolume, name, 79, false);
			prompt = malloc(256);
			sprintf(prompt, "Press any key to start up from %s, or press F8 to enter startup options.", name);
			free(name);
		}
		
		if (getIntForKey( kCDROMOptionKey, &optionKey, &bootInfo->bootConfig ))
		{
			// The key specified is a special key.
		}
		else
		{
			// Default to F8.
			optionKey = 0x4200;
		}
		
		// If the timeout is zero then it must have been set above due to the
		// early catch of F8 which means the user wants to set boot options
		// which we ought to interpret as meaning he wants to boot the CD.
		if (timeout != 0) {
			key = GUI_countdown(prompt, kMenuTopRow, timeout);
		} 
		else 
		{
			key = optionKey;
		}
		
		if (prompt != NULL)
		{
			free(prompt);
		}
		
		clearScreenRows( kMenuTopRow, kMenuTopRow + 2 );
		
		// Hit the option key ?
		if (key == optionKey) 
		{
			gBootMode &= ~kBootModeQuiet;
			timeout = 0;
		} 
		else 
		{
			key = key & 0xFF;
			
			// Try booting hard disk if user pressed 'h'
			if (biosDevIsCDROM(gBIOSDev) && key == 'h')
			{
				BVRef bvr;
				
				// Look at partitions hosting OS X other than the CD-ROM
				for (bvr = bvChain; bvr; bvr=bvr->next)
				{
					if ((bvr->flags & kBVFlagSystemVolume) && bvr->biosdev != gBIOSDev)
					{
						gBootVolume = bvr;
					}
				}
			}
			goto done;
		}
	}
	
	if (gBootMode & kBootModeQuiet)
	{
		// No input allowed from user.
		goto done;
	}
	
	if (firstRun && timeout > 0 && GUI_countdown("Press any key to enter startup options.", kMenuTopRow, timeout) == 0)
	{
		// If the user is holding down a modifier key,
		// enter safe mode.
		if ((readKeyboardShiftFlags() & 0x0F) != 0)
		{
			gBootMode |= kBootModeSafe;
		}
		goto done;
	}
	
	if (gDeviceCount)
	{
		// Allocate memory for an array of menu items.
		menuItems = malloc(sizeof(MenuItem) * gDeviceCount);
		if (menuItems == NULL) 
		{
			goto done;
		}
		
		// Associate a menu item for each BVRef.
		for (bvr=bvChain, i=gDeviceCount-1, selectIndex=0; bvr; bvr=bvr->next)
		{
			if (bvr->visible)
			{
				getBootVolumeDescription(bvr, menuItems[i].name, sizeof(menuItems[i].name) - 1, true);
				menuItems[i].param = (void *) bvr;
				if (bvr == menuBVR)
				{
					selectIndex = i;
				}
				i--;
			}
		}
	}
	
	if (bootArgs->Video.v_display == GRAPHICS_MODE)
	{
		// redraw the background buffer
		gui.logo.draw = true;
		drawBackground();
		gui.devicelist.draw = true;
		gui.redraw = true;
		if (!(gBootMode & kBootModeQuiet))
		{
			bool showBootBanner = true;
			
			// Check if "Boot Banner"=N switch is present in config file.
			getBoolForKey(kBootBannerKey, &showBootBanner, &bootInfo->bootConfig); 
			if (showBootBanner)
			{
				// Display banner and show hardware info.
				gprintf(&gui.screen, bootBanner + 1, (bootInfo->convmem + bootInfo->extmem) / 1024);
			}
			
			// redraw background
			memcpy(gui.backbuffer->pixels, gui.screen.pixmap->pixels, gui.backbuffer->width * gui.backbuffer->height * 4);
		}
	}
	else 
	{
		// Clear screen and hide the blinking cursor.
		clearScreenRows(kMenuTopRow, kMenuTopRow + 2);
		changeCursor(0, kMenuTopRow, kCursorTypeHidden, 0);
	}
	
	nextRow = kMenuTopRow;
	showPrompt = true;
	
	if (gDeviceCount)
	{
		if( bootArgs->Video.v_display == VGA_TEXT_MODE )
		{
			printf("Use \30\31 keys to select the startup volume.");
		}
		GUI_showMenu( menuItems, gDeviceCount, selectIndex, kMenuTopRow + 2, kMenuMaxItems );
		nextRow += min( gDeviceCount, kMenuMaxItems ) + 3;
	}
	
	// Show the boot prompt.
	showPrompt = (gDeviceCount == 0) || (menuBVR->flags & kBVFlagNativeBoot);
	GUI_showBootPrompt( nextRow, showPrompt );
	
	do {
		if (bootArgs->Video.v_display == GRAPHICS_MODE)
		{
			// redraw background
			memcpy( gui.backbuffer->pixels, gui.screen.pixmap->pixels, gui.backbuffer->width * gui.backbuffer->height * 4 );
			// reset cursor co-ords
			gui.debug.cursor = pos( gui.screen.width - 160 , 10 );
		}
		key = getc();
		GUI_updateMenu( key, (void **) &menuBVR );
		newShowPrompt = (gDeviceCount == 0) || (menuBVR->flags & kBVFlagNativeBoot);
		
		if (newShowPrompt != showPrompt)
		{
			showPrompt = newShowPrompt;
			GUI_showBootPrompt( nextRow, showPrompt );
		}
		
		if (showPrompt)
		{
			GUI_updateBootArgs(key);
		}
		
		switch (key)
		{
			case kReturnKey:
				if (gui.menu.draw)
				{ 
					key=0;
					break;
				}
				if (*gBootArgs == '?')
				{
					char * argPtr = gBootArgs;
					
					// Skip the leading "?" character.
					argPtr++;
					getNextArg(&argPtr, booterCommand);
					getNextArg(&argPtr, booterParam);
					
					/*
					 * TODO: this needs to be refactored.
					 */
					if (strcmp( booterCommand, "video" ) == 0)
					{
						if (bootArgs->Video.v_display == GRAPHICS_MODE)
						{
							showInfoBox(getVBEInfoString(), getVBEModeInfoString());
						}
						else
						{
							printVBEModeInfo();
						}
					}
					else if ( strcmp( booterCommand, "memory" ) == 0)
					{
						if (bootArgs->Video.v_display == GRAPHICS_MODE ) 
						{
							showInfoBox("Memory Map", getMemoryInfoString());
						}
						else
						{
							printMemoryInfo();
						}
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
						processRAMDiskCommand(&argPtr, booterParam);
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
					key = 0;
					GUI_showBootPrompt(nextRow, showPrompt);
					break;
				}
				gBootVolume = menuBVR;
				setRootVolume(menuBVR);
				gBIOSDev = menuBVR->biosdev;
				break;
				
			case kEscapeKey:
				clearBootArgs();
				break;
				
			case kF5Key:
				// New behavior:
				// Clear gBootVolume to restart the loop
				// if the user enabled rescanning the optical drive.
				// Otherwise boot the default boot volume.
				if (gEnableCDROMRescan)
				{
					gBootVolume = NULL;
					clearBootArgs();
				}
				break;
				
			case kF10Key:
				gScanSingleDrive = false;
				scanDisks(gBIOSDev, &bvCount);
				gBootVolume = NULL;
				clearBootArgs();
				break;
				
			case kTabKey:
				// New behavior:
				// Switch between text & graphic interfaces
				// Only Permitted if started in graphics interface
				if (useGUI)
				{
					if (bootArgs->Video.v_display == GRAPHICS_MODE)
					{
						setVideoMode(VGA_TEXT_MODE, 0);
						
						setCursorPosition(0, 0, 0);
						clearScreenRows(0, kScreenLastRow);
						
						// Display banner and show hardware info.
						printf(bootBanner, (bootInfo->convmem + bootInfo->extmem) / 1024);
						printf(getVBEInfoString());
						
						clearScreenRows(kMenuTopRow, kMenuTopRow + 2);
						changeCursor(0, kMenuTopRow, kCursorTypeHidden, 0);
						
						nextRow = kMenuTopRow;
						showPrompt = true;
						
						if (gDeviceCount)
						{
							printf("Use \30\31 keys to select the startup volume.");
							GUI_showMenu(menuItems, gDeviceCount, selectIndex, kMenuTopRow + 2, kMenuMaxItems);
							nextRow += min(gDeviceCount, kMenuMaxItems) + 3;
						}
						
						showPrompt = (gDeviceCount == 0) || (menuBVR->flags & kBVFlagNativeBoot);
						GUI_showBootPrompt(nextRow, showPrompt);
						//changeCursor( 0, kMenuTopRow, kCursorTypeUnderline, 0 );
					} 
					else 
					{
						gui.redraw = true;
						setVideoMode(GRAPHICS_MODE, 0);
						updateVRAM();
					}
				}
				key = 0;
				break;
				
			default:
				key = 0;
				break;
		}
	} while (0 == key);
	
done:
	if (bootArgs->Video.v_display == VGA_TEXT_MODE)
	{
		clearScreenRows(kMenuTopRow, kScreenLastRow);
		changeCursor(0, kMenuTopRow, kCursorTypeUnderline, 0);
	}
	shouldboot = false;
	gui.menu.draw = false;
	if (menuItems)
	{
		free(menuItems);
		menuItems = NULL;
	}
	return 0;
}



int GUI_error(const char * fmt, ...)
{
    va_list ap;
    gErrors = true;
    va_start(ap, fmt);
	
	if (bootArgs->Video.v_display == VGA_TEXT_MODE)
	{
		prf(fmt, ap, putchar, 0);
    }
	else
	{
		vprf(fmt, ap);
	}
	
	va_end(ap);
    return(0);
}

int GUI_verbose(const char * fmt, ...)
{
    va_list ap;
    
	va_start(ap, fmt);
    if (gVerboseMode)
    {
		if (bootArgs->Video.v_display == VGA_TEXT_MODE)
		{
			prf(fmt, ap, putchar, 0);
		}
		else
		{
			vprf(fmt, ap);
		}
    }
	
	/* Kabyl: BooterLog */
	struct putc_info pi;
	
	if (!msgbuf)
		return 0;
	
	if (((cursor - msgbuf) > (BOOTER_LOG_SIZE - SAFE_LOG_SIZE)))
		return 0;
	pi.str = cursor;
	pi.last_str = 0;
	prf(fmt, ap, sputc, &pi);
	cursor +=  strlen((char *)cursor);
	
	
    va_end(ap);
    return(0);
}

int GUI_printf(const char * fmt, ...)
{
    va_list ap;
	va_start(ap, fmt);
	if (bootArgs->Video.v_display == VGA_TEXT_MODE)
	{
		prf(fmt, ap, putchar, 0);
	}
	else
	{
		vprf(fmt, ap);
	}
	
	/* Kabyl: BooterLog */
	struct putc_info pi;
	
	if (!msgbuf)
		return 0;
	
	if (((cursor - msgbuf) > (BOOTER_LOG_SIZE - SAFE_LOG_SIZE)))
		return 0;
	pi.str = cursor;
	pi.last_str = 0;
	prf(fmt, ap, sputc, &pi);
	cursor +=  strlen((char *)cursor);
	
	va_end(ap);
    return 0;
}

void GUI_stop(const char * fmt, ...)
{
	va_list ap;
	
	printf("\n");
	va_start(ap, fmt);
	
	if (bootArgs->Video.v_display == VGA_TEXT_MODE)
	{
		prf(fmt, ap, putchar, 0);
	} 
	else 
	{
		vprf(fmt, ap);
	}
	va_end(ap);
	
	printf("\nThis is a non recoverable error! System HALTED!!!");
	halt();
	while (1);
}

void GUI_showHelp(void)
{
	if (bootArgs->Video.v_display == GRAPHICS_MODE) {
		showInfoBox("Help. Press q to quit.\n", (char *)BootHelp_txt);
	} else {
		showTextBuffer((char *)BootHelp_txt, BootHelp_txt_len);
	}
}
