#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>

struct pseudo_header{
  u_int32_t source_address;
  u_int32_t dest_address;
  u_int8_t placeholder;
  u_int8_t protocol;
  u_int16_t tcp_length;
};

int sock;
char *data;
char datagram[4096];
struct sockaddr_in sin;
struct iphdr *iph = (struct iphdr *) datagram;
struct tcphdr *tcph = (struct tcphdr *) (datagram + sizeof(struct ip));
struct pseudo_header psh;

unsigned short checkSum(unsigned short *buf, int n){
  unsigned long sum = 0;
  while(n > 0){
    sum += *buf++;
    n--;
  }
  sum = (sum >> 16) + (sum & 0xffff);
  sum += (sum >> 16);
  return ~sum;
}

int makeSocket(char *cSrcIP, char *destIP, int srcPort, int destPort){
  sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
  int myDick = 1;
  const int *uno = &myDick;
  setsockopt(sock, IPPROTO_IP, IP_HDRINCL, uno, sizeof(myDick));
  struct pseudo_header psh;

  char srcIP[32];
  
  /*I know I don't usually do error handling but this is a buffer overflow risk*/
  if(strlen(cSrcIP) > 32){
    printf("Aw shit nigga! What are you thinking?\n");
    exit(1);
  }

  strcpy(srcIP, cSrcIP);

  memset(datagram, 0, 4096);
  data = datagram + sizeof(struct iphdr) + sizeof(struct tcphdr);
  strcpy(srcIP, cSrcIP);

  /*
   * Set most of our values here to speed up the sending process.
   */

  iph -> ihl = 5;
  iph -> version = 4;
  iph -> tos = 0;
  iph -> id = htonl(27328);
  iph -> frag_off = 0;
  iph -> ttl = 255;
  iph -> protocol = IPPROTO_TCP;
  iph -> saddr = inet_addr(srcIP);
  iph -> daddr = sin.sin_addr.s_addr;

  tcph -> source = htons(srcPort);
  tcph -> dest = htons(destPort);
  tcph -> seq = 0;
  tcph -> ack_seq = 0;
  tcph -> doff = 5;
  tcph -> fin = 0;
  tcph -> syn = 1;
  tcph -> rst = 0;
  tcph -> psh = 0;
  tcph -> ack = 0;
  tcph -> urg = 0;
  tcph -> window = htons(5840);
  tcph -> check = 0;
  tcph -> urg_ptr = 0;

  psh.source_address = inet_addr(srcIP);
  psh.dest_address = sin.sin_addr.s_addr;
  psh.placeholder = 0;
  psh.protocol = IPPROTO_TCP;

  sin.sin_family = AF_INET;
  sin.sin_port = htons(destPort); //or srcport. idk
  sin.sin_addr.s_addr = inet_addr(destIP);
}

void sendMSG(char *msg){
  strcpy(data, msg);
  iph -> tot_len = sizeof(struct iphdr) + sizeof(struct tcphdr) + strlen(data);
  iph -> check = 0;
  iph -> check = checkSum((unsigned short *) datagram, iph -> tot_len);
  psh.tcp_length = htons(sizeof(struct tcphdr) + strlen(data));

  int psize = sizeof(struct pseudo_header) + sizeof(struct tcphdr) + strlen(data);
  char *pseudogram;
  pseudogram = malloc(psize);
  memcpy(pseudogram, (char*) &psh, sizeof(struct pseudo_header));
  memcpy(pseudogram + sizeof(struct pseudo_header), tcph, sizeof(struct tcphdr) + strlen(data));
  tcph -> check = checkSum((unsigned short*) pseudogram, psize);
  
  int sendS = sendto(sock, datagram, iph -> tot_len, 0, (struct sockaddr *) &sin, sizeof(sin)); //mother of god we made it
  if(sendS < 0){
    printf("Error! Please run as super user before using raw sockets.\n");
    exit(1);
  }
}
