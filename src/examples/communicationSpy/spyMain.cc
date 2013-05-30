#include "Spy.hh"
#include <arpa/inet.h>

#include <arpa/inet.h>
#include <arpa/inet.h>
#include <errno.h>

#include <fcntl.h>
#include <getopt.h>
#include <ifaddrs.h>
#include <iostream>
#include <net/ethernet.h>
#include <netdb.h>
#include <netinet/ether.h>
#include <netinet/if_ether.h>
#include <netinet/if_ether.h> /* includes net/ethernet.h */
#include <netinet/in.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <pcap.h>
#include <pcap.h>

#include <pcap/sll.h> // "any" capture
#include <signal.h>

#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <syslog.h>
#include <unistd.h>

typedef u_int32_t tcp_seq;
struct nread_ip {
    u_int8_t        ip_vhl;          /* header length, version    */
#define IP_V(ip)    (((ip)->ip_vhl & 0xf0) >> 4)
#define IP_HL(ip)   ((ip)->ip_vhl & 0x0f)
    u_int8_t        ip_tos;          /* type of service           */
    u_int16_t       ip_len;          /* total length              */
    u_int16_t       ip_id;           /* identification            */
    u_int16_t       ip_off;          /* fragment offset field     */
#define IP_DF 0x4000                 /* dont fragment flag        */
#define IP_MF 0x2000                 /* more fragments flag       */
#define IP_OFFMASK 0x1fff            /* mask for fragmenting bits */
    u_int8_t        ip_ttl;          /* time to live              */
    u_int8_t        ip_p;            /* protocol                  */
    u_int16_t       ip_sum;          /* checksum                  */
    struct  in_addr ip_src, ip_dst;  /* source and dest address   */
};

struct nread_tcp {
  u_short th_sport; /* source port            */
  u_short th_dport; /* destination port       */
    tcp_seq th_seq;   /* sequence number        */
    tcp_seq th_ack;   /* acknowledgement number */
#if BYTE_ORDER == LITTLE_ENDIAN
    u_int th_x2:4,    /* (unused)    */
    th_off:4;         /* data offset */
#endif
#if BYTE_ORDER == BIG_ENDIAN
    u_int th_off:4,   /* data offset */
    th_x2:4;          /* (unused)    */
#endif
    u_char th_flags;
#define TH_FIN 0x01
#define TH_SYN 0x02
#define TH_RST 0x04
#define TH_PUSH 0x08
#define TH_ACK 0x10
#define TH_URG 0x20
#define TH_ECE 0x40
#define TH_CWR 0x80
    u_short th_win; /* window */
    u_short th_sum; /* checksum */
    u_short th_urp; /* urgent pointer */
};

u_char* ip_handler(u_char *args, const struct pcap_pkthdr* pkthdr,
    const u_char* packet) {
  const struct nread_ip* ip; /* packet structure         */
  const struct nread_tcp* tcp; /* tcp structure            */
  u_int off, version; /* offset, version       */
  int len; /* length holder         */

  ip = (struct nread_ip*) (packet + sizeof(struct sll_header));
  tcp = (struct nread_tcp*) ((const u_char *) ip + IP_HL(ip)*4);

  len = ntohs(ip->ip_len); /* get packet length */

  len -= IP_HL(ip)*4;
  len -= tcp->th_off*4;

  if (len > 0) {

  version = IP_V(ip); /* get ip version    */

  off = ntohs(ip->ip_off);
/*
  fprintf(stdout, "ip: ");
  fprintf(stdout, "%s:%u->%s:%u ", inet_ntoa(ip->ip_src), htons(tcp->th_sport),
          inet_ntoa(ip->ip_dst), htons(tcp->th_dport));
  fprintf(stdout, "tos %u len %u off %u ttl %u prot %u cksum %u ", ip->ip_tos,
          len, off, ip->ip_ttl, ip->ip_p, ip->ip_sum);

  fprintf(stdout, "seq %u ack %u win %u ", tcp->th_seq, tcp->th_ack,
          tcp->th_win);
*/
  printf("\n");

  //printf("Offset = %i\n", sizeof(struct sll_header) + IP_HL(ip)*4        + tcp->th_off * 4);
  const u_char * data = (const u_char *) tcp + tcp->th_off * 4;
    for (int i = 0; i < len; i++) {
      if (isprint(data[i])) /* Check if the packet data is printable */
        printf("%c", data[i]); /* Print it */
      else
        printf(".", data[i]); /* If not print a . */
     /* if ((i % 64 == 0 && i != 0) || i == len - 1)
        printf("\n");*/
    }
  }


  return NULL;
}

/* callback function that is passed to pcap_loop(..) and called each time
 * a packet is recieved                                                    */
void my_callback(u_char *args, const struct pcap_pkthdr* pkthdr,
    const u_char* packet) {

//  u_int16_t type = ethernet_handler(args, pkthdr, packet+16);
  ip_handler(args, pkthdr, packet);
}

int grabLoop(const char * filter_str) {
  int i;
  char *dev;
  pcap_if_t * alldevsp, *device;
  char errbuf[PCAP_ERRBUF_SIZE];
  pcap_t* descr;
  const u_char *packet;
  struct pcap_pkthdr hdr; /* pcap.h */
  struct ether_header *eptr; /* net/ethernet.h */
  int count = 1, n;
  struct bpf_program filter;
  bpf_u_int32 net;

  /* open device for reading */
  descr = pcap_open_live("any", BUFSIZ, 0, -1, errbuf);
  if (descr == NULL) {
    printf("pcap_open_live(): %s\n", errbuf);
    exit(1);
  }

  if (pcap_compile(descr, &filter, filter_str, 0, net) == -1) {
    fprintf(stderr, "Error compiling pcap\n");
    return (1);
  }

  if (pcap_setfilter(descr, &filter)) {
    fprintf(stderr, "Error setting pcap filter\n");
    return (1);
  }

  /* allright here we call pcap_loop(..) and pass in our callback function */
  /* int pcap_loop(pcap_t *p, int cnt, pcap_handler callback, u_char *user)*/
  /* If you are wondering what the user argument is all about, so am I!!   */
  pcap_loop(descr, -1, my_callback, NULL);

  fprintf(stdout, "\nDone processing packets... wheew!\n");
  return 0;
}

int main(int argc, char **argv) {
  Spy s(argc,argv);
  std::cout << s.createFilter() << std::endl;
  return grabLoop(s.createFilter().c_str());

}
