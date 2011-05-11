/*
 * Copyright (c) 2009 Evan Lojewski. All rights reserved.
 *
 */
#ifndef __BOOT2_KERNEL_PATCHER_H
#define __BOOT2_KERNEL_PATCHER_H


void KextPatcher_start();

void kext_loaded(void* module, void* length, void* executableAddr, void* arg4, void* arg5, void* arg6);
void mkext_loaded(void* filespec, void* package, void* lenght, void* arg4, void* arg5, void* arg6);


#endif /* !__BOOT2_KERNEL_PATCHER_H */
