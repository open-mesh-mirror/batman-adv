/*
 * Copyright (C) 2007 B.A.T.M.A.N. contributors:
 * Marek Lindner
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 *
 */





#include "batman-adv-main.h"
#include "batman-adv-device.h"
#include "batman-adv-log.h"



#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
#include <linux/devfs_fs_kernel.h>
#else
	static struct class *batman_class;
#endif



static int Major = 0;	/* Major number assigned to our device driver */

static struct file_operations fops = {
	.open = bat_device_open,
	.release = bat_device_release,
	.write = bat_device_write,
};



void bat_device_setup(void)
{
	int tmp_major;

	if (Major)
		return;

	/* register our device - kernel assigns a free major number */
	if ((tmp_major = register_chrdev(0, DRIVER_DEVICE, &fops)) < 0) {
		debug_log(LOG_TYPE_WARN, "Registering the character device failed with %d\n", tmp_major);
		return;
	}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
	if (devfs_mk_cdev(MKDEV(tmp_major, 0), S_IFCHR | S_IRUGO | S_IWUGO, "batman-adv", 0)) {
		debug_log(LOG_TYPE_WARN, "Could not create /dev/batman-adv\n" );
		return;
	}
#else
	batman_class = class_create(THIS_MODULE, "batman-adv");

	if (IS_ERR(batman_class)) {
		debug_log(LOG_TYPE_WARN, "Could not register class 'batman-adv' \n");
		return;
	} else {
		class_device_create(batman_class, NULL, MKDEV(tmp_major, 0), NULL, "batman-adv");
	}
#endif

	Major = tmp_major;
}

void bat_device_destroy(void)
{
	int result=0;

	if (!Major)
		return;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
	devfs_remove("batman-adv", 0);
#else
	class_device_destroy(batman_class, MKDEV(Major, 0));
	class_destroy(batman_class);
#endif

	/* Unregister the device */

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,23)
	result = unregister_chrdev(Major, DRIVER_DEVICE);
#else
	unregister_chrdev(Major, DRIVER_DEVICE);
#endif

	if (result < 0)
		debug_log(LOG_TYPE_WARN, "Unregistering the character device failed with %d\n", result);
	Major = 0;
}

int bat_device_open(struct inode *inode, struct file *file)
{
	inc_module_count();
	return 0;
}

int bat_device_release(struct inode *inode, struct file *file)
{
	dec_module_count();
	return 0;
}

ssize_t bat_device_write(struct file *file, const char *buff, size_t len, loff_t *off)
{
// 	int minor_num;
// 	struct minor *minor = NULL;
//
//
// 	if ( len < sizeof(struct orig_packet) + 10 ) {
//
// 		printk( "B.A.T.M.A.N.: dropping data - packet too small (%i)\n", len );
//
// 		return -EINVAL;
//
// 	}
//
// 	if ( ( minor_num = iminor( file->f_dentry->d_inode ) ) == 0 ) {
//
// 		printk( "B.A.T.M.A.N.: write() not allowed on /dev/batman \n" );
// 		return -EPERM;
//
// 	}
//
// 	if ( !access_ok( VERIFY_READ, buff, sizeof(struct orig_packet) ) )
// 		return -EFAULT;
//
// 	minor = minor_array[minor_num];
//
// 	if ( minor == NULL ) {
//
// 		printk( "B.A.T.M.A.N.: write() - minor number not registered: %i \n", minor_num );
// 		return -EINVAL;
//
// 	}
//
//
// 	minor->iov.iov_base = buff;
// 	minor->iov.iov_len = len;
//
// 	__copy_from_user( &minor->addr_out.sin_port, &((struct orig_packet *)buff)->udp.dest, sizeof(minor->addr_out.sin_port) );
// 	__copy_from_user( &minor->addr_out.sin_addr.s_addr, &((struct orig_packet *)buff)->ip.daddr, sizeof(minor->addr_out.sin_addr.s_addr) );
//
// 	minor->siocb.size = len;
//
// 	return minor->raw_sock->ops->sendmsg( &minor->kiocb, minor->raw_sock, &minor->msg, len );
	return 0;
}


