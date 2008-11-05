/*
 * Copyright (C) 2007-2008 B.A.T.M.A.N. contributors:
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





#include "main.h"
#include "device.h"
#include "log.h"
#include "send.h"
#include "types.h"
#include "hash.h"

#include "compat.h"

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0)
static struct class *batman_class;
#endif



static int Major = 0;	/* Major number assigned to our device driver */

static struct file_operations fops = {
	.open = bat_device_open,
	.release = bat_device_release,
	.read = bat_device_read,
	.write = bat_device_write,
	.poll = bat_device_poll,
};

static struct device_client *device_client_hash[256];



void bat_device_init(void)
{
	int i;

	for (i = 0; i < 256; i++)
		device_client_hash[i] = NULL;
}

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
		device_create(batman_class, NULL, MKDEV(tmp_major, 0), NULL, "batman-adv");
	}
#endif

	Major = tmp_major;
}

void bat_device_destroy(void)
{
	int result = 0;

	if (!Major)
		return;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
	devfs_remove("batman-adv", 0);
#else
	device_destroy(batman_class, MKDEV(Major, 0));
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
	unsigned int i;
	struct device_client *device_client;

	device_client = kmalloc(sizeof(struct device_client), GFP_KERNEL);

	if (!device_client)
		return -ENOMEM;

	for (i = 0; i < 256; i++) {
		if (device_client_hash[i] == NULL) {
			device_client_hash[i] = device_client;
			break;
		}
	}

	if (device_client_hash[i] != device_client) {
		debug_log(LOG_TYPE_WARN, "Error - can't add another packet client: maximum number of clients reached \n");
		kfree(device_client);
		return -EXFULL;
	}

	INIT_LIST_HEAD(&device_client->queue_list);
	device_client->queue_len = 0;
	device_client->index = i;
	device_client->lock = __SPIN_LOCK_UNLOCKED(device_client->lock);
	init_waitqueue_head(&device_client->queue_wait);

	file->private_data = device_client;

	inc_module_count();
	return 0;
}

int bat_device_release(struct inode *inode, struct file *file)
{
	struct device_client *device_client = (struct device_client *)file->private_data;
	struct device_packet *device_packet;
	struct list_head *list_pos, *list_pos_tmp;

	spin_lock(&device_client->lock);

	/* for all packets in the queue ... */
	list_for_each_safe(list_pos, list_pos_tmp, &device_client->queue_list) {
		device_packet = list_entry(list_pos, struct device_packet, list);

		list_del(list_pos);
		kfree(device_packet);
	}

	device_client_hash[device_client->index] = NULL;
	spin_unlock(&device_client->lock);

	kfree(device_client);
	dec_module_count();

	return 0;
}

ssize_t bat_device_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	struct device_client *device_client = (struct device_client *)file->private_data;
	struct device_packet *device_packet;
	int error;

	if ((file->f_flags & O_NONBLOCK) && (device_client->queue_len == 0))
		return -EAGAIN;

	if ((!buf) || (count < sizeof(struct icmp_packet)))
		return -EINVAL;

	if (!access_ok(VERIFY_WRITE, buf, count))
		return -EFAULT;

	error = wait_event_interruptible(device_client->queue_wait, (device_client->queue_len));

	if (error)
		return error;

	spin_lock(&device_client->lock);

	device_packet = list_first_entry(&device_client->queue_list, struct device_packet, list);
	list_del(&device_packet->list);
	device_client->queue_len--;

	spin_unlock(&device_client->lock);

	error = __copy_to_user(buf, &device_packet->icmp_packet, sizeof(struct icmp_packet));

	kfree(device_packet);

	if (error)
		return error;

	return sizeof(struct icmp_packet);
}

ssize_t bat_device_write(struct file *file, const char __user *buff, size_t len, loff_t *off)
{
	struct device_client *device_client = (struct device_client *)file->private_data;
	struct icmp_packet icmp_packet;
	struct orig_node *orig_node;

	if (len < sizeof(struct icmp_packet)) {
		debug_log(LOG_TYPE_NOTICE, "Error - can't send packet from char device: invalid packet size\n");
		return -EINVAL;
	}

	if (!access_ok(VERIFY_READ, buff, sizeof(struct icmp_packet)))
		return -EFAULT;

	if (__copy_from_user(&icmp_packet, buff, sizeof(icmp_packet))) {
		return -EFAULT;
	}

	if ((icmp_packet.packet_type == BAT_ICMP) && (icmp_packet.msg_type == ECHO_REQUEST)) {

		spin_lock(&orig_hash_lock);
		orig_node = ((struct orig_node *)hash_find(orig_hash, icmp_packet.dst));

		if ((orig_node != NULL) && (orig_node->batman_if != NULL) && (orig_node->router != NULL)) {

			memcpy(icmp_packet.orig, orig_node->batman_if->net_dev->dev_addr, ETH_ALEN);
			icmp_packet.uid = device_client->index;

			send_raw_packet((unsigned char *)&icmp_packet, sizeof(struct icmp_packet), orig_node->batman_if->net_dev->dev_addr, orig_node->router->addr, orig_node->batman_if);

		} else {

			icmp_packet.msg_type = DESTINATION_UNREACHABLE;
			icmp_packet.uid = device_client->index;
			bat_device_add_packet(device_client, &icmp_packet);

		}

		spin_unlock(&orig_hash_lock);

	} else {

		debug_log(LOG_TYPE_NOTICE, "Error - can't send packet from char device: got bogus packet\n");
		return -EINVAL;

	}

	return len;
}

unsigned int bat_device_poll(struct file *file, poll_table *wait)
{
	struct device_client *device_client = (struct device_client *)file->private_data;

	poll_wait(file, &device_client->queue_wait, wait);

	if (device_client->queue_len > 0)
		return POLLIN | POLLRDNORM;

	return 0;
}

void bat_device_add_packet(struct device_client *device_client, struct icmp_packet *icmp_packet)
{
	struct device_packet *device_packet;

	device_packet = kmalloc(sizeof(struct device_packet), GFP_KERNEL);

	if (!device_packet)
		return;

	INIT_LIST_HEAD(&device_packet->list);
	memcpy(&device_packet->icmp_packet, icmp_packet, sizeof(icmp_packet));

	spin_lock(&device_client->lock);

	/* while waiting for the lock the device_client could have been deleted */
	if (device_client_hash[icmp_packet->uid] == NULL) {
		kfree(device_packet);
		return;
	}

	list_add_tail(&device_packet->list, &device_client->queue_list);
	device_client->queue_len++;

	if (device_client->queue_len > 100) {
		device_packet = list_first_entry(&device_client->queue_list, struct device_packet, list);

		list_del(&device_packet->list);
		kfree(device_packet);
		device_client->queue_len--;
	}

	spin_unlock(&device_client->lock);

	wake_up(&device_client->queue_wait);
}

void bat_device_receive_packet(struct icmp_packet *icmp_packet)
{
	if (device_client_hash[icmp_packet->uid] != NULL)
		bat_device_add_packet((struct device_client *)device_client_hash[icmp_packet->uid], icmp_packet);
}

