
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <Server_IP>\n", argv[0]);
        return -1;
    }

    int sockfd;
    char buffer[BUFFER_SIZE];
    char message[BUFFER_SIZE];
    struct sockaddr_in server_addr;

    // Create socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);

    // SET TIMEOUT FOR RECEIVE (To detect Packet Loss)
    struct timeval tv;
    tv.tv_sec = 3;  // 3 Seconds timeout
    tv.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        perror("Error setting timeout");
    }

    while(1) {
        printf("\n--- Scientific UDP Client ---\n");
        printf("Enter operation (e.g., 'sin 90', 'add 5 10', 'inv 4'): ");
        fgets(message, BUFFER_SIZE, stdin);

        // Send message to server
        sendto(sockfd, (const char *)message, strlen(message), MSG_CONFIRM, 
               (const struct sockaddr *)&server_addr, sizeof(server_addr));
        
        printf("Packet sent. Waiting for response...\n");

        int n;
        socklen_t len = sizeof(server_addr);
        
        // Try to receive
        n = recvfrom(sockfd, (char *)buffer, BUFFER_SIZE, MSG_WAITALL, 
                     (struct sockaddr *)&server_addr, &len);
        
        if (n < 0) {
            // This block executes if the 3-second timer expires (Packet Loss detected)
            printf("(!) Alert: Request Timed Out. Packet may be lost.\n");
        } else {
            buffer[n] = '\0';
            printf("Server Response: %s\n", buffer);
        }
    }

    close(sockfd);
    return 0;
}
