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
#include "batman-adv-log.h"


#define LOG_BUF_MASK (log_buf_len-1)
#define LOG_BUF(idx) (log_buf[(idx) & LOG_BUF_MASK])



char log_buf[LOG_BUF_LEN];
int log_buf_len = LOG_BUF_LEN;
unsigned long log_start = 0;
unsigned long log_end = 0;

spinlock_t logbuf_lock = SPIN_LOCK_UNLOCKED;

struct file_operations proc_log_operations = {
	.read           = log_read,
	.poll           = log_poll,
	.open           = log_open,
	.release        = log_release,
};

DECLARE_WAIT_QUEUE_HEAD(log_wait);


static void emit_log_char(char c)
{
	LOG_BUF(log_end) = c;
	log_end++;

	if (log_end - log_start > log_buf_len)
		log_start = log_end - log_buf_len;
}

int vdebug_log(const char *fmt, va_list args)
{
	int printed_len;
	char *p;
	static char debug_log_buf[1024];

	spin_lock(&logbuf_lock);

	printed_len = vscnprintf(debug_log_buf, sizeof(debug_log_buf), fmt, args);

	for (p = debug_log_buf; *p != 0; p++)
		emit_log_char(*p);

	spin_unlock(&logbuf_lock);

	wake_up(&log_wait);

	return printed_len;
}

int debug_log(int type, char *fmt, ...)
{
	va_list args;
	int retval;

	if (type == LOG_TYPE_CRIT) {
		va_start(args, fmt);
		vprintk(fmt, args);
		va_end(args);
	}

	va_start(args, fmt);
	retval = vdebug_log(fmt, args);
	va_end(args);

	return retval;
}

int log_open(struct inode * inode, struct file * file)
{
	inc_module_count();
	return 0;
}

int log_release(struct inode * inode, struct file * file)
{
	dec_module_count();
	return 0;
}

ssize_t log_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	int error, i = 0;
	char c;

	if ((file->f_flags & O_NONBLOCK) && !(log_end - log_start))
		return -EAGAIN;

	if ((!buf) || (count < 0))
		return -EINVAL;

	if (count == 0)
		return 0;

	if (!access_ok(VERIFY_WRITE, buf, count))
		return -EFAULT;

	error = wait_event_interruptible(log_wait, (log_start - log_end));

	if (error)
		return error;

	spin_lock(&logbuf_lock);

	while ((!error) && (log_start != log_end) && (i < count)) {
		c = LOG_BUF(log_start);

		log_start++;

		spin_unlock(&logbuf_lock);

		error = __put_user(c,buf);

		buf++;
		i++;

		spin_lock(&logbuf_lock);
	}

	spin_unlock(&logbuf_lock);

	if (!error)
		return i;

	return error;
}

unsigned int log_poll(struct file *file, poll_table *wait)
{
	poll_wait(file, &log_wait, wait);

	if (log_end - log_start)
		return POLLIN | POLLRDNORM;

	return 0;
}

