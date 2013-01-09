/*-
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Chris Torek.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)stdio.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
//__FBSDID("$FreeBSD: src/lib/libc/stdio/stdio.c,v 1.28 2008/05/05 16:14:02 jhb Exp $");
#include "stdio.h"
#include "errno.h"
#include "namespace.h"
#include <limits.h>
#include "stat.h"

#include "un-namespace.h"
#include "local.h"
#include "saio_types.h"

#define OFF_MIN		LLONG_MIN	/* min value for an off_t */
#define OFF_MAX		LLONG_MAX	/* max value for an off_t */

/*
 * Small standard I/O/seek/close functions.
 */
int
__sread(cookie, buf, n)
	void *cookie;
	char *buf;
	int n;
{
	FILE *fp = cookie;

	return(read(fp->_file, buf, (size_t)n));
}

int
__swrite(cookie, buf, n)
	void *cookie;
	char const *buf;
	int n;
{
	FILE *fp = cookie;

	return (write(fp->_file, buf, (size_t)n));
}

fpos_t
__sseek(cookie, offset, whence)
	void *cookie;
	fpos_t offset;
	int whence;
{
	FILE *fp = cookie;

	return (b_lseek(fp->_file, (off_t)offset, whence));
}

int
__sclose(cookie)
	void *cookie;
{

	return (close(((FILE *)cookie)->_file));
}

/*
 * Higher level wrappers.
 */
int
_sread(fp, buf, n)
	FILE *fp;
	char *buf;
	int n;
{
	int ret;

	ret = (*fp->_read)(fp->_cookie, buf, n);
	if (ret > 0) {
		if (fp->_flags & __SOFF) {
			if (fp->_offset <= OFF_MAX - ret)
				fp->_offset += ret;
			else
				fp->_flags &= ~__SOFF;
		}
	} else if (ret < 0)
		fp->_flags &= ~__SOFF;
	return (ret);
}

int
_swrite(fp, buf, n)
	FILE *fp;
	char const *buf;
	int n;
{
	int ret;
    int serrno;

	if (fp->_flags & __SAPP) {
        serrno = get_errno();
		if (_sseek(fp, (fpos_t)0, SEEK_END) == -1 &&
		    (fp->_flags & __SOPT))
			return (-1);
        set_errno(serrno);

	}
	ret = (*fp->_write)(fp->_cookie, buf, n);
	/* __SOFF removed even on success in case O_APPEND mode is set. */
	if (ret >= 0) {
		if ((fp->_flags & (__SAPP|__SOFF)) == (__SAPP|__SOFF) &&
		    fp->_offset <= OFF_MAX - ret)
			fp->_offset += ret;
		else
			fp->_flags &= ~__SOFF;

	} else if (ret < 0)
		fp->_flags &= ~__SOFF;
	return (ret);
}

fpos_t
_sseek(fp, offset, whence)
	FILE *fp;
	fpos_t offset;
	int whence;
{
	fpos_t ret;
    int serrno, errret;

    serrno = get_errno();
	set_errno(0);
	ret = (*fp->_seek)(fp->_cookie, offset, whence);
    errret = get_errno();
    if (get_errno() == 0)
		set_errno(serrno) ;
	/*
	 * Disallow negative seeks per POSIX.
	 * It is needed here to help upper level caller
	 * in the cases it can't detect.
	 */
	if (ret < 0) {
		if (errret == 0) {
			if (offset != 0 || whence != SEEK_CUR) {
				if (HASUB(fp))
					FREEUB(fp);
				fp->_p = fp->_bf._base;
				fp->_r = 0;
				fp->_flags &= ~__SEOF;
			}
			fp->_flags |= __SERR;
            set_errno(EINVAL)  ;
		} else if (errret == ESPIPE)
			fp->_flags &= ~__SAPP;
		fp->_flags &= ~__SOFF;
		ret = -1;
	} else if (fp->_flags & __SOPT) {
		fp->_flags |= __SOFF;
		fp->_offset = ret;
	}
	return (ret);
}

int
fstat(int fildes, struct stat *buf)
{
    struct stat *st =(struct stat *)buf;
    
    st->st_size = file_size(fildes);
    
    return 0;
}

void Libstdio_start(void);
void Libstdio_start(void)
{
    
}

