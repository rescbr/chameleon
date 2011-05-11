/*
 *  Keymapper.c
 *  Chameleon
 *
 *  Created by Cadet-petit Armel on 05/12/10. <armelcadetpetit@gmail.com>
 *  Copyright 2010. All rights reserved.
 *
 */

#include "libsaio.h"
#include "bootstruct.h"
#include "xml.h"
#include "modules.h"

// CPARM's AZERTY_switch : A Basic QWERTY to AZERTY switcher
int AZERTY_switch(int c)
{		
	switch (c) {
		case 4209: //q to a
			c = 7777;
			break;
		case 7777: // a to q
			c = 4209;
			break;
		case 4177: // Q to A
			c = 7745;
			break;
		case 7745: // A to Q
			c = 4177;
			break;
		case 4471: // w to z
			c = 11386;
			break;
		case 11386: // z to w
			c = 4471;
			break;
		case 4439: // W to Z
			c = 11354;
			break;
		case 11354: // Z to W
			c = 4439;
			break;			
		case 10043: // ; to m
			c = 12909;
			break;
		case 10042: // : to M
			c = 12877;
			break;
		case 12909: // m to ,
			c = 13100;
			break;		
		case 12877: // M to ? 
			c = 13631;
			break;			
		case 13100: // , to ;
			c = 10043;
			break;			
		case 13116: // < to .
			c = 13358;
			break;
		case 13374: // > to /
			c = 13615;
			break;
		case 13358: // . to :
			c = 10042;
			break;
		case 22108: // \ to <
			c = 13116;
			break;			
		case 22140: // | to >
			c = 13374;
			break;			
		case 13615: // / to !
			c = 545;
			break;
		case 10279: // ' to % 
			c = 1573;
			break;
		case 10274: // " to $ 
			c = 1316;
			break;
			
			/* switch for main keyboard (num) */
		case 10592: //  to #
			c = 1059;
			break;			
		case 10622: //  to @
			c = 832;
			break;			
		case 545: // ! to &
			c = 2086;
			break;			
		case 832: // @ to ~
			c = 10622;
			break;
		case 1059: // # to "
			c = 10274;
			break;			
		case 1316: // $ to '
			c = 10279;
			break;			
		case 1573: // % to (
			c = 2600;
			break;
		case 1886: // ^ to -
			c = 3117;
			break;			
		case 2086: // & to ´
			c = 10592;
			break;			
		case 2346: // * to _
			c = 3167;
			break;
		case 2600: // ( to ^
			c = 1886;
			break;
		case 2857: // ) to @
			c = 832;
			break;			
		case 3117: // - to )
			c = 2857;
			break;
		case 3167: // _ to ]
			c = 7005;
			break;			
		case 3389: // = to +
			c = 3371;
			break;			
		case 3371: // + to =
			c = 3389;
			break;
			
			/* switch for main keyboard (num) with alt pressed */
		case 30720: //  to & 
			c = 2086;
			break;
		case 30976: //  to ~ 
			c = 10622;
			break;
		case 31232: //  to #
			c = 1059;
			break;			
		case 31488: 
			c = 6779;
			break;
		case 31744: 
			c = 6747;
			break;
		case 32000: //  to |
			c = 11132;
			break;
		case 32256: 
			c = 10592;
			break;			
		case 32512: 
			c = 11100;
			break;
		case 32768: 
			c = 1886;
			break;
		case 33024:
			c = 832;
			break;
		case 33280: //  to {
			c = 7005;
			break;			
		case 33536: //  to }
			c = 7037;
			break;		
			
		default:			
			break;
	}
	return c;
}

static char *map_kb_type = NULL;	
static TagPtr match_map = NULL;

void Keymapper_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6)
{
	int *ret = (int *)arg1;
	int c = *(int *)ret;
	
	
	if (match_map == NULL)				
		match_map = XMLGetProperty(bootInfo->bootConfig.dictionary, (const char*)"KeyboardMap");
	
	if (match_map)
	{
		char *kMatchkey = 0; 
		sprintf(kMatchkey, "%d",c);
		TagPtr match_key;
		if (match_key = XMLGetProperty(match_map, (const char*)kMatchkey))
		{
			kMatchkey = XMLCastString(match_key);
			c  = strtoul((const char *)kMatchkey, NULL,10);		
			if (c) goto out;
		}
	}
	
	if (map_kb_type == NULL){		
		TagPtr match_type;
		if (match_type = XMLGetProperty(bootInfo->bootConfig.dictionary, (const char*)"KeyboardType"))
			map_kb_type = XMLCastString(match_type);
		else 
			map_kb_type =  "NONE"; // Default to QWERTY
	}
	
	if (strcmp(map_kb_type, "AZERTY") == 0)		
        c = AZERTY_switch(c);
	
out:
	
	*ret = c;
	
}

void Keymapper_start()
{
	register_hook_callback("Keymapper", &Keymapper_hook);
}