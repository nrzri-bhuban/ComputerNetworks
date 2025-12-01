#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>

// Define a buffer size to hold the packet
#define BUFFER_SIZE 65536

int main() {
    int raw_socket;
    struct sockaddr_in source_socket_address;
    socklen_t addr_len = sizeof(source_socket_address);
    unsigned char buffer[BUFFER_SIZE];

    // 1. Create a Raw Socket
    // AF_INET = IPv4, SOCK_RAW = Access to raw protocols, IPPROTO_TCP = TCP
    raw_socket = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    
    if (raw_socket < 0) {
        perror("Socket Error (Did you run with sudo?)");
        return 1;
    }

    printf("--- Simple TCP Sniffer Started ---\n");

    while (1) {
        // 2. Receive packets
        int packet_size = recvfrom(raw_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&source_socket_address, &addr_len);
        
        if (packet_size < 0) {
            printf("Failed to receive packet\n");
            continue;
        }

        // 3. Extract IP Header
        struct iphdr *ip_header = (struct iphdr *)buffer;
        
        // Calculate the size of the IP header (ihl is in 32-bit words, so * 4 gives bytes)
        unsigned short ip_header_len = ip_header->ihl * 4;

        // 4. Extract TCP Header
        // The TCP header is located immediately after the IP header
        struct tcphdr *tcp_header = (struct tcphdr *)(buffer + ip_header_len);

        // 5. Print Output (Source IP, Dest IP, Ports)
        struct sockaddr_in source, dest;
        source.sin_addr.s_addr = ip_header->saddr;
        dest.sin_addr.s_addr = ip_header->daddr;

        printf("\n[Packet Captured] Size: %d bytes\n", packet_size);
        printf("   Source IP: %s\n", inet_ntoa(source.sin_addr));
        printf("   Dest IP:   %s\n", inet_ntoa(dest.sin_addr));
        printf("   Source Port: %u\n", ntohs(tcp_header->source));
        printf("   Dest Port:   %u\n", ntohs(tcp_header->dest));
    }

    close(raw_socket);
    return 0;
}
