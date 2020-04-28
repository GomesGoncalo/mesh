
#include <utils/proc.h>

#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/list.h>

struct proc_command_handle {
	struct list_head list;
	char *command;
	char *help;
	int (*handler)(int argc, char **argv);
};

struct proc_handle {
#define PROC_HANDLE_BASENAME_SIZE 128
	char basename[PROC_HANDLE_BASENAME_SIZE];
	void *priv_data;
	int (*write_handler)(void *priv_data, struct seq_file *m);

	struct proc_dir_entry *proc_entry;

	rwlock_t command_list_lock;
	struct list_head command_list;
};


static int dyn_proc_show(struct seq_file *m, void *v)
{
	struct proc_handle *ph = m->private;
	if (ph->write_handler)
		ph->write_handler(ph->priv_data, m);
	return 0;
}

static int dyn_proc_open(struct inode *inode, struct file *filp)
{
	struct proc_handle *ph;

	ph = PDE_DATA(inode);
	return single_open(filp, dyn_proc_show, ph);
}

static ssize_t dyn_proc_write(struct file *file, const char *buffer, size_t count, loff_t *pos)
{
	struct inode *inode = file->f_path.dentry->d_inode;
	char *cmd;
	char *argv[16], *p;
	int i, argc;
	struct proc_handle *ph;
	struct proc_command_handle *pch;
	struct list_head *pl;

	if (!(cmd = (char*)kmalloc(count + 1, GFP_KERNEL)))
		return -1;

	cmd[count] = '\0';

	if (copy_from_user(cmd, buffer, count) > 0)
		goto out;

	ph = PDE_DATA(inode);

	for (i = 0; i < 16; i++)
		argv[i] = NULL;

	p = cmd;
	while (*p && strchr(" \t\r\n", *p))
		p++;
	if (*p == 0)
		goto out;

	i = 0;
	while (i < 15) {
		argv[i++] = p;
		while (*p && !strchr(" \t\r\n", *p))
			p++;
		if (*p == 0)
			break;
		*p++ = 0;
		while (*p && strchr(" \t\r\n", *p))
			p++;
		if (*p == 0)
			break;
	}
	argc = i;

	read_lock(&ph->command_list_lock);
	list_for_each(pl, &ph->command_list) {
		pch = list_entry(pl, struct proc_command_handle, list);
		if (!(strcmp(argv[0], pch->command))) {
			if (pch->handler) {
				pch->handler(argc, argv);
			}
			read_unlock(&ph->command_list_lock);
			goto out;
		}
	}
	read_unlock(&ph->command_list_lock);

out:
	kfree(cmd);
	return count;
}

static struct proc_ops dyn_proc_fops = {
	.owner   = THIS_MODULE,
	.open    = dyn_proc_open,
	.read    = seq_read,
	.llseek  = seq_lseek,
	.release = single_release,
	.write   = dyn_proc_write,
	.poll    = NULL,
	.unlocked_ioctl = NULL,
};

char *my_strndup(const char *s, size_t max, gfp_t gfp)
{
	size_t len;
	char *buf;

	if (!s)
		return NULL;

	len = strnlen(s, max);
	buf = kmalloc(len+1, gfp);
	if (buf) {
		memcpy(buf, s, len);
		buf[len] = '\0';
	}
	return buf;
}

int proc_register_command(struct proc_handle *ph, 
	const char *cmd, 
	const char *help,
	int(*handler)(int argc, char **argv))
{
	struct proc_command_handle *pch;

	if (!(pch = (struct proc_command_handle*)kmalloc(sizeof(struct proc_command_handle), GFP_KERNEL)))
		return -1;

	pch->command = my_strndup(cmd, 4096, GFP_KERNEL);
	pch->help = my_strndup(help, 4096, GFP_KERNEL);
	pch->handler = handler;

	write_lock(&ph->command_list_lock);
	list_add(&pch->list, &ph->command_list);
	write_unlock(&ph->command_list_lock);

	return 0;
}


struct proc_handle *proc_register(const char *basename, void *priv_data,
		int (*write_handler)(void *priv_data, struct seq_file *m))
{
	struct proc_handle *ph;

	if (!(ph = (struct proc_handle*)kmalloc(sizeof(struct proc_handle), GFP_KERNEL)))
		return NULL;

	memset(ph, 0, sizeof(struct proc_handle));

	strncpy(ph->basename, basename, PROC_HANDLE_BASENAME_SIZE - 1);

	INIT_LIST_HEAD(&ph->command_list);
	rwlock_init(&ph->command_list_lock);

	ph->proc_entry = proc_create_data(ph->basename, 0666, NULL, &dyn_proc_fops, ph);
	if (ph->proc_entry == NULL) {
		kfree(ph);
		return NULL;
	}

	ph->priv_data = priv_data;
	ph->write_handler = write_handler;
	return ph;
}


static inline void __del_command_handle(struct proc_command_handle *pch)
{
	kfree(pch->command);
	kfree(pch->help);
	pch->handler = NULL;
}


void proc_unregister(struct proc_handle *ph)
{
	struct list_head *p, *n;
	struct proc_command_handle *pch;

	write_lock(&ph->command_list_lock);
	list_for_each_safe(p, n, &ph->command_list) {
		pch = list_entry(p, struct proc_command_handle, list);
		list_del(p);
		__del_command_handle(pch);
	}

	remove_proc_entry(ph->basename, NULL);
	write_unlock(&ph->command_list_lock);
	kfree(ph);
}

