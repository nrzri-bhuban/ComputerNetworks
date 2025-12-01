#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netinet/ip.h>

// --- CONFIGURATION ---
#define SRC_IP "10.0.0.2"    // YOUR IP
#define DST_IP "10.0.0.1"    // TARGET IP
#define PAYLOAD "CSM24008"  // YOUR ROLL NO
// ---------------------

struct pseudo_header {
    u_int32_t source_address;
    u_int32_t dest_address;
    u_int8_t placeholder;
    u_int8_t protocol;
    u_int16_t tcp_length;
};

// Simple Checksum Function
unsigned short csum(unsigned short *ptr, int nbytes) {
    long sum = 0;
    unsigned short oddbyte;
    while(nbytes > 1) { sum += *ptr++; nbytes -= 2; }
    if(nbytes == 1) { oddbyte = 0; *((unsigned char*)&oddbyte) = *(unsigned char*)ptr; sum += oddbyte; }
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return (unsigned short)~sum;
}

int main (void) {
    int s = socket (PF_INET, SOCK_RAW, IPPROTO_TCP);
    char datagram[4096] , source_ip[32] , *data , *pseudogram;
    memset (datagram, 0, 4096);

    // Pointers to headers
    struct iphdr *iph = (struct iphdr *) datagram;
    struct tcphdr *tcph = (struct tcphdr *) (datagram + sizeof (struct ip));
    struct sockaddr_in sin;
    struct pseudo_header psh;

    // Set Data
    data = datagram + sizeof(struct iphdr) + sizeof(struct tcphdr);
    strcpy(data , PAYLOAD);

    sin.sin_family = AF_INET;
    sin.sin_port = htons(80);
    sin.sin_addr.s_addr = inet_addr (DST_IP);

    // 1. IP Header
    iph->ihl = 5; iph->version = 4; iph->tos = 0;
    iph->tot_len = sizeof (struct iphdr) + sizeof (struct tcphdr) + strlen(data);
    iph->id = htonl (54321); iph->frag_off = 0; iph->ttl = 255;
    iph->protocol = IPPROTO_TCP; iph->saddr = inet_addr (SRC_IP); iph->daddr = sin.sin_addr.s_addr;
    iph->check = csum ((unsigned short *) datagram, iph->tot_len);

    // 2. TCP Header
    tcph->source = htons (12345); tcph->dest = htons (80);
    tcph->seq = 0; tcph->ack_seq = 0;
    tcph->doff = 5; tcph->syn=1; tcph->window = htons (5840);

    // 3. TCP Checksum (Required Pseudo Header)
    psh.source_address = inet_addr(SRC_IP);
    psh.dest_address = sin.sin_addr.s_addr;
    psh.placeholder = 0; psh.protocol = IPPROTO_TCP;
    psh.tcp_length = htons(sizeof(struct tcphdr) + strlen(data));
    
    char pbuffer[1024];
    memcpy(pbuffer , (char*) &psh , sizeof (struct pseudo_header));
    memcpy(pbuffer + sizeof(struct pseudo_header) , tcph , sizeof(struct tcphdr) + strlen(data));
    tcph->check = csum( (unsigned short*) pbuffer , sizeof(struct pseudo_header) + sizeof(struct tcphdr) + strlen(data));

    // 4. Send
    int one = 1;
    const int *val = &one;
    setsockopt (s, IPPROTO_IP, IP_HDRINCL, val, sizeof (one));
    sendto (s, datagram, iph->tot_len , 0, (struct sockaddr *) &sin, sizeof (sin));
    
    printf("Sent TCP packet with payload: %s\n", data);
    return 0;
}
