#include <linux/init.h>
#include <linux/tcp.h>

#define DROP 0
#define ACCEPT 1

unsigned int magic_packet_parse(struct sk_buff *socket_buffer);
