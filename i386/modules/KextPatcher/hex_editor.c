/*
 *  hex_editor.c
 *  
 *
 *  Created by Meklort on 10/19/10.
 *  Copyright 2010 Evan Lojewski. All rights reserved.
 *
 */

#include "hex_editor.h"

int replace_patern(char* pattern, char* replacement, char* buffer, long buffer_size)
{
	long index = 0;
	int num_replaced = 0;
	// NOTE: patehrn and replacement are null terminated. This may change later
	// If I need to replce null bytes
	
	if(!pattern || 
	   !replacement ||
	   !buffer ||
	   strlen(pattern) != strlen(replacement)
	   ) return 0;
	
	
	while(index < buffer_size - strlen(pattern))
	{
		bool located = true;
		int i = 0;
		while(located && i < strlen(pattern))
		{
			if(pattern[i] != buffer[i + index]) located = false;
			i++;
		}
		
		if(located)
		{
			printf("Located patern\n");
			index += strlen(pattern) - 1;
			num_replaced++;
		}
		
		index++;
	}
	
	return num_replaced;
}


int replace_word(uint32_t pattern, uint32_t repalcement, char* buffer, long buffer_size)
{
	int num_replaced = 0;
	char* tmp = buffer;
	
	if(!buffer || !buffer_size) return 0;
	
	
	while(tmp < buffer + buffer_size - sizeof(uint32_t))
	{

		uint32_t* current= (uint32_t*)tmp;
		
		if(*current == pattern)
		{
			*current = repalcement;
			num_replaced++;
			tmp += 4;
		}
		else 
		{
			tmp++;
		}
	}
	
	return num_replaced;
}

void replace_string(char* find, char* replace, char* string, int length)
{
	if(!find ||
	   !replace ||
	   !string ||
	   !length ||
	   strlen(find) != strlen(replace)) return;
	
	char* str = string;
	while(length && strncmp(str, find, strlen(find)-1))
	{
		length--;
		str++;
	}
	strncpy(str, replace, strlen(replace));	// don't copy the null char
}

void replace_bytes(char* find, int find_size, char* replace, int replace_size, char* exec, int length)
{
	if(!find ||
	   !replace ||
	   !exec ||
	   !length ||
	   find_size > replace_size)	// Allow find_size to be less than replace_size. Will overwrite bytes including and *after* located pattern
		return;
	
	char* search = exec;	
	
	
	while(memcmp(search, find, find_size) != 0
		  && ((search - exec) < length))
	{
		search++;
	}

	if((search - exec) < length)
	{
		// Mem found, replace it
		memcpy(search, replace, replace_size);
	}
}

