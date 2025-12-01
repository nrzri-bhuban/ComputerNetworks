#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>

// --- CONFIGURATION ---
#define DST_IP "10.0.0.1"  // TARGET IP
// ---------------------

unsigned short csum(unsigned short *ptr, int nbytes) {
    long sum = 0;
    unsigned short oddbyte;
    while(nbytes > 1) { sum += *ptr++; nbytes -= 2; }
    if(nbytes == 1) { oddbyte = 0; *((unsigned char*)&oddbyte) = *(unsigned char*)ptr; sum += oddbyte; }
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return (unsigned short)~sum;
}

int main(void) {
    int s = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    char packet[1024];
    struct icmphdr *icmph = (struct icmphdr *) packet;
    struct sockaddr_in dest_addr;

    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr.s_addr = inet_addr(DST_IP);

    // 1. Fill ICMP Header (Type 13 = Timestamp)
    memset(packet, 0, 1024);
    icmph->type = 13; 
    icmph->code = 0;
    icmph->checksum = 0;

    // 2. Add 3 Timestamps (3 integers set to 0) after the header
    // The payload for Timestamp requests is 12 bytes (3 x 4 bytes)
    // We just leave them as 0s in the memset buffer, but we must count them in size.
    
    // 3. Calculate Checksum (Header + 12 bytes of data)
    icmph->checksum = csum((unsigned short *)packet, sizeof(struct icmphdr) + 12);

    // 4. Send
    sendto(s, packet, sizeof(struct icmphdr) + 12, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    
    printf("Sent ICMP Timestamp Request to %s\n", DST_IP);
    return 0;
}
