/*
 *  gui.h
 *  
 *
 *  Created by Jasmin Fazlic on 18.12.08.
 *  Copyright 2008/09 Jasmin Fazlic All rights reserved.
 *  Copyright 2008/09 iNDi All rights reserved.
 *
 */

/*
 * cparm : cleaned
 */
#ifndef __BOOT2_GUI_H
#define __BOOT2_GUI_H

#include "libsaio.h"
#include "bootstruct.h"
#include "graphic_utils.h"
#include "picopng.h"
//#include "options.h"

int GUI_initGraphicsMode (void);
int GUI_countdown( const char * msg, int row, int timeout , int *optionKey);

#define kGUIKey				"GUI"
#define kBootBannerKey		"Boot Banner"
#define envSelectIndex      "GUISelIdx"

#define CHARACTERS_COUNT	223

#define BOOT_NORMAL		0
#define BOOT_VERBOSE		1
#define BOOT_IGNORECACHE	2
#define BOOT_SINGLEUSER		3
#define DO_NOT_BOOT		4
#define CLOSE_INFO_MENU		5

#define INFOMENU_NATIVEBOOT_START 1
#define INFOMENU_NATIVEBOOT_END	3

#define MENU_SHOW_MEMORY_INFO	4
#define MENU_SHOW_VIDEO_INFO	5
#define MENU_SHOW_HELP		6

typedef struct {
    char   name[80];
    void * param;
} MenuItem;


enum {
	HorizontalLayout	= 0,
	VerticalLayout		= 1,
};

typedef struct themeList_t
{
	char* theme;
	unsigned char nb;
	struct themeList_t* next;
} themeList_t;

/*
 * Menu item structure.
 */

typedef struct {
	position_t	pos;
	char		*text;
	bool		enabled;
	bool		expandable;
} menuitem_t;

/*
 * Image structure.
 */
typedef struct {
	pixmap_t	*image;
	char		name[32];
} image_t;

/*
 * Font structure.
 */
typedef struct {
	uint16_t	height;			// Font Height 
	uint16_t	width;			// Font Width for monospace font only
	pixmap_t	*chars[CHARACTERS_COUNT];
	uint16_t    count;          // Number of chars in font
} font_t;

/*
 * Window structure.
 */
typedef struct
{
	position_t	pos;			// X,Y Position of window on screen
	pixmap_t	*pixmap;		// Buffer
	uint16_t	width;			// Width
	uint16_t	height;			// Height
	uint16_t	hborder;		// Horizontal border
	uint16_t	vborder;		// Vertical border
	uint16_t	iconspacing;		// Icon spacing
	position_t	cursor;			// Text Cursor X,Y Position will be multiples of font width & height
	uint32_t	bgcolor;		// Background color AARRGGBB
	uint32_t	fgcolor;		// Foreground color AARRGGBB
	uint32_t	font_small_color;	// Color for small  font AARRGGBB
	uint32_t	font_console_color;	// Color for consle font AARRGGBB
	bool		draw;			// Draw flag
} window_t;
	
/*
 * gui structure
 */
typedef struct
{
	uint8_t		maxdevices;		//
	uint8_t		layout;			// Horizontal or Vertical layout
	
	pixmap_t	*backbuffer;		// Off screen buffer

	window_t	screen;			// 
	window_t	background;		// Position of background graphic within screen
	window_t	logo;			// Logo
	window_t	bootprompt;		// Bootprompt Window
	window_t	devicelist;		// Devicelist Window
	window_t	infobox;		// Infobox Window
	window_t	menu;			// Menu

	window_t	progressbar;		// Progress bar
	window_t	countdown;		// Countdown text
	
	window_t	debug;			// Debug

	bool		initialised;		// Initialised
	bool		redraw;			// Redraw flag
} gui_t;


extern gui_t gui;					// gui structure


int  initGUI();
void drawBootGraphics(void);
void drawBackground();
void drawLogo();


void drawDeviceList (int start, int end, int selection, MenuItem *  menuItems);

void showInfoBox(char *title, char *text);

int  gprintf( window_t * window, const char * fmt, ...);
int	 vprf(const char * fmt, va_list ap);

int  drawInfoMenu();
int  updateInfoMenu(int key);

void showGraphicBootPrompt();
void clearGraphicBootPrompt();
void updateGraphicBootPrompt(int key);

void updateVRAM();

#endif /* !__BOOT2_GUI_H */
