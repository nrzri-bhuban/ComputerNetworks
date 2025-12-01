#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 9090
#define BUF_SIZE 2048

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUF_SIZE];
    char input[100];

    if (argc != 2) {
        printf("Usage: %s <server_ip>\n", argv[0]);
        return 1;
    }

    // Create Socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    // Convert IP string to binary
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

    // Connect
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        return 1;
    }

    printf("Connected to Fruit Shop!\n");
    printf("Format: <FruitName> <Quantity> (e.g., Apple 5)\n");
    printf("Type 'quit' to exit.\n\n");

    while (1) {
        printf("Enter Request: ");
        fgets(input, sizeof(input), stdin);

        if (strncmp(input, "quit", 4) == 0) break;

        // Send request
        send(sock, input, strlen(input), 0);

        // Receive response
        memset(buffer, 0, BUF_SIZE);
        if (recv(sock, buffer, BUF_SIZE, 0) <= 0) {
            printf("Server disconnected.\n");
            break;
        }

        printf("%s\n", buffer);
    }

    close(sock);
    return 0;
}
