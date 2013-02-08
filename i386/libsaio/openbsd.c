#include "libsaio.h"
#include "sl.h"
#include "openbsd.h"

#define OpenBSDProbeSize	2048

bool OpenBSDProbe (const void *buf)
{
	return (OSReadLittleInt32(buf+0x200,0)==0x82564557);
}
void OpenBSDGetDescription(CICell ih, char *str, long strMaxLen)
{
	char * buf=calloc(OpenBSDProbeSize, sizeof(char));
	if (!buf)
		return;
    str[0]=0;
	Seek(ih, 0);
	Read(ih, (long)buf, OpenBSDProbeSize);
	if (!OpenBSDProbe (buf))
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