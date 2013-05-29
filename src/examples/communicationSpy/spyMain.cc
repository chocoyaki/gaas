#include "Spy.hh"
#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <pcap.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/if_ether.h> /* includes net/ethernet.h */
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <arpa/inet.h>
#include <net/ethernet.h>
#include <netinet/ether.h>
#include <netinet/if_ether.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>

#include <fcntl.h>
#include <getopt.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <pcap.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <arpa/inet.h>

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

u_int16_t ethernet_handler(u_char *args, const struct pcap_pkthdr* pkthdr,
    const u_char* packet)

    {
  u_int caplen = pkthdr->caplen; /* length of portion present from bpf  */
  u_int length = pkthdr->len; /* length of this packet off the wire  */
  struct ether_header *eptr; /* net/ethernet.h                      */
  u_short ether_type; /* the type of packet (we return this) */
  eptr = (struct ether_header *) packet;
  ether_type = ntohs(eptr->ether_type);
  fprintf(stdout, "eth: ");
  fprintf(stdout, "%s ", ether_ntoa((struct ether_addr*) eptr->ether_shost));
  fprintf(stdout, "%s ", ether_ntoa((struct ether_addr*) eptr->ether_dhost));

  if (ether_type == ETHERTYPE_IP) {
    fprintf(stdout, "(ip)");
  } else if (ether_type == ETHERTYPE_ARP) {
    fprintf(stdout, "(arp)");
  } else if (eptr->ether_type - ether_type == ETHERTYPE_REVARP) {
    fprintf(stdout, "(rarp)");
  } else {
    fprintf(stdout, "(?)");
  }

  fprintf(stdout, " %d\n", length); /* print len */

  return ether_type;

}

u_char* ip_handler(u_char *args, const struct pcap_pkthdr* pkthdr,
    const u_char* packet) {
  const struct nread_ip* ip; /* packet structure         */
  const struct nread_tcp* tcp; /* tcp structure            */
  u_int length = pkthdr->len; /* packet header length  */
  u_int off, version; /* offset, version       */
  int len; /* length holder         */

  ip = (struct nread_ip*) (packet + sizeof(struct ether_header));
  length -= sizeof(struct ether_header);
  tcp = (struct nread_tcp*) (packet + sizeof(struct ether_header)
      + sizeof(struct nread_ip));

  len = ntohs(ip->ip_len); /* get packer length */
  version = IP_V(ip); /* get ip version    */

  off = ntohs(ip->ip_off);

  fprintf(stdout, "ip: ");
  fprintf(stdout, "%s:%u->%s:%u ", inet_ntoa(ip->ip_src), tcp->th_sport,
          inet_ntoa(ip->ip_dst), tcp->th_dport);
  fprintf(stdout, "tos %u len %u off %u ttl %u prot %u cksum %u ", ip->ip_tos,
          len, off, ip->ip_ttl, ip->ip_p, ip->ip_sum);

  fprintf(stdout, "seq %u ack %u win %u ", tcp->th_seq, tcp->th_ack,
          tcp->th_win);
  //fprintf(stdout, "%s", payload);
  printf("\n");

  return NULL;
}

/* callback function that is passed to pcap_loop(..) and called each time
 * a packet is recieved                                                    */
void my_callback(u_char *args, const struct pcap_pkthdr* pkthdr,
    const u_char* packet) {
  u_int16_t type = ethernet_handler(args, pkthdr, packet);
  ip_handler(args, pkthdr, packet);
          if (type == ETHERTYPE_IP) {
                  ip_handler(args, pkthdr, packet);
           } else if (type == ETHERTYPE_ARP) {
                  /* noop */
          } else if (type == ETHERTYPE_REVARP) {
                  /* noop */
          }
}

int grabLoop() {
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

  if (pcap_compile(descr, &filter, "port 46220", 0, net) == -1) {
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
  pcap_loop(descr, 10, my_callback, NULL);

  fprintf(stdout, "\nDone processing packets... wheew!\n");
  return 0;
}

int grabFirst() {
  int i;
  char *dev;
  char errbuf[PCAP_ERRBUF_SIZE];
  pcap_t* descr;
  const u_char *packet;
  struct pcap_pkthdr hdr; /* pcap.h */
  struct ether_header *eptr; /* net/ethernet.h */

  u_char *ptr; /* printing out hardware header info */

  /* grab a device to peak into... */
  dev = pcap_lookupdev(errbuf);

  if (dev == NULL) {
    printf("%s\n", errbuf);
    exit(1);
  }

  printf("DEV: %s\n", dev);

  /* open the device for sniffing.

   pcap_t *pcap_open_live(char *device,int snaplen, int prmisc,int to_ms,
   char *ebuf)

   snaplen - maximum size of packets to capture in bytes
   promisc - set card in promiscuous mode?
   to_ms   - time to wait for packets in miliseconds before read
   times out
   errbuf  - if something happens, place error string here

   Note if you change "prmisc" param to anything other than zero, you will
   get all packets your device sees, whether they are intendeed for you or
   not!! Be sure you know the rules of the network you are running on
   before you set your card in promiscuous mode!!     */

  descr = pcap_open_live(dev, BUFSIZ, 0, -1, errbuf);

  if (descr == NULL) {
    printf("pcap_open_live(): %s\n", errbuf);
    exit(1);
  }

  /*
   grab a packet from descr (yay!)
   u_char *pcap_next(pcap_t *p,struct pcap_pkthdr *h)
   so just pass in the descriptor we got from
   our call to pcap_open_live and an allocated
   struct pcap_pkthdr                                 */

  packet = pcap_next(descr, &hdr);

  if (packet == NULL) {/* dinna work *sob* */
    printf("Didn't grab packet\n");
    exit(1);
  }

  /*  struct pcap_pkthdr {
   struct timeval ts;   time stamp
   bpf_u_int32 caplen;  length of portion present
   bpf_u_int32;         lebgth this packet (off wire)
   }
   */

  printf("Grabbed packet of length %d\n", hdr.len);
  printf("Recieved at ..... %s\n", ctime((const time_t*) &hdr.ts.tv_sec));
  printf("Ethernet address length is %d\n", ETHER_HDR_LEN);

  /* lets start with the ether header... */
  eptr = (struct ether_header *) packet;

  /* Do a couple of checks to see what packet type we have..*/
  if (ntohs(eptr->ether_type) == ETHERTYPE_IP) {
    printf("Ethernet type hex:%x dec:%d is an IP packet\n",
           ntohs(eptr->ether_type), ntohs(eptr->ether_type));
  } else if (ntohs(eptr->ether_type) == ETHERTYPE_ARP) {
    printf("Ethernet type hex:%x dec:%d is an ARP packet\n",
           ntohs(eptr->ether_type), ntohs(eptr->ether_type));
  } else {
    printf("Ethernet type %x not IP", ntohs(eptr->ether_type));
    exit(1);
  }

  /* copied from Steven's UNP */
  ptr = eptr->ether_dhost;
  i = ETHER_ADDR_LEN;
  printf(" Destination Address:  ");
  do {
    printf("%s%x", (i == ETHER_ADDR_LEN) ? " " : ":", *ptr++);
  } while (--i > 0);
  printf("\n");

  ptr = eptr->ether_shost;
  i = ETHER_ADDR_LEN;
  printf(" Source Address:  ");
  do {
    printf("%s%x", (i == ETHER_ADDR_LEN) ? " " : ":", *ptr++);
  } while (--i > 0);
  printf("\n");

  return 0;
}

int simpleListen() {
  char *dev; /* name of the device to use */
  char *net; /* dot notation of the network address */
  char *mask;/* dot notation of the network mask    */
  int ret; /* return code */
  char errbuf[PCAP_ERRBUF_SIZE];
  bpf_u_int32 netp; /* ip          */
  bpf_u_int32 maskp;/* subnet mask */
  struct in_addr addr;

  /* ask pcap to find a valid device for use to sniff on */
  dev = pcap_lookupdev(errbuf);

  /* error checking */
  if (dev == NULL) {
    printf("%s\n", errbuf);
    exit(1);
  }

  /* print out device name */
  printf("DEV: %s\n", dev);

  /* ask pcap for the network address and mask of the device */
  ret = pcap_lookupnet(dev, &netp, &maskp, errbuf);

  if (ret == -1) {
    printf("%s\n", errbuf);
    exit(1);
  }

  /* get the network address in a human readable form */
  addr.s_addr = netp;
  net = inet_ntoa(addr);

  if (net == NULL)/* thanks Scott :-P */
  {
    perror("inet_ntoa");
    exit(1);
  }

  printf("NET: %s\n", net);

  /* do the same as above for the device's mask */
  addr.s_addr = maskp;
  mask = inet_ntoa(addr);

  if (mask == NULL) {
    perror("inet_ntoa");
    exit(1);
  }

  printf("MASK: %s\n", mask);

  return 0;

}

int main(int argc, char **argv) {
//  Spy s(argc,argv);
  return grabLoop();

}
