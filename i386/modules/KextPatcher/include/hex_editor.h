/*
 *  hex_editor.h
 *  
 *
 *  Created by Meklort on 10/19/10.
 *  Copyright 2010 Evan Lojewski. All rights reserved.
 *
 */
#ifndef H_HEX_EDITOR
#define H_HEX_EDITOR
#include "libsaio.h"

int replace_patern(char* pattern, char* repalcement, char* buffer, long buffer_size);
int replace_word(uint32_t pattern, uint32_t repalcement, char* buffer, long buffer_size);
void replace_string(char* find, const char* replace, char* string, int length);
void replace_bytes(char* find, int find_size, char* replace, int replace_size, char* exec, int length);
#endif /* H_HEX_EDITOR */