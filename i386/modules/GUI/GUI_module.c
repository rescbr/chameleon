/*
 * Copyright (c) 2009 Evan Lojewski. All rights reserved.
 *
 */

/*
 * cparm : cleaned
 */


#include "libsaio.h"
#include "platform.h"
#include "graphic_utils.h"
#include "embedded.h"

#include "picopng.h"
#include "gui.h"

#include "modules.h"

enum {
    kMenuTopRow    = 5,
    kMenuMaxItems  = 10,
    kScreenLastRow = 24
};

enum {
	kBackspaceKey	= 0x08,
	kTabKey			= 0x09,
	kReturnKey		= 0x0d,
	kEscapeKey		= 0x1b,
	kUpArrowkey		= 0x4800, 
	kDownArrowkey	= 0x5000,
	kASCIIKeyMask	= 0x7f,
	kF5Key			= 0x3f00,
	kF10Key			= 0x4400
};
/*
 * Flags to the booter or kernel
 */
#define kVerboseModeFlag	"-v"
#define kSafeModeFlag		"-x"
#define kIgnoreCachesFlag	"-f"
#define kSingleUserModeFlag	"-s"
#define kIgnorePrelinkKern  "-F"
#define kIgnoreBootFileFlag	"-B"

/*
 * Booter behavior control
 */
#define kBootTimeout         -1
#define kCDBootTimeout       8

/*
 * A global set by boot() to record the device that the booter
 * was loaded from.
 */
#define Cache_len_name 512

/*
 * Boot Modes
 */
enum {
    kBootModeNormal = 0,
    kBootModeSafe   = 1,
    kBootModeSecure = 2,
    kBootModeQuiet  = 4
};
typedef struct {
    int x;
    int y;
    int type;
} CursorState;

/* Kabyl: BooterLog */
#define BOOTER_LOG_SIZE    (128 * 1024)
#define SAFE_LOG_SIZE    134

static bool useGUI = true;
static int			selectIndex = 0;

void GUI_Kernel_Start_hook(void* kernelEntry, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6);
void GUI_PreBoot_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6);

int GUI_getBootOptions(bool firstRun);

static void GUI_updateBootArgs( int key );
void GUI_clearBootArgs(void);
void GUI_addBootArg(const char * argStr);

static void GUI_showBootPrompt(int row, bool visible);
static void GUI_showMenu( const MenuItem * items, int count, int selection, int row, int height );
static int GUI_updateMenu( int key, void ** paramPtr );
static void GUI_showHelp(void);
static void GUI_showMessage(char *message);

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

static void (*showTextBuffer)(char *, int ) = NULL;
static char *(*getMemoryInfoString)(void) = NULL;
static BVRef (*getBvChain)(void) = NULL;
static void (*lspci)(void) = NULL;
static void (*printMemoryInfo)(void) = NULL;
static void (*showHelp)(void) = NULL;
static void (*showTextFile)(const char * ) = NULL;
static void (*showMessage)(char * ) = NULL;

static const MenuItem * gMenuItems = NULL;
static MenuItem *  menuItems = NULL;

static char   gBootArgs[BOOT_STRING_LEN];
static char * gBootArgsPtr = gBootArgs;
static char * gBootArgsEnd = gBootArgs + BOOT_STRING_LEN - 1;
static char   booterCommand[BOOT_STRING_LEN];
static char   booterParam[BOOT_STRING_LEN];

static void printMenuItem( const MenuItem * item, int highlight )
{
    printf("  ");
	
    if ( highlight )
        putca(' ', 0x70, strlen(item->name) + 4);
    else
        putca(' ', 0x07, 40);
	
    printf("  %40s\n", item->name);
}
//==========================================================================

static void changeCursor( int col, int row, int type, CursorState * cs )
{
    if (cs) getCursorPositionAndType( &cs->x, &cs->y, &cs->type );
    setCursorType( type );
    setCursorPosition( col, row, 0 );
}

static void moveCursor( int col, int row )
{
    setCursorPosition( col, row, 0 );
}

static int restoreCursor( const CursorState * cs )
{
    if (!cs) {
        return 0;
    }
    
    setCursorPosition( cs->x, cs->y, 0 );
    setCursorType( cs->type );
    
    return 1;
}

char GUI_bootRescanPrompt[] =
"Press Enter to start up Darwin/x86 with no options, or you can:\n"
"  Press F5 after you swapped the media. The drive will be rescanned.\n"
"  Type -v and press Enter to start up with diagnostic messages\n"
"  Type ? and press Enter to learn about advanced startup options\n\n"
"boot: ";


static bool KernelStart = false;
/**
 ** The kernel is about to start, draw the boot graphics if we are not in
 ** verbose mode.
 **/
void GUI_ExecKernel_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6)
{
	if(!gVerboseMode)
	{
		// Note: shouldn't be needed, but just in case
		drawBootGraphics();
	}
	else
	{
#if UNUSED
		setVideoMode(GRAPHICS_MODE, 0);
#else
		setVideoMode(GRAPHICS_MODE);
#endif
		
	}
	KernelStart = true;
}

/**
 ** A boot option has been selected, disable the graphical elements on screen.
 **/
void GUI_PreBoot_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6)
{		
	// Turn off any GUI elements
	if( getVideoMode() == GRAPHICS_MODE )
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
			replace_system_function("_printf", &GUI_verbose);
			//drawBootGraphics();
		}
		
	}	
    
}

void GUI_diplay_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6)
{
	// Start and display the gui
	msglog("* Attempting to Display GUI\n");
    
	if (initGUI())
	{
		
		useGUI = false; // initGUI() returned with an error, disabling GUI.		
		msglog("* GUI failed to Display, or disabled by user (a.k.a you)\n");
        
		
	}
	else
	{
		//replace_system_function("_initGraphicsMode", &GUI_initGraphicsMode);
		replace_system_function("_getBootOptions", &GUI_getBootOptions);
		replace_system_function("_clearBootArgs", &GUI_clearBootArgs);
        replace_system_function("_addBootArg", &GUI_addBootArg);  
        
        replace_system_function("_showHelp", &GUI_showHelp);
		
		replace_system_function("_printf", &GUI_printf);
		replace_system_function("_verbose", &GUI_verbose);
		replace_system_function("_error", &GUI_error);
		replace_system_function("_stop", &GUI_stop);
		replace_system_function("_showMessage", &GUI_showMessage);
        
		
		// Hook for the boot screen 	
		register_hook_callback("GUI_ExecKernel", &GUI_ExecKernel_hook);
		register_hook_callback("GUI_PreBoot", &GUI_PreBoot_hook);
		
        
        getBvChain = (void*)lookup_all_symbols(SYMBOLS_BUNDLE,"_getBvChain");
        getMemoryInfoString = (void*)lookup_all_symbols(SYMBOLS_BUNDLE,"_getMemoryInfoString");
        
        printMemoryInfo = (void*)lookup_all_symbols(SYMBOLS_BUNDLE,"_printMemoryInfo");
        lspci = (void*)lookup_all_symbols(SYMBOLS_BUNDLE,"_lspci");        
        showHelp = (void*)lookup_all_symbols(SYMBOLS_BUNDLE,"_showHelp");
        showTextFile = (void*)lookup_all_symbols(SYMBOLS_BUNDLE,"_showTextFile");
        showMessage = (void*)lookup_all_symbols(SYMBOLS_BUNDLE,"_showMessage");        
        showTextBuffer = (void*)lookup_all_symbols(SYMBOLS_BUNDLE,"_showTextBuffer");
		safe_set_env(envgBootArgs,(uint32_t)gBootArgs);
        
		msglog("* GUI successfully Displayed\n");
		
	}
}

/**
 ** Module startup code. Replace console only print functions as well as
 ** replace various menu functions. Finaly, initialize the gui and hook
 ** into important events.
 **/
void GUI_start(void);
void GUI_start(void)
{
	register_hook_callback("GUI_Display", &GUI_diplay_hook);				
	
}

/**
 ** Overriden chameleon function. Draws the updated menu.
 **/
static int GUI_updateMenu( int key, void ** paramPtr )
{
    int moved = 0;
    
    int MenuTop = (int)get_env(envgMenuTop);
    int MenuSelection = (int)get_env(envgMenuSelection);
    int MenuRow = (int)get_env(envgMenuRow);
    int MenuHeight = (int)get_env(envgMenuHeight);
    int MenuBottom = (int)get_env(envgMenuBottom);
    int MenuStart = (int)get_env(envgMenuStart);
    int MenuEnd = (int)get_env(envgMenuEnd);
	
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
	
	if( getVideoMode() == GRAPHICS_MODE )
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
			if ( MenuSelection > MenuTop )
				draw.f.selectionUp = 1;
			else if ( MenuTop > 0 )
				draw.f.scrollDown = 1;
			
		}
		
		else if ( key ==  subsequent )
		{
			if ( MenuSelection != MenuBottom)
				draw.f.selectionDown = 1;
			else if ( MenuBottom < ( get_env(envgMenuItemCount) - 1 ) )
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
				bool shouldboot = ( res != DO_NOT_BOOT );
                safe_set_env(envShouldboot, shouldboot);
                
				if ( shouldboot )
					gui.menu.draw = false;
				
				switch (res)
				{
					case BOOT_NORMAL:
						gVerboseMode = false;
                        safe_set_env(envgBootMode, kBootModeNormal);
                        
						break;
						
					case BOOT_VERBOSE:
						gVerboseMode = true;
                        safe_set_env(envgBootMode, kBootModeNormal);
						GUI_addBootArg(kVerboseModeFlag);
						break;
						
					case BOOT_IGNORECACHE:
						gVerboseMode = false;
                        safe_set_env(envgBootMode, kBootModeNormal);
						GUI_addBootArg(kIgnoreCachesFlag);
						break;
						
					case BOOT_SINGLEUSER:
						gVerboseMode = true;
                        safe_set_env(envgBootMode, kBootModeNormal);
						GUI_addBootArg(kSingleUserModeFlag);
						break;
					default:
						break;
				}
				
			}
			
		}	
		
	} else {
		switch ( key )
		{
        	case 0x4800:  // Up Arrow
				if ( MenuSelection != MenuTop )
				{
					draw.f.selectionUp = 1;
				}
				else if ( MenuTop > 0 )
				{
					draw.f.scrollDown = 1;
				}
				break;
				
			case 0x5000:  // Down Arrow
				if ( MenuSelection != MenuBottom )
				{
					draw.f.selectionDown = 1;
				}
				else if ( MenuBottom < (get_env(envgMenuItemCount) - 1) ) 
				{
					draw.f.scrollUp = 1;
				}
				break;
			default:
				break;
		}
	}
	
    if ( draw.w )
    {
        if ( draw.f.scrollUp )
        {
            scollPage(0, MenuRow, 40, MenuRow + MenuHeight - 1, 0x07, 1, 1);
            MenuTop++; MenuBottom++;
			MenuStart++; MenuEnd++;
            draw.f.selectionDown = 1;
        }
		
        if ( draw.f.scrollDown )
        {
            scollPage(0, MenuRow, 40, MenuRow + MenuHeight - 1, 0x07, 1, -1);
            MenuTop--; MenuBottom--;
            MenuStart--; MenuEnd--;
            draw.f.selectionUp = 1;
        }
		
        if ( draw.f.selectionUp || draw.f.selectionDown )
        {
			
			CursorState cursorState;
			cursorState.x = cursorState.y = cursorState.type=0;
			
			// Set cursor at current position, and clear inverse video.
			
			if( getVideoMode() == VGA_TEXT_MODE )
			{
				changeCursor( 0, (MenuRow + MenuSelection - MenuTop), kCursorTypeHidden, &cursorState );
				printMenuItem( &gMenuItems[MenuSelection], 0 );
			}
			
			if ( draw.f.selectionUp )
			{
				MenuSelection--;
				if(( MenuSelection - MenuStart) == -1 )
				{
					MenuStart--;
					MenuEnd--;
				}
				
			} else {
				MenuSelection++;
				if(( MenuSelection - ( gui.maxdevices - 1) - MenuStart) > 0 )
				{
					MenuStart++;
					MenuEnd++;
				}
			}
			
			if( getVideoMode() == VGA_TEXT_MODE )
			{
				moveCursor( 0, MenuRow + MenuSelection - MenuTop );
				printMenuItem( &gMenuItems[MenuSelection], 1 );
				/*moved =*/ restoreCursor( &cursorState );
				
			}
			else
			{
				drawDeviceList (MenuStart, MenuEnd, MenuSelection, menuItems);
			}
			
		}
		
        *paramPtr = gMenuItems[MenuSelection].param;        
        moved = 1;
    }
    
	safe_set_env(envgMenuSelection,MenuSelection);
    safe_set_env(envgMenuTop,MenuTop );
    safe_set_env(envgMenuRow,MenuRow);
    safe_set_env(envgMenuHeight,MenuHeight);
    safe_set_env(envgMenuBottom,MenuBottom);
    safe_set_env(envgMenuStart,MenuStart);
    safe_set_env(envgMenuEnd,MenuEnd);
    
	return moved;
}


static void GUI_showMenu( const MenuItem * items, int count,
						 int selection, int row, int height )
{
    int         i;
    CursorState cursorState;
	cursorState.x = cursorState.y = cursorState.type=0;

    if ( items == NULL || count == 0 ) 
		return;
	
    // head and tail points to the start and the end of the list.
    // top and bottom points to the first and last visible items
    // in the menu window.
	
    gMenuItems		= items;
    int MenuTop		= 0;
    int MenuBottom		= min( count, height ) - 1;
    int MenuSelection	= selection;	
    int MenuStart		= 0;
    int MenuEnd	    = count; //min( count, gui.maxdevices ) - 1;
	
	// If the selected item is not visible, shift the list down.
	
    if ( MenuSelection > MenuBottom )
    {
        MenuTop += ( MenuSelection - MenuBottom );
        MenuBottom = MenuSelection;
    }
	
	if ( MenuSelection > MenuEnd )
    {
		MenuStart += ( MenuSelection - MenuEnd );
        MenuEnd = MenuSelection;
    }
	
	// Draw the visible items.
	
	if( getVideoMode() == GRAPHICS_MODE )
	{
		drawDeviceList(MenuStart, MenuEnd, MenuSelection, menuItems);
	}
	else 
	{
		
		changeCursor( 0, row, kCursorTypeHidden, &cursorState );
		
		for ( i = MenuTop; i <= MenuBottom; i++ )
		{
			printMenuItem( &items[i], (i == MenuSelection) );
		}
		
		restoreCursor( &cursorState ); // FIXME : handle the return error
    }
    
    safe_set_env(envgMenuRow,row);
    safe_set_env(envgMenuHeight,height);
    safe_set_env(envgMenuItemCount,count);
    safe_set_env(envgMenuTop,MenuTop);
    safe_set_env(envgMenuBottom,MenuBottom);
    safe_set_env(envgMenuSelection,MenuSelection);
    safe_set_env(envgMenuStart,MenuStart);
    safe_set_env(envgMenuEnd,MenuEnd);
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
				if( getVideoMode() == VGA_TEXT_MODE )
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
				if( getVideoMode() == VGA_TEXT_MODE )
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
	char * bootPrompt = (char*)(uint32_t)get_env(envBootPrompt);
	
	if( getVideoMode() == VGA_TEXT_MODE )
	{
		changeCursor( 0, row, kCursorTypeUnderline, 0 );    
		clearScreenRows( row, kScreenLastRow );
	}
	
	GUI_clearBootArgs();
	
	if (visible)
	{
		if (getVideoMode() == VGA_TEXT_MODE) 
		{
			if (get_env(envgEnableCDROMRescan))
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
		if (getVideoMode() == GRAPHICS_MODE)
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
	
	if (getVideoMode() == GRAPHICS_MODE) 
	{
		clearGraphicBootPrompt();
	}
}

void GUI_addBootArg(const char * argStr)
{
	if ( (gBootArgsPtr + strlen(argStr) + 1) < gBootArgsEnd)
	{
		*gBootArgsPtr++ = ' ';
		strcat(gBootArgs, argStr);
		gBootArgsPtr += strlen(argStr);
	}
}

int GUI_getBootOptions(bool firstRun)
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
    int     optionKey;
    int devcnt = (int)get_env(envgDeviceCount);
    
	// Initialize default menu selection entry.
	gBootVolume = menuBVR = selectBootVolume(getBvChain());
	
	if (biosDevIsCDROM((int)get_env(envgBIOSDev)))
	{
		isCDROM = true;
	} 
	else
	{
		isCDROM = false;
	}
    
	// ensure we're in graphics mode if gui is setup
	if (gui.initialised && (getVideoMode() == VGA_TEXT_MODE))
	{
#if UNUSED
		setVideoMode(GRAPHICS_MODE, 0);
#else
		setVideoMode(GRAPHICS_MODE);
#endif
	}
	
	// Clear command line boot arguments
	GUI_clearBootArgs();
	
	// Allow user to override default timeout.
	if (!getIntForKey(kTimeoutKey, &timeout, DEFAULT_BOOT_CONFIG))
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
			timeout = get_env(envSysConfigValid) ? kBootTimeout : 0;
		}
	}
    
	long gBootMode = get_env(envgBootMode);
    
	if (timeout < 0) 
	{
		gBootMode |= kBootModeQuiet;
        safe_set_env(envgBootMode,gBootMode);
        
	}
	
	// If the user is holding down a modifier key, enter safe mode.
	if ((readKeyboardShiftFlags() & 0x0F) != 0) 
	{
		
		gBootMode |= kBootModeSafe;
        safe_set_env(envgBootMode,gBootMode);
        
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
        safe_set_env(envgBootMode,gBootMode);
        
		timeout = 0;
	}
	// If user typed 'v' or 'V', boot in verbose mode.
	if ((gBootMode & kBootModeQuiet) && firstRun && vpress)
	{
		GUI_addBootArg(kVerboseModeFlag);
	}
	// If user typed 's' or 'S', boot in single user mode.
	if ((gBootMode & kBootModeQuiet) && firstRun && spress)
	{
		GUI_addBootArg(kSingleUserModeFlag);
	}
	
	if (getVideoMode() == VGA_TEXT_MODE)
	{
		setCursorPosition(0, 0, 0);
		clearScreenRows(0, kScreenLastRow);
		if (!(gBootMode & kBootModeQuiet)) 
		{
            char * bootBanner = (char*)(uint32_t)get_env(envBootBanner);
			// Display banner and show hardware info.
			printf(bootBanner, (int)(get_env(envConvMem) + get_env(envExtMem)) / 1024);
			printf(getVBEInfoString());
		}
		changeCursor(0, kMenuTopRow, kCursorTypeUnderline, 0);
		msglog("Scanning device %x...", (uint32_t)get_env(envgBIOSDev));
	}
	
	// When booting from CD, default to hard drive boot when possible. 
	if (isCDROM && firstRun)
	{
		const char *val;
		char *prompt = NULL;
		char *name = NULL;
		int cnt;
		
		if (getValueForKey(kCDROMPromptKey, &val, &cnt, DEFAULT_BOOT_CONFIG)) {
			prompt = malloc(cnt + 1);
            if (!prompt) {
                stop("Couldn't allocate memory for the prompt\n"); //TODO: Find a better stategie
                return -1;
            }
			strncat(prompt, val, cnt);
		} else {
			name = malloc(80);
            if (!name) {
                stop("Couldn't allocate memory for the device name\n"); //TODO: Find a better stategie
                return -1;
            }
			getBootVolumeDescription(gBootVolume, name, 79, false);
			prompt = malloc(256);            
            if (!prompt) {
                free(name);
                stop("Couldn't allocate memory for the prompt\n"); //TODO: Find a better stategie
                return -1;
            }
			sprintf(prompt, "Press ENTER to start up from %s, or press any key to enter startup options.", name);
			free(name);
		}
		
		if (getIntForKey( kCDROMOptionKey, &optionKey, DEFAULT_BOOT_CONFIG ))
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
			key = GUI_countdown(prompt, kMenuTopRow, timeout, &optionKey);
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
		
		do {
            // Hit the option key ?
            if (key == optionKey) {
                
                if (key != 0x1C0D) {
                    gBootMode &= ~kBootModeQuiet;
                    safe_set_env(envgBootMode,gBootMode);
                    timeout = 0;
                    break;
                }
                
            } 
            
            key = key & 0xFF;
            
            // Try booting hard disk if user pressed 'h'
            if (biosDevIsCDROM((int)get_env(envgBIOSDev)) && key == 'h') {
                BVRef bvr;
                
                // Look at partitions hosting OS X other than the CD-ROM
                for (bvr = getBvChain(); bvr; bvr=bvr->next) {
                    if ((bvr->flags & kBVFlagSystemVolume) && bvr->biosdev != (int)get_env(envgBIOSDev)) {
                        gBootVolume = bvr;
                    }
                }
            }
            goto done;
            
        } while (0);
	}
	
	if (gBootMode & kBootModeQuiet)
	{
		// No input allowed from user.
		goto done;
	}
	
	if (firstRun && timeout > 0 ) {
        
        key = GUI_countdown("Press ENTER to start up, or press any key to enter startup options.", kMenuTopRow, timeout, &optionKey);
        
        if (key == 0x1C0D) {
            goto done;
            
        } 
        else if (key == 0)
        {
            // If the user is holding down a modifier key,
            // enter safe mode.     
            
            if ((readKeyboardShiftFlags() & 0x0F) != 0) {
                gBootMode |= kBootModeSafe;
                safe_set_env(envgBootMode,gBootMode);
            }
            goto done;
        }
	}
    
	if (devcnt)
	{
		// Allocate memory for an array of menu items.
		menuItems = malloc(sizeof(MenuItem) * devcnt);
		if (menuItems == NULL) 
		{
			goto done;
		}
		
		// Associate a menu item for each BVRef.
		for (bvr=getBvChain(), i=devcnt-1, selectIndex=0; bvr; bvr=bvr->next)
		{
			if (bvr->visible)
			{
				getBootVolumeDescription(bvr, menuItems[i].name, sizeof(menuItems[i].name) - 1, true);
				menuItems[i].param = (void *) bvr;
				if (bvr == menuBVR)
				{
					selectIndex = i;
                    safe_set_env(envSelectIndex, selectIndex);
				}
				i--;
			}
		}
	}
	
	if (getVideoMode() == GRAPHICS_MODE)
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
			getBoolForKey(kBootBannerKey, &showBootBanner, DEFAULT_BOOT_CONFIG); 
			if (showBootBanner)
			{
                char * bootBanner = (char*)(uint32_t)get_env(envBootBanner);
                
				// Display banner and show hardware info.
				gprintf(&gui.screen, bootBanner + 1, (int)(get_env(envConvMem) + get_env(envExtMem)) / 1024);
                
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
	
	if (devcnt)
	{
		if( getVideoMode() == VGA_TEXT_MODE )
		{
			printf("Use \30\31 keys to select the startup volume.");
		}
		GUI_showMenu( menuItems, devcnt, selectIndex, kMenuTopRow + 2, kMenuMaxItems );
		nextRow += min( devcnt, kMenuMaxItems ) + 3;
	}
	
	// Show the boot prompt.
	showPrompt = (devcnt == 0) || (menuBVR->flags & kBVFlagNativeBoot);
	GUI_showBootPrompt( nextRow, showPrompt );
	
	do {
		if (getVideoMode() == GRAPHICS_MODE)
		{
			// redraw background
			memcpy( gui.backbuffer->pixels, gui.screen.pixmap->pixels, gui.backbuffer->width * gui.backbuffer->height * 4 );
			// reset cursor co-ords
			gui.debug.cursor = pos( gui.screen.width - 160 , 10 );
		}
		key = getc();
		GUI_updateMenu( key, (void **) &menuBVR );
		newShowPrompt = (devcnt == 0) || (menuBVR->flags & kBVFlagNativeBoot);
		
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
						if (getVideoMode() == GRAPHICS_MODE)
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
						if (getVideoMode() == GRAPHICS_MODE ) 
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
						if (execute_hook("processRAMDiskCommand", (void*)argPtr, &booterParam, NULL, NULL, NULL, NULL) != EFI_SUCCESS)
							showMessage("ramdisk module not found, please install RamdiskLoader.dylib in /Extra/modules/");
					} 
					else if (strcmp(booterCommand, "norescan") == 0)
					{
						if (get_env(envgEnableCDROMRescan))
                        {
                            safe_set_env(envgEnableCDROMRescan,false);
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
                safe_set_env(envgBIOSDev,menuBVR->biosdev);
				break;
				
			case kEscapeKey:
				GUI_clearBootArgs();
				break;
				
			case kF5Key:
				// New behavior:
				// Clear gBootVolume to restart the loop
				// if the user enabled rescanning the optical drive.
				// Otherwise boot the default boot volume.
				if (get_env(envgEnableCDROMRescan)) {
					gBootVolume = NULL;
					GUI_clearBootArgs();
				}
				break;
				
			case kF10Key:
                safe_set_env(envgScanSingleDrive, false);
#if UNUSED
                scanDisks((int)get_env(envgBIOSDev), &bvCount);
#else
                scanDisks();
#endif
				gBootVolume = NULL;
				GUI_clearBootArgs();
				break;
				
			case kTabKey:
				// New behavior:
				// Switch between text & graphic interfaces
				// Only Permitted if started in graphics interface
				if (useGUI)
				{
					if (getVideoMode() == GRAPHICS_MODE)
					{
#if UNUSED
						setVideoMode(VGA_TEXT_MODE, 0);
#else
						setVideoMode(VGA_TEXT_MODE);
#endif					
						setCursorPosition(0, 0, 0);
						clearScreenRows(0, kScreenLastRow);
                        
						char * bootBanner = (char*)(uint32_t)get_env(envBootBanner);
                        
						// Display banner and show hardware info.
						printf(bootBanner, (int)(get_env(envConvMem) + get_env(envExtMem)) / 1024);
						printf(getVBEInfoString());
						
						clearScreenRows(kMenuTopRow, kMenuTopRow + 2);
						changeCursor(0, kMenuTopRow, kCursorTypeHidden, 0);
						
						nextRow = kMenuTopRow;
						
						if (devcnt)
						{
							printf("Use \30\31 keys to select the startup volume.");
							GUI_showMenu(menuItems, devcnt, selectIndex, kMenuTopRow + 2, kMenuMaxItems);
							nextRow += min(devcnt, kMenuMaxItems) + 3;
						}
						
						showPrompt = (devcnt == 0) || (menuBVR->flags & kBVFlagNativeBoot);
						GUI_showBootPrompt(nextRow, showPrompt);
						//changeCursor( 0, kMenuTopRow, kCursorTypeUnderline, 0 );
					} 
					else 
					{
						gui.redraw = true;
#if UNUSED
						setVideoMode(GRAPHICS_MODE, 0);
#else
						setVideoMode(GRAPHICS_MODE);
#endif
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
	if (getVideoMode() == VGA_TEXT_MODE)
	{
		clearScreenRows(kMenuTopRow, kScreenLastRow);
		changeCursor(0, kMenuTopRow, kCursorTypeUnderline, 0);
	}
    safe_set_env(envShouldboot, false);
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
	
	if (getVideoMode() == VGA_TEXT_MODE)
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
	
    if (gVerboseMode && (KernelStart == false))
    {
		if (getVideoMode() == VGA_TEXT_MODE)
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
	
	if (KernelStart == false) {
		
		if (getVideoMode() == VGA_TEXT_MODE)
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
    return 0;
}

void GUI_stop(const char * fmt, ...)
{
	va_list ap;
	
	printf("\n");
	va_start(ap, fmt);
	
	if (getVideoMode() == VGA_TEXT_MODE)
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
	if (getVideoMode() == GRAPHICS_MODE) {
		showInfoBox("Help. Press q to quit.\n", (char *)BootHelp_txt);
	} else {
		showTextBuffer((char *)BootHelp_txt, BootHelp_txt_len);
	}
}

void GUI_showMessage(char *message)
{
	if (getVideoMode() == GRAPHICS_MODE) {
		showInfoBox("Help. Press q to quit.\n", message);
	} else {
		showTextBuffer(message, strlen(message));
	}
}