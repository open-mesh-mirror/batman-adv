//
// C++ Interface: batman-adv-proc
//
// Description:
//
//
// Author: Marek Lindner <lindner_marek@yahoo.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <linux/proc_fs.h>

#define PROC_ROOT_DIR "batman-adv"
#define PROC_FILE_INTERFACES "interfaces"

int proc_interfaces_read( char *buf, char **start, off_t offset, int size, int *eof, void *data);
