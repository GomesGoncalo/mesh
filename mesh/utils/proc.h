#pragma once

#include <linux/seq_file.h>

struct proc_handle;

struct proc_handle *proc_register(const char *basename, void *priv_data,
		int (*write_handler)(void *priv_data, struct seq_file *m));
void proc_unregister(struct proc_handle *ph);
int proc_register_command(struct proc_handle *ph, 
		const char *cmd, const char *help, 
		int(*handler)(int argc, char **argv));
