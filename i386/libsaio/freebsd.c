#include "libsaio.h"
#include "sl.h"
#include "freebsd.h"

#define FreeBSDProbeSize	2048

bool FreeBSDProbe (const void *buf)
{
	return (OSReadLittleInt32(buf+0xA55C,0)==0x19540119);
}
void FreeBSDGetDescription(CICell ih, char *str, long strMaxLen)
{
	char * buf=malloc(FreeBSDProbeSize);
	if (!buf)
		return;
	bzero(buf, FreeBSDProbeSize);
	
    str[0]=0;
	Seek(ih, 0);
	Read(ih, (long)buf, FreeBSDProbeSize);
	if (!FreeBSDProbe (buf))
	{
		free (buf);
		return;
	}
	if (OSReadLittleInt32 (buf+0x44c,0)<1)
	{
		free (buf);
		return;
	}
	str[strMaxLen]=0;
	strncpy (str, buf+0x478, min (strMaxLen, 32));
	free (buf);
}