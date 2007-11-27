/*
*  C Implementation: batman-adv-proc
*
* Description:
*
*
* Author: Marek Lindner <lindner_marek@yahoo.de>, (C) 2007
*
* Copyright: See COPYING file that comes with this distribution
*
*/

#include "batman-adv-proc.h"


int proc_interfaces_read( char *buf, char **start, off_t offset, int size, int *eof, void *data)
{
	int bytes_written=0, ret;

	ret=snprintf(buf+bytes_written,(size-bytes_written),"proc_read wurde\n");
	bytes_written += (ret>(size-bytes_written)) ? (size-bytes_written):ret;
	ret=snprintf(buf+bytes_written, size-bytes_written, "aufgerufen.\n");
	bytes_written += (ret>(size-bytes_written)) ? (size-bytes_written):ret;

	*eof = 1;
	return bytes_written;
}

