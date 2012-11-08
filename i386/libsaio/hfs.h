/*
 * Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * The contents of this file constitute Original Code as defined in and
 * are subject to the Apple Public Source License Version 2.0 (the
 * "License").  You may not use this file except in compliance with the
 * License.  Please obtain a copy of the License at
 * http://www.apple.com/publicsource and read it before using this file.
 * 
 * This Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

extern long HFSInitPartition(CICell ih);
extern long HFSLoadFile(CICell ih, char * filePath);
extern long HFSReadFile(CICell ih, char * filePath, void *base, uint64_t offset, uint64_t length);
extern long HFSGetDirEntry(CICell ih, char * dirPath, long long * dirIndex,
                           char ** name, long * flags, long * time,
                           FinderInfo * finderInfo, long * infoValid);
extern void HFSGetDescription(CICell ih, char *str, long strMaxLen);
extern long HFSGetFileBlock(CICell ih, char *str, unsigned long long *firstBlock);
extern long HFSGetUUID(CICell ih, char *uuidStr,  long strMaxLen);
extern void HFSFree(CICell ih);
extern bool HFSProbe (const void *buf);


extern int32_t	FastRelString(u_int8_t * str1, u_int8_t * str2);
extern int32_t BinaryUnicodeCompare (u_int16_t * str1, u_int32_t length1,
									 u_int16_t * str2, u_int32_t length2);