#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 9090
#define BUF_SIZE 2048

int main(int argc, char *argv[]) {
    int sockfd;
    struct sockaddr_in server_addr;
    socklen_t addr_len;
    char buffer[BUF_SIZE];
    char input[100];

    if (argc != 2) {
        printf("Usage: %s <server_ip>\n", argv[0]);
        return 1;
    }

    // 1. Create UDP Socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

    printf("Ready to send UDP packets to %s!\n", argv[1]);
    printf("Format: <FruitName> <Quantity>\n");

    while (1) {
        printf("Enter Request: ");
        fgets(input, sizeof(input), stdin);

        if (strncmp(input, "quit", 4) == 0) break;

        // 2. Send Packet
        sendto(sockfd, input, strlen(input), 0, (struct sockaddr*)&server_addr, sizeof(server_addr));

        // 3. Wait for Reply
        memset(buffer, 0, BUF_SIZE);
        addr_len = sizeof(server_addr);
        
        // recvfrom blocks until a packet arrives
        int n = recvfrom(sockfd, buffer, BUF_SIZE, 0, (struct sockaddr*)&server_addr, &addr_len);
        if (n < 0) {
            perror("Recv failed");
            break;
        }

        buffer[n] = '\0';
        printf("%s\n", buffer);
    }

    close(sockfd);
    return 0;
}
